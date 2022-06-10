/*
 * SERIAL
 * 
 * Manages the serial connection and Serial buffer (Both hardware and software serial)
 * 
 * SOURCE: Default SoftwareSerial
 * ALTERNATIVE: For implementing interrupted HardwareSerial check: https://github.com/SlashDevin/NeoHWSerial
 */
 
#ifdef ENABLE_RX_OUT
   #include <SoftwareSerial.h>
#endif
 
void clearSerialInputBuffer();                                                          // Empty the serial buffer
char * serial_read     (char * input, int length, char * question, bool immediate);     // Get input from the serial console for managing the command os
char * serial_read_file(char * input, int length, char * question, bool immediate);

//char * serial_read_file(String question, String readUntil);                           // Get file input from the serial console
void serial_transmit(String inputString);                                               // Transmit a string of serial data
void init_serialsniffer();                                                              // Initialize the serial sniffer

#ifdef ENABLE_RX_OUT
  SoftwareSerial SWSerial3(SERIAL_OUT_RX_PIN, SERIAL_OUT_TX_PIN);                       // Define the serial OUTPUT port as Software Serial
#endif  

void init_PRINTER(int BaudRate = DEFAULT_SERIAL_BAUD) {
  #ifdef ENABLE_RX_OUT
    SWSerial3.begin(BaudRate);
    SerialMon.print(F("Initializing Serial OUTPUT to printer with "));
    SerialMon.print(BaudRate);
    SerialMon.println(F(" Baud"));
  #else
    SerialMon.println(F("Disabled printing; ENABLE_RX_OUT is undefined")); 
  #endif  
}

void clearSerialOutputBuffer() {
  #ifdef ENABLE_RX_OUT
    SWSerial3.flush();
  #endif  
}

void clearSerialInputBuffer() {
  while (Serial2.available() > 0) {
    Serial2.flush();
    Serial2.read();
    yield();
  }
}

char SerialReadFile[MESSAGE_SIZE];                                                        // Char array used to catch serial command data
// Changed to suit the MESSAGE_SIZE instead of being static 1024
// This should be moved towards PSRAM!

char * serial_read_file(char * input, int length, const char * question, bool immediate=false) {

  int i=0;
  uint32_t lastRead = millis();
  
  while ( ! SerialMon.available() ) {
    _delay(10);
    yield();      // Wait for input
  }

  while ( i < length ) {
    if ( SerialMon.available() ) {
      input[i] = SerialMon.read();
      SerialMon.print(input[i]);
      i++;
      lastRead = millis();
    }     
    
    if ( ! SerialMon.available() && ( millis() - lastRead > SERIAL_WAIT ) ) break; 
    yield();
  }
  input[i+1] = '\0';    
  
  return input;
}

#define MAX_SERIAL_INPUT_SIZE 120
char SerialReadInput[MAX_SERIAL_INPUT_SIZE];   // Char array used to catch serial command data

char * serial_read(char * input, int length, const char * question, bool immediate=false) {  
  if ( strnlen(question,MAX_SERIAL_INPUT_SIZE) > 0 ) SerialMon.println(question);
  
  while ( ! SerialMon.available() ) {
    _delay(10);
    yield();      // Wait for input
  }
  
  int i = 0;
  while ( SerialMon.available() ) {  
    input[i] = SerialMon.read();    
    if(input[i] == '\n')  break;          // Return is found!    
    i++;  
    if( i >= length - 1 ) break;
    yield();
  }

  input[i] = '\0';  // Close the array    

  // Clear the remainder
  while ( SerialMon.available() ) SerialMon.read();     // Clear the input buffer
   
  return input;
}
