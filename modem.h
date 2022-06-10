/*
 * MODEM
 * 
 * Manages the modem connection
 * 
 * SOURCE: https://github.com/vshymanskyy/TinyGSM
 * 
 */

/*
Value RSSI dBm  Condition
2 -109  Marginal
3 -107  Marginal
4 -105  Marginal
5 -103  Marginal
6 -101  Marginal
7 -99   Marginal
8 -97   Marginal
9 -95   Marginal

10  -93 OK
11  -91 OK
12  -89 OK
13  -87 OK
14  -85 OK

15  -83 Good
16  -81 Good
17  -79 Good
18  -77 Good
19  -75 Good

20  -73 Excellent
21  -71 Excellent
22  -69 Excellent
23  -67 Excellent
24  -65 Excellent
25  -63 Excellent
26  -61 Excellent
27  -59 Excellent
28  -57 Excellent
29  -55 Excellent
30  -53 Excellent
 */
 
// https://github.com/Appiko/nrf5x-firmware/tree/master/codebase/AT_lib
// THIS UNINTERRUPTED VERSION WOULD NEED TO BE IMPLEMENTED INSTEAD


#include <Wire.h>

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_RX_BUFFER   1024   // Set RX buffer to 1Kb ; moved to config
//#define TINY_GSM_RX_BUFFER   128   // Set RX buffer to 2 bytes


// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Add a reception delay - may be needed for a fast processor at a slow baud rate
// Enabled this 8 feb @ 22:19 seems to block activity at contacting the broker! NO GO?!
//#define TINY_GSM_YIELD() { _delay(2); }

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClient GPRS_GW(modem);   // Define the GPRS Gateway

#if defined(SIM800L_IP5306_VERSION_20190610)

#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

#define I2C_SDA              21
#define I2C_SCL              22

#define LED_ON               HIGH
#define LED_OFF              LOW

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// setPowerBoostKeepOn
bool setupPMU()
{
    bool en = true;
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en) {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}


#elif defined(SIM800L_AXP192_VERSION_20200327)

#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define MODEM_DTR            32
#define MODEM_RI             33

#define I2C_SDA              21
#define I2C_SCL              22

#define LED_ON               HIGH
#define LED_OFF              LOW


#elif defined(SIM800C_AXP192_VERSION_20200609)
// pin definitions
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       25
#define MODEM_TX             27
#define MODEM_RX             26
#define MODEM_DTR            32
#define MODEM_RI             33

// Disabled These two since it is our OUTPUT SERIAL
#define I2C_SDA              21
#define I2C_SCL              22

#define LED_ON               LOW
#define LED_OFF              HIGH

#elif defined(SIM800L_IP5306_VERSION_20200811)


#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

#define MODEM_DTR            32
#define MODEM_RI             33

// Disabled These two since it is our OUTPUT SERIAL
#define I2C_SDA              21
#define I2C_SCL              22

#define LED_ON               HIGH
#define LED_OFF              LOW

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// setPowerBoostKeepOn
bool setupPMU()
{
    bool en = true;
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en) {
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    } else {
        Wire.write(0x35); // 0x37 is default reg value
    }
    return Wire.endTransmission() == 0;
}

#else

#error "Please select the corresponding model"

#endif


#if defined(SIM800L_AXP192_VERSION_20200327) || defined(SIM800C_AXP192_VERSION_20200609)
#include <axp20x.h>         //https://github.com/lewisxhe/AXP202X_Library

AXP20X_Class axp;

bool setupPMU()
{
// For more information about the use of AXP192, please refer to AXP202X_Library https://github.com/lewisxhe/AXP202X_Library
    Wire.begin(I2C_SDA, I2C_SCL);
    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);

    if (ret == AXP_FAIL) {
        SerialMon.println(F("AXP Power begin failed"));
        return false;
    }

    //! Turn off unused power
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_OFF);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_OFF);

    //! Do not turn off DC3, it is powered by esp32
    // axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);

    // Set the charging indicator to turn off
    // Turn it off to save current consumption
    axp.setChgLEDMode(AXP20X_LED_OFF);

    // Set the charging indicator to flash once per second
    // axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);


    //! Use axp192 adc get voltage info
    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    float vbus_v = axp.getVbusVoltage();
    float vbus_c = axp.getVbusCurrent();
    float batt_v = axp.getBattVoltage();
    // axp.getBattPercentage();   // axp192 is not support percentage
    SerialMon.printf("VBUS:%.2f mV %.2f mA ,BATTERY: %.2f\n", vbus_v, vbus_c, batt_v);

    return true;
}

