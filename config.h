/*
 * CONFIGURATION FILE HOLDING ALL POSSIBLE SETTINGS
 * 
 * NOTICE: for EVERY THING :
 * 1 RENAME   THING_NAME to the Device GUID (or at least something UNIQUE)
 * 2 CREATE   A NEW CERTIFICATE & KEY using the AWS environment
 */

const char compile_date[] = __DATE__ " " __TIME__;

/*
 * DEVICE
 */
#define DEVICE_TYPE       F("ESP32_TTGO_TCALLv1.5")
#define HARDWARE_GUID     F("5fb8dce0-22d5-4e37-9cdd-6ca8eba43f05")
#define HARDWARE_VERSION  15
#define SENSOR_TYPE       F("Serial sensor")


/* 
 *  DEBUGGING SETTINGS
 */

#define SerialMon Serial      // Set serial for debug console (to the Serial Monitor, default speed 115200)

#define VERBOSE_SERIAL_IN
#define SHOW_TRANSMIT         // Show transmission activity
//#define VERBOSE_TRANSMIT      // Debug transmission details
//#define VERBOSE_SSL         // Show all details on SSL connectivity



/*
 * FREERTOS
 */

//#define USE_FREERTOS



/*
 * INDICATORS
 */
#define TESTINDICATORS                                             // Test the on-board NeoPixels; Show RED, GREEN and BLUE at startup
 
const int LED_BUILTIN        = 13;
const int INDICATOR_DATA_PIN = 25;                                 // Is pin 2 used for something like MODEM_RST??


/*
 * SERIAL
 * 
 * NOTE THAT 
 *  SerialMon   = standard output (Controller)
 *  Serial1     = SerialAT, being the MODEM
 *  Serial2     = -reading- from the first RS232 (IN) port
 *  SWSerial3   = -writing- to the  second RS232 (OUT) port 
 */

#define DEFAULT_SERIAL_BAUD             9600
#define SERIAL_WAIT                    10000        // Wait N seconds for more serial information


#if HARDWARE_VERSION == 15                          // Hardware specific settings
  
  #define SERIAL_IN_RX_PIN                34        // Receive pin
  #define SERIAL_IN_TX_PIN                 2        // Transmit pin
  #define SERIAL_OUT_RX_PIN               35        // Receive pin
  #define SERIAL_OUT_TX_PIN                5        // Transmit pin

  #define FLASH_PIN                       15
  #define PRINT_PIN                        0

  #define ENABLE_RX_OUT                             // Normally RX IN is shorted to RX OUT, otherwise enable RX_OUT which will be writing to SWSerial3 

#else

  #define SERIAL_IN_RX_PIN                18        // Receive pin
  #define SERIAL_IN_TX_PIN                 2        // Transmit pin
  #define SERIAL_OUT_RX_PIN                2        // Receive pin
  #define SERIAL_OUT_TX_PIN                2        // Transmit pin

  #define FLASH_PIN                        0
  #define PRINT_PIN                       15

#endif

#define LOG_CHARRAY_MAXBUFFER          32768          // Incoming serial buffersize ; Serial2.setRxBufferSize() / Keep large to overcome LittleFS hanging giving issues on large amounts of data
//#define LOG_CHARRAY_MAXBUFFER           2048

#define ESTIMATE_HEADER_SIZE             250      // So the JSON text; GUID, Filename etc...
//#define MESSAGE_SIZE                     768      // Setting to 768 seems to handle communication nicely
#define MESSAGE_SIZE                     900      // Setting to 768 seems to handle communication nicely
                                                  // Increasing the value seems to make the active SSL disconnect
                                                  // Maximum message size (note that buffer must be about twice this size
                                                  // Size bigger than 4000 seems to give BEARSSL issues and are unreliable
                                                    
#define FILE_CONTENT_SIZE       MESSAGE_SIZE      // Maximum file content size to get processed

#include <rBase64.h>
const static int MAX_MESSAGE_SIZE = RBASE64_ENC_SIZECALC(MESSAGE_SIZE+ESTIMATE_HEADER_SIZE);  // Calculate the estimated message buffer_size + header

#define NEWLINE                           13  // '\n'
#define CARRIAGERETURN                    10  // '\r'

#define SPLIT_CHAR            CARRIAGERETURN        // Split character is normally either 13 or 10
//#define NEWLINE_NEWFILE                             // Split files on SPLIT_CHAR


/*
 * DATA PROCESSING
 */
 
char deduplicate[] = { '_', '-', ' ', '=', '\n', '\r', '\t' };
bool deduplicateSpecialChars = true;    // Making sure that below array of characters is included only ONCE if they would re-occur.
unsigned long messagesTransmitted = 0;

