#include <FastLED.h>

#define NUM_LEDS             2
#define UPDATES_PER_SECOND 100
#define BRIGHTNESS          32
#define LED_TYPE       WS2812B
#define COLOR_ORDER        GRB

#define LED_NETWORK 0
#define LED_READY   1

CRGB leds[NUM_LEDS];

bool  led_builtin = false;
bool  initblink   = true;

void InitLedBuiltin() {
  pinMode(LED_BUILTIN, OUTPUT);
  initblink = false;
}

void LedBuiltin(bool value = HIGH) {
  led_builtin = value;
  digitalWrite(LED_BUILTIN, led_builtin);
}

void heartBeat() {
  static uint32_t lasttime; // Function static maintains it's value after function exit.
  if ( initblink ) InitLedBuiltin();
  LedBuiltin();
  _delay(500);
  LedBuiltin(LOW);
  _delay(500);
  LedBuiltin();
  _delay(500);
  LedBuiltin(LOW);
  _delay(500);
}

void ChangeLedBuiltin() {
  LedBuiltin(!led_builtin);
}

void IndicatorShow() {
    FastLED.show();
}


void Indicator(CRGB led1, CRGB led2) {
  bool update_leds=false;
  // NOTE THAT WITH THE FIRST PCB version LED1 and LED2 are SWAPPED!
  if ( leds[0] != led2 ) {
    leds[0]     = led2;
    update_leds = true;
  }

  if ( leds[1] != led1 ) {
    leds[1]     = led1;
    update_leds = true;
  }

  if ( update_leds ) IndicatorShow();
}

enum StatusType { INDICATION_OFF, HARDWARE_OK, HARDWARE_INIT, NETWORK_CONNECTED, IP_OBTAINED, DISCONNECTED, BROKEN, RECEIVING, TRANSMITTING, SUCCESS, FAILED, FORWARDING };

void IndicatorStatus(StatusType Status) {
  switch ( Status ) {
    case INDICATION_OFF:
      Indicator(CRGB::Black, CRGB::Black);                          // Hardware_OK
     break;
    case HARDWARE_OK:
      Indicator(CRGB::Red, CRGB::White);                            // Hardware_OK
     break;
    case HARDWARE_INIT:
      Indicator(CRGB::Red, CRGB::Orange);                           // Init
     break;
    case NETWORK_CONNECTED:
      Indicator(CRGB::Red, CRGB::Yellow);                           // AP Connected
     break;
    case IP_OBTAINED:
      Indicator(CRGB::Red, CRGB::Green);                            // IP Connected
     break;
    case DISCONNECTED:
      Indicator(CRGB::Red, CRGB::Red);                              // Disconnected
     break;
    case BROKEN:
      Indicator(CRGB::Purple, CRGB::Purple);                        // Broken
     break;    
    case RECEIVING:
      Indicator(CRGB::White, CRGB::Green);                          // Receiving  SERIAL DATA
     break;    
    case FORWARDING:
      Indicator(CRGB::Green,  CRGB::Green);                         // FORWARDING TO SERIAL OUT
     break;    
    case TRANSMITTING:
      Indicator(CRGB::Blue,  CRGB::White);                          // Sending    TO CLOUD
     break;    
    case SUCCESS:
      Indicator(CRGB::Blue, CRGB::Blue);                            // SUCCESS
     break;    
    case FAILED:
      Indicator(CRGB::Red,  CRGB::Red);                             // FAILED
     break;    
    default:
      Indicator(CRGB::White, CRGB::White);
    break;  
  }
  IndicatorShow();

}

void DemoLeds() {
  Indicator(CRGB::Green, CRGB::Green);
  IndicatorShow();
  _delay(500);
  Indicator(CRGB::Red, CRGB::Red);
  IndicatorShow();
  _delay(500);
  Indicator(CRGB::Blue, CRGB::Blue);
  IndicatorShow();
  _delay(500);
  Indicator(CRGB::Black, CRGB::Black);
  IndicatorShow();
}


void InitIndicators() {
  SerialMon.print(F("Configure FastLEDs... "));
  if ( FLUSH ) SerialMon.flush();
  
  FastLED.addLeds<LED_TYPE, INDICATOR_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // GRB ordering is typical 
  FastLED.setBrightness(  BRIGHTNESS );

  #ifdef TESTINDICATORS
    DemoLeds();
  #endif
  
  SerialMon.println(F("OK"));
    if ( FLUSH ) SerialMon.flush();
  
  Indicator(CRGB::Black, CRGB::Black);
  IndicatorShow();
}

#define showWaitingAfter    10000     // Display a dot after -n- milliseconds of inactivity
uint32_t lastWait    = 0;

void resetLastWait() {
  lastWait = millis();
}

void WaitFeedback() {

  if ( lastWait == 0 ) lastWait = millis() ;

  if ( millis() - lastWait > showWaitingAfter ) {
    SerialMon.print(F("+"));  
    resetLastWait();    // Reset wait timer
  }
}
