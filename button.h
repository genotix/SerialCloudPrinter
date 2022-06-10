/*
 * BUTTON
 * 
 * Manages buttons being pressed
 * 
 */

class Button {
  private:
    byte Pin;
    bool pullup;
    bool state;

  public:
    bool     changed_state  = false;
    uint32_t pressedTime    = 0;
    uint32_t releasedTime   = 0;
  
Button(byte InitPin, bool InitPullup = true) {
  Pin         = InitPin;                          // Setting the pin at init
  pullup      = InitPullup;
  pinMode(Pin, pullup ? INPUT_PULLUP : INPUT);    // Setting the correct pin mode

  state       = pullup ? !digitalRead(Pin) : digitalRead(Pin); // Register state of the switch on boot; ASSUMING NOT PRESSED!
}

bool update(uint32_t momentTime = millis()) {
  bool newstate = pullup ? !digitalRead(Pin) : digitalRead(Pin); // Handle pullup here
  
  if ( state != newstate ) {
    state         = newstate;
    changed_state = true;
    
    if ( state ) {
      pressedTime  = momentTime;
    } else {
      releasedTime = momentTime;
    }    
    return true;
  }
  return false;
}

bool read() {
  changed_state = false;  
  return state;
}

};

Button  FLASH(FLASH_PIN);
Button  PRINT(PRINT_PIN);