enum file_activity { DELETE, RENAME, NOACTION };
#define ON_TRANSMIT_SUCCESS       DELETE
#define RETRY_FAILED_MESSAGES                 // Either retry or mark as failed

/*
 * LittleFS
 */
#define FORMAT_LITTLEFS_IF_FAILED true

const char SERIAL_LOG_EXT[] PROGMEM   = ".log";
const char TEMP_EXT[] PROGMEM         = ".tmp";
const char FILE_SENT_EXT[] PROGMEM    = ".sent";
const char SEND_FAIL_EXT[] PROGMEM    = ".fail";

#define MAX_DIRLIST_COUNT             10  // Limit to N files
#define MAX_FILENAME_SIZE             32  // Limit filenamesize
#define MAX_ENDPOINT_SIZE             40  // Limit endpoint size
#define MAX_SHADOW_ENDPOINT_SIZE      60  // Limit shadow endpoint size

#define SEND_DELAY_TIME_MS         20000  // Send every 20 seconds
#define UPDATE_HEALTH_EVERY       600000  // Every 10 minutes
#define HEALTH_RECOVER_TIMEOUT   3600000  // Postpone for 60 minutes to recover from a problematic state before rebooting


 /*
  * CONNECTION (Define how you're planning to connect to the internet)
  */
//#define USE_WIFI


/*
 * WIFI
 */

#ifdef USE_WIFI
  #define STASSID   "Walmolen 5 - IoT"
  #define STAPSK    "starlink"
//  #define STASSID   "iPhone-Eric"
//  #define STAPSK    "steinweg1847"
#else
  #define STASSID   ""
  #define STAPSK    ""
#endif 


/*
 * MODEM
 */
#define TINY_GSM_MODEM_SIM800  // Unsure where this is used
// Please select the corresponding model
// #define SIM800L_IP5306_VERSION_20190610
#define SIM800L_AXP192_VERSION_20200327
// #define SIM800C_AXP192_VERSION_20200609
// #define SIM800L_IP5306_VERSION_20200811
//#define TINY_GSM_RX_BUFFER   32768   // Set RX buffer to 32Kb
#define TINY_GSM_RX_BUFFER   1024   // Set RX buffer to 1Kb (we are merely receiving confirmations for now)

#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "";
const char gprsUser[] = "";
const char gprsPass[] = "";


/*
 * MQTT details
 */
#define ANALOG_RNG_PIN  33
// Credentials

//const char[] PROGMEM mqtt_user = "eric";
//const char[] PROGMEM mqtt_pass = "1234";

// Port
#define MQTT_PORT 8883

//#define MQTT_VERSION MQTT_VERSION_3_1_1 //DEFAULT

// MQTT_KEEPALIVE (15 default): keepAlive interval in Seconds. Override with setKeepAlive()
#define MQTT_KEEPALIVE  30

// How long should the client wait until we would consider the connection timed-out?
//#define MQTT_SOCKET_TIMEOUT           30    // Increased hoping to avoid SSL_CLIENT_WRITE_FAIL errors
// MQTT_SOCKET_TIMEOUT (15 default): socket timeout interval in Seconds. Override with setSocketTimeout()
//#define MQTT_SOCKET_TIMEOUT           15  // Default

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE        80  // Leave this define out in case of Wifi usage

// Default; good for WIFI
// MQTT_MAX_PACKET_SIZE (256 default): Maximum packet size. Override with setBufferSize().
//#define MQTT_PACKET_SIZE            1024  // This is used in SSLClient AWS example
//#define MQTT_PACKET_SIZE            8192  // The maximum size of a packet including headers
                                            // Using wifi this size can easily be 32768
                                            // The limits seem to be on the Tx side of the modem buffer

//#define MQTT_PACKET_SIZE      MESSAGE_SIZE
#define MQTT_PACKET_SIZE             MESSAGE_SIZE
#define MQTT_MAX_TRANSFER_SIZE   MAX_MESSAGE_SIZE

// NOTE!!!!
// Any issues occorring with larger packet sized might be due to the
// Modem software not completing fast enough and making the call to 
//  mqtt.loop() too late to be in-sync.
// A-synchronous MQTT maybe an option?
// https://github.com/marvinroger/async-mqtt-client/blob/master/examples/FullyFeatured-ESP32/FullyFeatured-ESP32.ino
// Check:
// https://openslab-osu.github.io/SSLClient/index.html
//  On Read Buffer Overflow

// ***********************************************
// Best approach for UNLIMITED TRANSFER SIZE WOULD
// considerably be -INLINE STREAMING- of both BASE64
// and MQTT transfer. (This is possible using BEGIN, WRITE and END!)
// IT IS HOWEVER UNSURE IF THIS WORKS PROPERLY WITH THE MODEM
// PACKAGE SIZE MUST BE DECREASED THOUGH GIVEN THAT THE MODEM SEEMS TO BE LIMITED IN UPLOAD SIZE
// ***********************************************


