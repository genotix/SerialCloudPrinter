/*
 * UTIL
 * 
 * Utility library providing some special functions
 * 
 */ 
#include <rom/rtc.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

void verbose_print_reset_reason(RESET_REASON reason)
{
  switch (reason)
  {
    case 1  : SerialMon.println (F("Vbat power on reset"));break;
    case 3  : SerialMon.println (F("Software reset digital core"));break;
    case 4  : SerialMon.println (F("Legacy watch dog reset digital core"));break;
    case 5  : SerialMon.println (F("Deep Sleep reset digital core"));break;
    case 6  : SerialMon.println (F("Reset by SLC module, reset digital core"));break;
    case 7  : SerialMon.println (F("Timer Group0 Watch dog reset digital core"));break;
    case 8  : SerialMon.println (F("Timer Group1 Watch dog reset digital core"));break;
    case 9  : SerialMon.println (F("RTC Watch dog Reset digital core"));break;
    case 10 : SerialMon.println (F("Intrusion tested to reset CPU"));break;
    case 11 : SerialMon.println (F("Time Group reset CPU"));break;
    case 12 : SerialMon.println (F("Software reset CPU"));break;
    case 13 : SerialMon.println (F("RTC Watch dog Reset CPU"));break;
    case 14 : SerialMon.println (F("for APP CPU, resetted by PRO CPU"));break;
    case 15 : SerialMon.println (F("Reset when the vdd voltage is not stable"));break;
    case 16 : SerialMon.println (F("RTC Watch dog reset digital core and rtc module"));break;
    default : SerialMon.println (F("NO_MEAN"));
  }
}

void resetReason() {
  SerialMon.print(F("CPU0 reset reason:\t"));
  verbose_print_reset_reason(rtc_get_reset_reason(0));

  SerialMon.print(F("CPU1 reset reason:\t"));
  verbose_print_reset_reason(rtc_get_reset_reason(1));
}

 void ESP32DeviceInfo() {
  float         flashFreq   = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
  FlashMode_t   ideMode     = ESP.getFlashChipMode();
  uint32_t      chipId      = 0;

  SerialMon.println(F("==============================================================="));
  SerialMon.println(F("                    ESP32 Device information                   "));
  SerialMon.println(F("==============================================================="));
  SerialMon.print(F(" Version: \t\t"));      SerialMon.println(SKETCH_VERSION);
  SerialMon.print(F(" Build DATE: \t\t"));   SerialMon.printf("%s\n",compile_date);
  SerialMon.print(F(" Thing name: \t\t"));    SerialMon.println(thingName);
  SerialMon.print(F(" Device GUID: \t\t"));  SerialMon.println(thingName);

  #ifdef USE_WIFI
  SerialMon.print(F(" WiFi MAC : \t\t"));    SerialMon.println(String(WiFi.macAddress()));
  #endif
  
  SerialMon.print(F(" Flash frequency: \t")); SerialMon.print(flashFreq); SerialMon.println(F(" MHz"));
  SerialMon.printf(" Flash write mode: \t%s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
  SerialMon.printf(" CPU frequency: \t%u MHz\n", ESP.getCpuFreqMHz());
  checkPSRAM();

  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    yield();
  }

  SerialMon.print(F(" Chip ID: \t\t")); SerialMon.println(chipId);
  SerialMon.println(F("----------------------------------------------------------------------------"));
  resetReason();
  SerialMon.println(F("----------------------------------------------------------------------------"));
 }

void displayFreeHeap() {
   SerialMon.println(F("=============================================================="));
   SerialMon.print(F("               MEMORY USAGE ")); SerialMon.print(memoryFree()); SerialMon.println(F("% FREE"));
   SerialMon.println(F("=============================================================="));
   SerialMon.printf(  "Heap size      : %d\t\t Free          :  %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
   
   if ( PSRAM_AVAILABLE ) {
     SerialMon.printf("PSRAM size     : %d\t Free          :  %d\n", ESP.getPsramSize(), ESP.getFreePsram());
   }  
   SerialMon.printf(  "Max Alloc      : %d\t Min Free Heap :  %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT), heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
   SerialMon.println(F("--------------------------------------------------------------"));

   if ( PSRAM_AVAILABLE ) {
     SerialMon.printf("Total size     : %d\t", heap_caps_get_free_size(MALLOC_CAP_8BIT) + ESP.getFreePsram());
   } else {
     SerialMon.printf("Total size     : %d\t\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));    
   }
}

unsigned long messageNumber = 0;

// WARNING: LittleFS filesize limit is 32 bytes!
char * getLogFileName(char * FileName) { 
  messageNumber ++;
//  sprintf(FileName, "/%02d-%02d-%02dT%02d:%02d:%02d_%09d%s\0", rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHour(true), rtc.getMinute(), rtc.getSecond(), messageNumber, SERIAL_LOG_EXT); 
  sprintf(FileName, "/%lu_%09d%s\0", getEpoch(), messageNumber, TEMP_EXT);   
  if ( strnlen(FileName, MAX_FILENAME_SIZE) > 31 ) SerialMon.println(F("WARNING; filename size can't be more than 32 characters"));
  
  return FileName;
}

void handleButtonPressed(){
  if ( FLASH.pressedTime > 0 && Health[_CONNECTION] == GOOD ) {
      SerialMon.println(F("Flash pressed"));
      size_t maxMessageSize = 50;
      char Message[maxMessageSize] = "{ \"button\": \"flash\", \"state\":\"pressed\"}\0";
      if ( MQTTPublish(pub_button_topic, Message, strnlen(Message, maxMessageSize)) ) {
        //SerialMon.println(F("Flash reported"));
        Indicator(CRGB::Blue, CRGB::Blue);
        IndicatorShow();

        FLASH.pressedTime=0;
        FLASH.read();
      }
  }

  if ( PRINT.pressedTime > 0 && Health[_CONNECTION] == GOOD ) {
      SerialMon.println(F("Print pressed"));
      size_t maxMessageSize = 50;
      char Message[maxMessageSize] = "{ \"button\": \"print\", \"state\":\"pressed\"}\0";
      if ( MQTTPublish(pub_button_topic, Message, strlen(Message)) ){
        //SerialMon.println(F("Print reported"));
        Indicator(CRGB::Blue, CRGB::Blue);
        IndicatorShow();

        PRINT.pressedTime=0;
        PRINT.read();
      }
  }
}
