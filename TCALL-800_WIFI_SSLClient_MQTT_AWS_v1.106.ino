/*
   NOTICE: BEFORE FLASHING (PROVIDE The Certificate and Key in auth.h

   DO NOT TRY TO COMPILE WITH ESP32 FROM THE SKETCHBOOK!
   
   Board          : ESP32 Wrover Module (Automatically enables QIO PSRAM; which is fastest!)

   Partition scheme: Minimal 
   
   CPU Frequency  : 240 Mhz Wifi/Bt
   The PSRam gives us another 4 Mb of RAM capacity added from an SPI connected memory module which the ESP32 has.

   In terms of Flash performance: QIO > QOUT > DIO > DOUT where more to the left is faster
   
*/

#define SKETCH_VERSION "1.0.105"

/*
 1.106 Changes
 Slightly increased message size (To 900)
 Reflashed the devices; ran into a Certificate Expiry issue; see https://github.com/OPEnSLab-OSU/SSLClient/issues/27
 Problem: When the filesystem is too full, LittleFS responds too slow causing SSL to break connection.
          Current only fix is format the filesystem
 
 1.105 Changes
 Increased SERIAL_WAIT for Line 1 to retrieve full lines
 
 1.104 Changes 
 Increasing message size...
 Setup retry structure whenever a send has failed
 WARNING! LittleFS has issues writing a large number of files (there is a tipping moment where it starts to "defrag") taking a long time and ending the SSL connection
 Added mqtt.loop() after subscribing to topics seems to avoid halting at that stage
 Added mqtt_loop(); in serial update
 // Set KEEPALIVE and SOCKET_TIMEOUT of MQTT connection to DEFAULT

 1.103 Changes
 Changed the status command making it less verbose
 Changed modem reset to modem restart (More rigorous)
 Checking health right before starting to send messages now
 Added mqtt.loop() to processSerial() to avoid SSL disconnect while receiving serial data
 Changed Serial_read_file size to suit the MESSAGE_SIZE instead of being static 1024
 Corrected some message size inconsitencies
 
 1.102 Changes
 Added last parameter "false" to the serial modem connect ".begin"
 Changed restart_MODEM to reset_MODEM (Perhaps better?)
 Increased status update to every minute
 Lowered the max filesize to 768 showing to provide stability with larger files
 Renaming files to .fail when the communication apparently failed
 Changed the devicetype to 1.5 (didn't match hardware type)
 Added compile time to status message
 Modified the SSL buffer in the SSLClient Libraray according to https://github.com/OPEnSLab-OSU/SSLClient/issues/46 to avoid Discarded unread data to favor a write operation
 Enabled SSL Timeout
*/

#include "uuid.h"
#include "auth.h"
#include "delay.h"
#include "config.h"
#include "indicator.h"

#include "button.h"
#include "serial.h"
#include "filesystem.h"
#include "queuefs.h"

#include "network.h"
#include "network_time.h"
#include "json.h"
#include "mqtt.h"
#include "shadow.h"

#include "sniff.h"

#include "health.h"
#include "util.h"
#include "queue.h"

#include "cmd_os.h"


#ifdef USE_FREERTOS
  #include "freertos_def.h"
#endif

/*
  Application structure setup

main + uuid.h                                    // Transmitted content structure
     + auth.h                                      // The device certificate and private key
     + delay.h                                     // Wrapped delay function to assist FreeRTOS implementation
     + config.h                                    // The generic configuration
     + serial.h                                    // The serial handlings
     + button.h                                    // Managing buttons
     + indicator.h                                 // Managing onboard leds
     + littlefs.h +--------- b64writer.h           // Streaming Base64 data
     |
     + sniff.h
     |
     + network.h +----- wifi.h                     // Manage Wifi / Modem connection
     |           +----- modem.h -- Wire.h
     |           \----- ssl.h +--- SSLClient.h     // The connectionwrapper that gives us SSL
     |                        +--- certificates.h  // The Public CA certificate; e.g. Amazon Root CA 1
     |                        \--- https.h         // DISABLED; we can do HTTPS calls with this library
     |
     + network_time.h                              // NTP based time client
     |
     + json.h --------- base64.h                   // perhaps use b64writer.h instead...
     |
     + mqtt.h --------- PubSubClient.h             // Transmitting message to the AWS IoT Core service
     |
     + shadow.h                                    // Manage (shadow) device information to AWS
     |
     + health.h                                    // Manage device health and connectivity
     |
     + util.h                                      // Various useful functions
     |
     + transmit_log.h                              // Manage files to be transmitted to AWS
     |
     + cmd_os.h                                    // Tiny operating system for executing Serial driven commands

*/

void setup()
{
  heartBeat();                    // Blink to show we are awake
  
  set_endpoints();
  
  // Set console baud rate
  SerialMon.begin(115200);
  ESP32DeviceInfo();

//  if ( ! heap_caps_check_integrity_all(true) ) SerialMon.println("1: Heap corrupt!");

  // Some generic initialisations
  InitIndicators();
  if ( ! init_LittleFS() ) SerialMon.println(F("*** LittleFS INITIALIZATION FAILED! ***"));
  init_serialsniffer();
  init_PRINTER();
  init_Network();                                                   // Connect to the configured option (Wifi or Modem)

  _delay(1000);

  updateTime();

#ifdef USE_FREERTOS
  FreeRTOS_inittasks();
#else
  mqttInit();                                                       // Initialize our MQTT client
  _delay(200);
  mqttConnect();

  //if ( ! heap_caps_check_integrity_all(true) ) SerialMon.println(F("3: Heap corrupt!"));

  updateHealth();
#endif
}

void wait(unsigned long waitTime) {
  unsigned long startTime = millis();
  bool actionNeeded       = false;

  while ( millis() - startTime < waitTime ) {
    if ( transmitQueue > 0 )      actionNeeded = true;
    if ( Serial2.available()   )  actionNeeded = true;  // Incoming serial stream
    if ( SerialMon.available() )  actionNeeded = true;  // Incoming command; first use this seems to place something in the Serial2 buffer too
    if ( FLASH.update() )         actionNeeded = true;
    if ( PRINT.update() )         actionNeeded = true;

    mqtt_loop();    
    
    if ( actionNeeded == true ) break;
    yield();    
  }
}

// void delayedStart(unsigned long delay

void loop() {
#ifndef USE_FREERTOS
  if ( enableNetwork ) manageLifeLine();                          // The full IoT connection to the cloud
  mqtt_loop();   

  wait(2000);                                                     // Execute the interruptable wait
  serial_update();                                                // Handle Serial stream coming in
  cmd_os_update();                                                // Handle commands coming in

  if ( ! getHealthState() == _NONE ) fixHealth();
  if ( Health[_CONNECTION] == GOOD ) {                            // ONLY deal with this if we have connection
    handleButtonPressed();
    if ( ! NetworkTimeSet ) updateTime();                         // Try to update the time because it isn't set yet

    mqtt_loop();    
    updateTransmitQueue();
    mqtt_loop();                                                  // Immediately update the queue after transmission
     
    if ( transmitQueue > 0 && !pauseTransmission) processQueue(); // Handle sending outgoing messages
  }
  
  yield();
#endif
}