/*
 *  CLOUD CONNECTIVITY
 */

#ifdef USE_WIFI
  #define SSL_TIMEOUT  30000    // Default = 30000
#else
  #define SSL_TIMEOUT 120000    // Default = 30000
#endif 

// This is normally ONLY set for ETHERNET, trying to find if this also helps Modem connectivity
//#define ETHERNET_LARGE_BUFFERS  // Tried but it failed

 
#define HIDE_CERTIFICATES false

#define AWS

#ifndef AWS       // Example for generic MQTT traffic
  const char broker[] PROGMEM           = "broker.hivemq.com";
  const char post_topic[] PROGMEM       = "GsmClientTest";
  const char topicLed[] PROGMEM         = "GsmClientTest/led";
  const char topicInit[] PROGMEM        = "GsmClientTest/init";
  const char topicLedStatus[] PROGMEM   = "GsmClientTest/ledStatus";
#else             // AWS Works fine through WiFi
  #define USE_CERTIFICATE_AUTH    // Should be set, normally
  const char broker[] PROGMEM           = "a2kiuttiadslks-ats.iot.eu-central-1.amazonaws.com";

  char pub_button_topic  [MAX_ENDPOINT_SIZE];
  char pub_sensor_topic  [MAX_ENDPOINT_SIZE];
  char pub_health_topic  [MAX_ENDPOINT_SIZE];
  char pub_response_topic[MAX_ENDPOINT_SIZE];
  char sub_command_topic [MAX_ENDPOINT_SIZE];
  char sub_status_topic  [MAX_ENDPOINT_SIZE];
  
  char publishShadowUpdate        [MAX_SHADOW_ENDPOINT_SIZE];
  char publishShadowUpdate_Accept [MAX_SHADOW_ENDPOINT_SIZE];
  char publishShadowUpdate_Reject [MAX_SHADOW_ENDPOINT_SIZE];
  char publishShadowUpdate_Delta  [MAX_SHADOW_ENDPOINT_SIZE];
  char publishShadowGet_Accepted  [MAX_SHADOW_ENDPOINT_SIZE];
  char publishShadowGet_Rejected  [MAX_SHADOW_ENDPOINT_SIZE];

  char* thingName = DeviceGUID();
  // .  iot/command/C44F335679FD

void set_endpoints() {
  snprintf(pub_button_topic,           MAX_ENDPOINT_SIZE,        "iot/button/%s",                         thingName);    // Register button press here
  snprintf(pub_sensor_topic,           MAX_ENDPOINT_SIZE,        "iot/serialdata_raw/%s",                 thingName);    // Publishing weights here
  snprintf(pub_health_topic,           MAX_ENDPOINT_SIZE,        "iot/health/%s",                         thingName);    // Responding to commands
  snprintf(pub_response_topic,         MAX_ENDPOINT_SIZE,        "iot/response/%s",                       thingName);    // Responding to commands
  snprintf(sub_command_topic,          MAX_ENDPOINT_SIZE,        "iot/command/%s",                        thingName);    // Listen to commands
  snprintf(sub_status_topic,           MAX_ENDPOINT_SIZE,        "status%s",                              thingName);    // Subscribing to the status request topic -> Would be good to have this "specific"

  snprintf(publishShadowUpdate,        MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/update",          thingName);    // Shadow endpoint
  snprintf(publishShadowUpdate_Accept, MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/update/accepted", thingName);
  snprintf(publishShadowUpdate_Reject, MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/update/rejected", thingName);
  snprintf(publishShadowUpdate_Delta,  MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/update/delta",    thingName);
  snprintf(publishShadowGet_Accepted,  MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/get/accepted",    thingName);
  snprintf(publishShadowGet_Rejected,  MAX_SHADOW_ENDPOINT_SIZE, "$aws/things/%s/shadow/get/rejected",    thingName);
}

#endif


/*
 * PROCESS ; default queue settings and activity management
 */

#define FLUSH             false     // Make sure there are no blocking SerialMon flushes

bool pauseTransmission  = false;    // Enable the handling of the queue default
bool enableNetwork      = true;     // Default enable the network connection for transmitting messages


// SET PROPER VARIABLES FOR CONFIG SETTINGS
#ifdef VERBOSE_TRANSMIT
  bool verbose_transmit = true;
#else
  bool verbose_transmit = false;
#endif

#ifdef SHOW_TRANSMIT
  bool show_transmit    = true;
#else
  bool show_transmit    = false;
#endif

#ifdef VERBOSE_SERIAL_IN
  bool  verboseSerialIn = true;
#else
  bool  verboseSerialIn = false;
#endif
