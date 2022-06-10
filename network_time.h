/*
 * NETWORK TIME
 * 
 * Manages the time and with that filenaming
 * 
 */

#include <ESP32Time.h>

ESP32Time rtc;
bool gpsAcquired=false;

/*
  setTime(30, 24, 15, 17, 1, 2021);  // 17th Jan 2021 15:24:30
  setTime(1609459200);  // 1st Jan 2021 00:00:00
  setTime();            // default 1st Jan 2021 00:00:00
  
  getTime()          //  (String) 15:24:38
  getDate()          //  (String) Sun, Jan 17 2021
  getDate(true)      //  (String) Sunday, January 17 2021
  getDateTime()      //  (String) Sun, Jan 17 2021 15:24:38
  getDateTime(true)  //  (String) Sunday, January 17 2021 15:24:38
  getTimeDate()      //  (String) 15:24:38 Sun, Jan 17 2021
  getTimeDate(true)  //  (String) 15:24:38 Sunday, January 17 2021
  
  getSecond()        //  (int)     38    (0-59)
  getMinute()        //  (int)     24    (0-59)
  getHour()          //  (int)     3     (0-12)
  getHour(true)      //  (int)     15    (0-23)
  getAmPm()          //  (String)  pm
  getAmPm(true)      //  (String)  PM
  getDay()           //  (int)     17    (1-31)
  getDayofWeek()     //  (int)     0     (0-6)
  getDayofYear()     //  (int)     16    (0-365)
  getMonth()         //  (int)     0     (0-11)
  getYear()          //  (int)     2021
  
  getTime("%A, %B %d %Y %H:%M:%S")   // (String) returns time with specified format 
 */

bool    NetworkTimeSet  = false;    // Networktime should retrieve UTC ; not LOCAL time
char    bootTime[40];

#ifdef USE_WIFI
  #include <NTPClient.h>
  #include <WiFiUdp.h>
  WiFiUDP ntpUDP;
  
void updateTime() {
  //NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000); // If using a fixed timeserver
  NTPClient timeClient(ntpUDP);

  timeClient.begin();
  if ( timeClient.update() ) {  
    rtc.setTime(timeClient.getEpochTime());  // Setting time based on EPOCH
    _delay(100);
    if ( ! NetworkTimeSet ) strncpy(bootTime, rtc.getTimeDate(true).c_str(), 40);
    SerialMon.println(rtc.getTimeDate(true));        
    NetworkTimeSet = true;
  }
  timeClient.end();
}
  
#else 

#ifdef TINY_GSM_MODEM_HAS_GSM_LOCATION
char  location[40];

void updateGPS() {
  int   year     = 0;
  int   month    = 0;
  int   day      = 0;
  int   hour     = 0;
  int   min      = 0;
  int   sec      = 0;

  float lat           = 0;
  float lon           = 0;
  float accuracy      = 0;

  if (modem.getGsmLocation(&lat, &lon, &accuracy, &year, &month, &day, &hour, &min, &sec)) {
      snprintf(location, 40, "%s,%s", String(lon, 8).c_str(), String(lat, 8).c_str());
      SerialMon.println(F("GPS Position acquired!"));
      SerialMon.println(location);
      gpsAcquired=true;
  }
}
#endif

void updateTime() {   
    int year3    = 0;
    int month3   = 0;
    int day3     = 0;
    int hour3    = 0;
    int utc_hour = 0;
    int min3     = 0;
    int sec3     = 0;
    
    float timezone = 0;
                
  if (  modem.getNetworkTime(&year3, &month3, &day3, &hour3, &min3, &sec3,&timezone)  ) {  

    if ( timezone > 0 ) {
        SerialMon.println(F("Timezone > 0"));
        if ( (hour3 - timezone) < 0 ) {             // Correct time back to UTC
        utc_hour = 24 - (hour3 - timezone);          // Whoops; we'd get negative time
      } else {
        SerialMon.println(F("Subtracting timezone..."));
        utc_hour = hour3 - timezone;
      }
    } else {  // Timezone either == 0 or > 0
      if ( hour3 + timezone > 23 ) {              // Correct time back to UTC
        utc_hour = ( timezone + hour3 ) - 24;        // Whoops; we'd get hours greater than 24
      } else {
        utc_hour = hour3 + timezone;
      }      
    }

    SerialMon.printf("Year: %i,\t Month: %i,\t Day: %i\n", year3, month3, day3);
    SerialMon.printf("Hour: %i \t Minute: %i,\t Second: %i\n",utc_hour, min3, sec3);
    SerialMon.printf("Timezone: %f\n", timezone);
  
    rtc.setTime(sec3, min3, utc_hour, day3, month3, year3);  // 17th Jan 2021 15:24:30
    if ( ! NetworkTimeSet ) strncpy(bootTime, rtc.getTimeDate(true).c_str(), 40);
    SerialMon.println(rtc.getTimeDate(true));
    NetworkTimeSet = true;
  }
}
#endif

#define ta_Size 50
char * getCurrentTime(char * timeArray) {
  strncpy(timeArray, rtc.getTimeDate(true).c_str(), ta_Size);
}

#include "time.h"

// Function that gets current epoch time
uint32_t getEpoch() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //SerialMon.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
