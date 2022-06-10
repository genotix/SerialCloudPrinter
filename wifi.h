/*
 * WIFI
 * 
 * Manages the WIFI connectivity
 * 
 */

#include <WiFi.h>

// Update these with values suitable for your network.
WiFiClient WIFI_GW;

void WiFiEvent(WiFiEvent_t event)
{
    SerialMon.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case SYSTEM_EVENT_WIFI_READY: 
            SerialMon.println(F("WiFi interface ready"));
            IndicatorStatus(HARDWARE_OK);
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            SerialMon.println(F("Completed scan for access points"));
            break;
        case SYSTEM_EVENT_STA_START:
            SerialMon.println(F("WiFi client started"));
            IndicatorStatus(HARDWARE_INIT);
            break;
        case SYSTEM_EVENT_STA_STOP:
            SerialMon.println(F("WiFi clients stopped"));
            IndicatorStatus(BROKEN);
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            SerialMon.println(F("Connected to access point"));
            IndicatorStatus(NETWORK_CONNECTED);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            SerialMon.println(F("Disconnected from WiFi access point"));
            IndicatorStatus(DISCONNECTED);
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            SerialMon.println(F("Authentication mode of access point has changed"));
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            SerialMon.print(F("Obtained IP address: "));
            SerialMon.println(WiFi.localIP());
            IndicatorStatus(IP_OBTAINED);
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            SerialMon.println(F("Lost IP address and IP address is reset to 0"));
            Indicator(CRGB::Red, CRGB::Red);
            break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            SerialMon.println(F("WiFi Protected Setup (WPS): succeeded in enrollee mode"));
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            SerialMon.println(F("WiFi Protected Setup (WPS): failed in enrollee mode"));
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            SerialMon.println(F("WiFi Protected Setup (WPS): timeout in enrollee mode"));
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            SerialMon.println(F("WiFi Protected Setup (WPS): pin code in enrollee mode"));
            break;
        case SYSTEM_EVENT_AP_START:
            SerialMon.println(F("WiFi access point started"));
            break;
        case SYSTEM_EVENT_AP_STOP:
            SerialMon.println(F("WiFi access point  stopped"));
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            SerialMon.println(F("Client connected"));
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            SerialMon.println(F("Client disconnected"));
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            SerialMon.println(F("Assigned IP address to client"));
            break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            SerialMon.println(F("Received probe request"));
            break;
        case SYSTEM_EVENT_GOT_IP6:
            SerialMon.println(F("IPv6 is preferred"));
            break;
        case SYSTEM_EVENT_ETH_START:
            SerialMon.println(F("Ethernet started"));
            break;
        case SYSTEM_EVENT_ETH_STOP:
            SerialMon.println(F("Ethernet stopped"));
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            SerialMon.println(F("Ethernet connected"));
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            SerialMon.println(F("Ethernet disconnected"));
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            SerialMon.println(F("Obtained IP address"));
            break;
        default:
            break;
    }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    SerialMon.print(F("WiFi connected"));
    SerialMon.print(F("IP address: "));
    SerialMon.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void start_WIFI() {
  WiFi.setSleep(false);// <--- this command disables WiFi energy save mode and eliminate connected(): Disconnected: RES: 0, ERR: 128 problem
  WiFi.onEvent(WiFiEvent);        // This is very interesting coding!!!
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
      SerialMon.print(F("WiFi lost connection. Reason: "));
      SerialMon.println(info.disconnected.reason);
  }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED); 
   
  // We start by connecting to a WiFi network
  SerialMon.println();
  SerialMon.print(F("Connecting to WIFI "));
  SerialMon.println(STASSID);

  WiFi.begin(STASSID, STAPSK);
}

void stop_WIFI() {
  WiFi.disconnect();  
}

void init_WIFI(){
  stop_WIFI();
  start_WIFI();

  while (WiFi.status() != WL_CONNECTED) {
    SerialMon.print(F("-"));
    if ( FLUSH ) SerialMon.flush();
    yield();
  }

  randomSeed(micros());
}

void reset_WIFI() {
  init_WIFI();
}
