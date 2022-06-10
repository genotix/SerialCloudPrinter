/*
 * HEALTH
 * 
 * Manages the devices storage, memory, connections and other conditions
 * Has the ability to intervene, should something be wrong
 * 
 */

enum Component  { _PROVIDER, _SIM, _GPRS, _WIFI, _NETWORK, _CONNECTION, _TLS, _MQTT, _STORAGE, _MEM, _NONE};
enum State      { NA, INIT, GOOD, BAD, RECOVERING, RECOVER_TIMEOUT };
enum Warning    { NONE, WARNING };

#define ISSUE_DESCRIPTION_SIZE 60
State Health[13];
uint32_t lastHealthUpdate = 0;

#include "health_helper.h"

/*
 * MEMORY
 * STORAGE
 * 
 * 1. WIFI      NETWORK
 *    TLS       CONNECTION
 *    MQTT      CONNECTION
 * 
 * 2. MODEM     NETWORK
 *    GSM       NETWORK
 *    SIM       NETWORK
 *    GPRS      NETWORK
 *    TLS       CONNECTION
 *    MQTT      CONNECTION
 *    
 */

uint32_t HealthOk              = 0;  // epoch time

Component getHealthState(bool break_on_error = false) {
  if ( break_on_error && Health[_MEM]       != GOOD )  return _MEM;
  if ( break_on_error && Health[_STORAGE]   != GOOD )  return _STORAGE;  
  
#ifdef USE_WIFI
  if ( break_on_error && Health[_WIFI]      != GOOD)   return _WIFI;
#else
  if ( break_on_error && Health[_PROVIDER]  != GOOD)   return _PROVIDER;
  if ( break_on_error && Health[_SIM]       != GOOD)   return _SIM;
  if ( break_on_error && Health[_GPRS]      != GOOD)   return _GPRS;
#endif

  if ( break_on_error && Health[_NETWORK]   != GOOD)   return _NETWORK;
  if ( break_on_error && Health[_TLS]       != GOOD)   return _TLS;
  if ( break_on_error && Health[_MQTT]      != GOOD)   return _MQTT;

  if ( Health[_NETWORK]                     != GOOD )  return _NETWORK;
  if ( Health[_CONNECTION]                  != GOOD )  return _CONNECTION;

  return _NONE;  // All is well that ends well
}


void fixHealth() {
    static uint32_t HealthRecoverStart    = 0;        // millis

    Component health_state = getHealthState(true);
    static Component previousHealthIssueUnfixed  = _NONE;

    if ( previousHealthIssueUnfixed == health_state ) {
      SerialMon.println(F("Still fixing the same issue..."));
      if ( millis() - HealthRecoverStart > HEALTH_RECOVER_TIMEOUT ) {
        SerialMon.println(F("Rebooting due to unresponsiveness of fixes!"));
        ESP.restart();  // More clean reboot than reset
      } else {
        SerialMon.print(F("Will reboot in "));
        int MinutesUntilReboot = HEALTH_RECOVER_TIMEOUT - ( millis() - HealthRecoverStart );
        if ( MinutesUntilReboot > 0 ) {
          SerialMon.print( MinutesUntilReboot / 60000 );
        } else {
          SerialMon.print(F("XX"));
        }
        SerialMon.println(F(" minutes if health state hasn't recovered by then."));
      }
    } else {
      SerialMon.println(F("Health issue detected; trying to fix..."));
      HealthOk              = 0;
      HealthRecoverStart    = millis();
    }

    char issueDescription[ISSUE_DESCRIPTION_SIZE]= { '\0' };

    switch ( health_state ) {
    case _NONE:
          SerialMon.println(F("!!! No reason for calling fixHealth; all is good !!!"));
          break;
    case _MEM:  // Memory issue
          SerialMon.println(F("Potential memory leak discovered; too much memory in use; rebooting"));
          ESP.restart();
          break;
    case _STORAGE:
          SerialMon.println(F("Storage requirement too high; we need to transmit and cleanup!"));
          // Clean storage
          break;

// Network          
    case _WIFI:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - Wifi network issue", (int)health_state); 
          }
    case _PROVIDER:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - Provider issue", (int)health_state); 
          }
    case _SIM:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - SIM issue", (int)health_state); 
          }
    case _GPRS:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - GPRS issue", (int)health_state); 
          }
    case _NETWORK:
          SerialMon.println(issueDescription);
          if ( reset_Network() ) {
            SerialMon.println(F("NETWORK recovered!"));    
            HealthOk              = getEpoch();        
            HealthRecoverStart    = 0;
          } else {
            SerialMon.println(F("NETWORK Reset not effective.. will retry later!"));
            previousHealthIssueUnfixed=health_state;            
          }
          break;  

