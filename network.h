/*
 * NETWORK
 * 
 * Responsible for managing the network connection upto the MQTT activity
 * Currently supports EITHER WiFi or GPRS / 2G
 * 
 */

// Initialize the SSL client library
// Initialize Wifi and the TinyGSM modem

#ifdef USE_WIFI
  #include "wifi.h"
#else  
  #include "modem.h"
#endif 

#include "ssl.h"

//#include "https.h"  // Should we require doing HTTPS calls

enum NetworkType { WIFI, MOBILE };


unsigned long connectionAt;     // Use this for reporting purposes; storing EPOCH
unsigned long disconnectedAt;



struct networkInfo {
                          NetworkType   Type;
                          IPAddress     IP;
                   };

networkInfo network_info;

void init_Network() {
  #ifdef USE_WIFI
    stop_WIFI();
    network_info.Type  = WIFI;
    SerialMon.println(F("****** Using WIFI ******"));
    delay(200);
    yield();
    init_WIFI();
    network_info.IP    = WiFi.localIP();
  #else
    stop_MODEM(); // If restart is due to crash; this can recover modem properly
    network_info.Type = MOBILE;
    SerialMon.println(F("****** Using MODEM ******"));
    delay(200);
    yield();
    init_MODEM();
    network_info.IP    = modem.localIP();    
  #endif
}

bool NetworkConnected() {
#ifdef USE_WIFI
    return ( WiFi.status() == WL_CONNECTED);
#else
    return ( modem.isNetworkConnected() && modem.getSimStatus() == 1 && modem.isGprsConnected() );
#endif
}

bool reset_Network() {
  SerialMon.println(F("Resetting network!"));
#ifdef USE_WIFI
  if ( ! NetworkConnected() ) {
    reset_WIFI();
    return NetworkConnected();
  }
#else
  if ( ! NetworkConnected() )  {
    restart_MODEM(); // Recovers from antenna disconnect and reconnect
//    reset_MODEM(); // Does not recover from antenna disconnect and reconnect
    return NetworkConnected();
  }
#endif
}