#endif



void start_MODEM()
{
    // Start power management
    /*
     * Disabled These two since SDC and SDA are our OUTPUT SERIAL
    if (setupPMU() == false) {
        SerialMon.println("Setting power error");
    }
    */

#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    _delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    _delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    digitalWrite(LED_BUILTIN, LED_OFF);
    IndicatorStatus(HARDWARE_OK);
}

void connect_MODEM() {
  #if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }
  #endif

  SerialMon.print(F("** Waiting for mobile network..."));  
  // This is actually optional if -somewhere- further along we check the modem.isNetworkConnected()
  if (!modem.waitForNetwork()) {
      SerialMon.println(F(" FAIL"));
      IndicatorStatus(BROKEN);
      _delay(2000);
      return;
  }
  SerialMon.println(F(" success!"));

  if (modem.isNetworkConnected()) {
      SerialMon.println(F("** Mobile network is connected!"));
      IndicatorStatus(NETWORK_CONNECTED);
  }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("** Connecting to APN ["));
  SerialMon.print(apn);
  SerialMon.print(F("] for GPRS "));
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(F("FAILED"));
      IndicatorStatus(BROKEN);
      _delay(2000);
      return;
  }
  SerialMon.println(F("success"));
  IndicatorStatus(IP_OBTAINED);

  SerialMon.print(F("** GPRS "));
  if (modem.isGprsConnected()) {
      SerialMon.println(F("connected!"));
  } else {
      IndicatorStatus(BROKEN);
      SerialMon.println(F("FAILED!"));
  }
}

void reset_MODEM() {  // Not really useful I'd say
  modem.restart();
}

void stop_MODEM() {
    SerialAT.flush();
    SerialAT.end();

    IndicatorStatus(DISCONNECTED);
    
    // Start power management
    /*
     * Disabled These two since SDC and SDA are our OUTPUT SERIAL
    if (setupPMU() == false) {
        SerialMon.println("Setting power error");
    }
     */

#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, LOW);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, LOW);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, LOW);
    _delay(1000);
    
    // Initialize the indicator as an output
    digitalWrite(LED_BUILTIN, LED_OFF);
}

void init_MODEM() {
  start_MODEM();

  IndicatorStatus(HARDWARE_INIT);
  // Set GSM module baud rate and UART pins
  
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX, false);
  
  SerialMon.println(F("Initializing modem..."));
  modem.init();  

  connect_MODEM();
}

void restart_MODEM() {
  SerialMon.println(F("Restarting modem..."));
  stop_MODEM();
  _delay(1000);
  init_MODEM();
}


struct modemInfo {
                    char      CCID[30];
                    char      IMEI[30];
                    char      IMSI[30];
                    char      Operator[30];
                    IPAddress IP;
                    int       Signal;
                 };

modemInfo modem_info; // Instantiate struct

// Would be good to report this status in a Status message!
// INCLUDING UUID!
void setModemInfo() {
  strncpy(modem_info.CCID, modem.getSimCCID().c_str(),      30);
  strncpy(modem_info.IMEI, modem.getIMEI().c_str(),         30);
  strncpy(modem_info.IMSI, modem.getIMSI().c_str(),         30);
  strncpy(modem_info.Operator, modem.getOperator().c_str(), 30);
  
  modem_info.IP       = modem.localIP();
  modem_info.Signal   = modem.getSignalQuality();
}

void test_Modem() {
  bool res = modem.isGprsConnected();
  SerialMon.println((String)"APN status: %s" + res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  SerialMon.println((String)"CCID: " + ccid);

  String imei = modem.getIMEI();
  SerialMon.println((String)"IMEI: " + imei);

  String imsi = modem.getIMSI();
  SerialMon.println((String)"IMSI: " + imsi);

  String cop = modem.getOperator();
  SerialMon.println("Operator: " + cop);

  IPAddress local = modem.localIP();
  SerialMon.println((String)"Local IP: " + local);

  int csq = modem.getSignalQuality();
  SerialMon.println((String)"Signal quality: " + csq);
}