// Connection            
    case _TLS:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - TLS issue", (int)health_state); 
          }
    case _MQTT:
          if ( strlen(issueDescription) == 0 ) {
            snprintf(issueDescription, ISSUE_DESCRIPTION_SIZE, "FIXING [%d] - MQTT issue", (int)health_state); 
          }
    case _CONNECTION:
          SerialMon.println(issueDescription);
          if ( mqtt_manage_connection() ) {
            SerialMon.println(F("CONNECTION recovered! perhaps inform the server?"));    
            HealthOk              = getEpoch();        
            HealthRecoverStart    = 0;
          } else {
            SerialMon.println(F("CONNECTION Reset not effective.. will retry later!"));
            previousHealthIssueUnfixed=health_state;  
          }
         break;              
    default:
          SerialMon.printf("Invalid health state [%d]", (int)health_state);
          break; 
  }
}

void showHealth(bool verboseOutput = false) {
  SerialMon.println(F("---------------------------------"));
  SerialMon.printf("Memory     %s , threshold at: %i%%\n", okError(Health[_MEM]),     HEAP_WARN_BELOW);
  SerialMon.printf("Storage    %s , threshold at: %i%%\n", okError(Health[_STORAGE]), STORAGE_WARN_BELOW);
  SerialMon.printf("Messages transmitted : %lu\n", messagesTransmitted);

  SerialMon.println(F("---------------------------------"));  
  SerialMon.printf("Network    %s \n", okError(Health[_NETWORK]));
  #ifdef USE_WIFI
  SerialMon.printf("- WIFI     %s \n", okError(Health[_WIFI]));
  #else
  SerialMon.printf("- PROVIDER %s \n", okError(Health[_PROVIDER]));
  SerialMon.printf("- SIM      %s \n", okError(Health[_SIM]));
  SerialMon.printf("- GPRS     %s \n", okError(Health[_GPRS]));
  #endif
  SerialMon.printf("Connection %s \n", okError(Health[_CONNECTION]));
  SerialMon.printf("- TLS      %s \n", okError(Health[_TLS]));
  SerialMon.printf("- MQTT     %s \n", okError(Health[_MQTT]));
  SerialMon.println(F("================================="));
}


// INDICATOR
void indicateHealth(bool verboseOutput=false) {
  _delay(10);
  
  Component health_result = getHealthState(true);
  // NOTICE; DON'T START UPDATING ON HEALTH CHANGES
  CRGB Network, Ready;


  if ( health_result != _NONE && verboseOutput ) SerialMon.printf("HEALTH: [%d] - ", (int)health_result);
  
  switch ( health_result ) {
    case _NONE:
          if ( verboseOutput ) SerialMon.println(F("Connected"));
          Network = CRGB::Green;
          Ready   = transmitQueue == 0 ? CRGB::Black : CRGB::Yellow;
          break;
    case _MEM:
          if ( verboseOutput ) SerialMon.println(F("Memory issue"));
          break;
    case _STORAGE:
          Indicator(CRGB::Red, CRGB::Black);
          if ( verboseOutput ) SerialMon.println(F("Storage issue"));
          // Clean storage
          break;
    case _WIFI:
          if ( verboseOutput ) SerialMon.println(F("WiFi Network issue"));  
          Network = CRGB::Red;
          break;
    case _PROVIDER:
          if ( verboseOutput ) SerialMon.println(F("Provider issue"));
    case _SIM:
          if ( verboseOutput ) SerialMon.println(F("SIM issue"));
    case _GPRS:
          if ( verboseOutput ) SerialMon.println(F("GPRS issue"));
          Network = CRGB::Red;
          break;
    case _NETWORK:
          if ( verboseOutput ) SerialMon.println(F("Network issue"));
          
    case _TLS:
          Network = CRGB::Orange;   
          Ready   = CRGB::Black;
          if ( verboseOutput ) SerialMon.println(F("TLS not active..."));
          break;
    case _MQTT:
          Network = CRGB::Purple;            
          Ready   = CRGB::Black;
          if ( verboseOutput ) SerialMon.println(F("Connecting MQTT... "));   
          break;
    case _CONNECTION:
          if ( verboseOutput ) SerialMon.println(F("Connection issue"));     

          Network = CRGB::Orange;
          Ready = ( transmitQueue == 0 ) ? CRGB::Black : CRGB::Yellow;          
          break;              
    default:
          if ( verboseOutput ) SerialMon.println(F("Unknown issue..."));
          Network = CRGB::Pink;
          Ready   = CRGB::Blue;
          showHealth(true);
          break;
  }
  
  Indicator(Ready,  Network);     
  _delay(10);
  IndicatorShow();
}

void indicateHealthCheck() {
  Indicator(CRGB::Blue, CRGB::Green);         // Indicate health check / heartbeat
  IndicatorShow();
}


void updateHealth(bool verboseLog = false) {
  clearHealth();

  if( verboseLog ) SerialMon.println("Updating Health");
  Health[_MEM]       = memOk(verboseLog)                    ? GOOD : BAD;
  Health[_STORAGE]   = storageOk(verboseLog)                ? GOOD : BAD;

  // Connection
#ifdef USE_WIFI
  Health[_WIFI]       = ( WiFi.status() == WL_CONNECTED )   ? GOOD : BAD;
#else
  Health[_PROVIDER]   = modem.isNetworkConnected()          ? GOOD : BAD;
  Health[_SIM]        = (modem.getSimStatus() == 1 )        ? GOOD : BAD;  // _SIM is ok if status == 1
  Health[_GPRS]       = modem.isGprsConnected()             ? GOOD : BAD;
#endif

  Health[_TLS]       = SSLconnected()                       ? GOOD : BAD;
  Health[_MQTT]      = mqtt.connected()                     ? GOOD : BAD;

  Health[_NETWORK]    = (( Health[_WIFI] == GOOD ) || ( Health[_PROVIDER] == GOOD && Health[_SIM] == GOOD && Health[_GPRS] == GOOD ))  ? GOOD : BAD;
  Health[_CONNECTION] = (( Health[_MQTT] == GOOD ) && ( Health[_TLS] == GOOD ))                                                        ? GOOD : BAD;


  if( verboseLog ) showHealth(true);
  if( verboseLog ) SerialMon.println("Done updating Health");
  mqtt_loop();    // Added Sun 21 Mar 23:48
  yield();
}

void sendHealthStatus() {
  SerialMon.println(F("Reporting status to broker..."));
  // Inform main server on our status
  size_t maxMessageSize = 700;
  char  Report[maxMessageSize];
 
  #ifndef USE_WIFI
  snprintf(Report, maxMessageSize, "{\n" \
    " \"DeviceID\"    : \"%s\",\n"    \
    " \"Version\"    : \"%s\",\n"     \
    " \"Compiled\"    : \"%s\",\n"    \
    " \"GPS_LongLat\" : \"%s\",\n"    \
    " \"Free_Storage\" : %i,\n"       \
    " \"Free_Memory\"  : %i,\n"       \
    " \"Messages\"     : %lu,\n"      \
    " \"System_time\" : \"%s\",\n"    \
    " \"Boot_time\"   : \"%s\",\n"    \
    " \"IP\" : \"%s\",\n"             \
    " \"CCID\" : \"%s\",\n"           \
    " \"IMEI\" : \"%s\",\n"           \
    " \"IMSI\" : \"%s\",\n"           \
    " \"Operator\" : \"%s\",\n"       \
    " \"Signal\" : %i\n"              \
    "}",  thingName,
          SKETCH_VERSION,
          compile_date,
          location, 
          storageFree(),
          memoryFree(),
          messagesTransmitted,
          rtc.getTimeDate(true).c_str(), 
          bootTime, 
          network_info.IP.toString().c_str(),
          modem.getSimCCID().c_str(),
          modem.getIMEI().c_str(),
          modem.getIMSI().c_str(),
          modem.getOperator().c_str(),
          modem.getSignalQuality()
    );  
  #else
  snprintf(Report, maxMessageSize, "{\n" \
    " \"DeviceID\"    : \"%s\",\n"    \
    " \"Version\"    : \"%s\",\n"     \
    " \"Compiled\"    : \"%s\",\n"    \
    " \"Free_Storage\" : %i,\n"       \
    " \"Free_Memory\"  : %i,\n"       \
    " \"System_time\" : \"%s\",\n"    \
    " \"Messages\"     : %lu,\n"      \
    " \"Boot_time\"   : \"%s\",\n"    \
    " \"IP\" : \"%s\"\n"              \
    "}",  thingName,
          SKETCH_VERSION,
          compile_date,
          storageFree(),
          memoryFree(),
          rtc.getTimeDate(true).c_str(), 
          messagesTransmitted,
          bootTime, 
          network_info.IP.toString().c_str());  
  #endif

/*
  SerialMon.println(Report);
  SerialMon.print(F("Message size: "));
  SerialMon.println(strnlen(Report, MESSAGE_SIZE));
*/
  
  if ( MQTTPublish(pub_health_topic, Report, strnlen(Report, maxMessageSize)) ) {           // Reset update time if update succeeded
    SerialMon.println(F("Health updated successful!"));
    lastHealthUpdate=millis();
  } else {
    SerialMon.println(F("Update health failed!"));
  }
}

Component previous_health_result = _NONE;

void manageLifeLine() {  
  updateHealth(false);
  Component health_result = getHealthState(true);
  
  indicateHealthCheck();        // Show the blue ready light to indicate we are alive

  // Report on health change
  bool health_changed = ( previous_health_result != health_result ); 
  if ( health_changed ) {
    if ( previous_health_result == _NONE && Health[_CONNECTION] == GOOD ) {
      updateThing(); // Always report when we got a reboot
    }
    previous_health_result = health_result;

    SerialMon.print(F("Health result "));
    SerialMon.println(health_result);
  }

  indicateHealth(health_changed); // Show the current status

  mqtt_loop();    // Added Sun 21 Mar 23:48

  if ( Health[_CONNECTION] == GOOD ) {      // Only report if we have at least a network connection
    if ( lastHealthUpdate == 0 || millis() - lastHealthUpdate > UPDATE_HEALTH_EVERY ) {
      #ifndef USE_WIFI
        setModemInfo();                     // Retrieve fresh modem information
        if ( ! gpsAcquired ) updateGPS();   // Just do this once
      #endif

      shadowUpdate();
      sendHealthStatus();
    }
  } else {
     // Fix health issue here
     fixHealth(); // Health state will automatiacally be checked the next run
  }
}
