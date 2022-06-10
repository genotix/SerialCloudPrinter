/*
 * SNIFF
 * 
 * Manages listening to the serial port and initiating buffer read-out when it is filled
 * 
 */

void serial_update();                                          // Check if there is data in the Serial buffer and handle it
void processSerial(File* logFile, bool verboseOutput);         // Execute this when there is Serial data in the buffer waiting for processing
char * getLogFileName(char * FileName);                        // This will be overwritten in the network_time.h
void storeStringAsFile(char * FileName, char * fileContent);


bool newlineNewFile() {
  bool breakFromLoop = false;
  
  #ifdef NEWLINE_NEWFILE    // Splitup the files per line  
    while ( Serial2.available() && int(Serial2.peek()) == SPLIT_CHAR ) {
      SerialMon.println("FOUND NEWLINE CHARACTER!");
      Serial2.read();
      breakFromLoop = true;
      yield();
    }     
  #endif  
  
  return breakFromLoop;
}


#define MAX_SERIAL_CHUNK  (MESSAGE_SIZE - 1)
#define LARGE_BUFFER_READ

#ifdef LARGE_BUFFER_READ
void processSerial(File* logFile, bool verboseOutput=false) {
  int      readChunk;
  uint32_t SerialIncoming = millis();

  // Note that if you see INT Values: 255, 252, 253, 254 showing here there is a big chance you are missing the yellow jumper connecting Ri to R1
  /*
  SerialMon.print(" [");
  SerialMon.print((uint8_t)Serial2.peek());
  SerialMon.print("]");
  SerialMon.println(Serial2.peek(), BIN);
  */
  
  readChunk = Serial2.available();
  while ( readChunk < MAX_SERIAL_CHUNK && millis() - SerialIncoming < SERIAL_WAIT ) {
    if ( readChunk < Serial2.available() ) {
      readChunk = Serial2.available();  // Serial buffer chunck
      SerialIncoming = millis();
    }
    mqtt_loop();
  }

  SerialIncoming = 0;                                   // Reset timeout

  if ( readChunk > MAX_SERIAL_CHUNK ) readChunk = MAX_SERIAL_CHUNK; // Align the readchunk to be MAX the size of the file

  uint8_t SerialBuffer[readChunk+1];  // Serial buffer chunck + \0 space

  // We could grab _A LOT OF BYTES_ here at once...
  #ifdef NEWLINE_NEWFILE                                                          // Splitup the files per line  
    int actuallyRead = Serial2.readBytesUntil('\r',SerialBuffer,readChunk);       // Grab a "chunk" until '\r'
//    Serial2.readBytesUntil(char(SPLIT_CHAR),SerialBuffer,readChunk);              // Grab a "chunk" until '\r'
  #else
    int actuallyRead = Serial2.readBytes(SerialBuffer,readChunk);                 // Grab a "chunk"
  #endif

  SerialBuffer[actuallyRead] = '\0';

  // Note that our LittleFS cache size should likely need to match the MAX_SERIAL_CHUNK size
  // Besides LittleFS size of prog_buffer, read_buffer and cache_size must all be equal.
  // Configured #define CONFIG_LITTLEFS_CACHE_SIZE 128
  logFile->write(SerialBuffer, actuallyRead + 1);                                 // Write a "chunk"

  #ifdef ENABLE_RX_OUT
    IndicatorStatus(TRANSMITTING);
    SWSerial3.write(SerialBuffer, actuallyRead);                                     // Forward signal to output
  #endif

  if ( actuallyRead == MAX_SERIAL_CHUNK ) SerialMon.print(".");
  
  if ( verboseOutput ) {
    char str[actuallyRead + 1];
    strlcpy(str, (char *) SerialBuffer, actuallyRead + 1);
    SerialMon.println(str);
  }
}
#else
void processSerial(File* logFile, bool verboseOutput=false) {
      byte LastReadByte=0;
      uint32_t lastReadMoment=millis();
      int i = 0;

      while ( Serial2.available() > 0 || millis()-lastReadMoment < SERIAL_WAIT ) {           
          if ( Serial2.available() ) {
            LastReadByte = Serial2.read();                             // Grab a "byte"
            
  //          logFile->write(LastReadByte);                            // Output to logfile
            logFile->write((char)LastReadByte);                        // Output to logfile
  
            #ifdef ENABLE_RX_OUT
              IndicatorStatus(FORWARDING);
              SWSerial3.write(LastReadByte);                           // Forward signal to output
            #endif
            
            if ( verboseOutput ) {
              SerialMon.print(char(LastReadByte));                     // Optionally SHOW in standard output
              if ( LastReadByte == SPLIT_CHAR ) SerialMon.println();   // Optionally SHOW in standard output
            }
            i++;
  
            if ( i >= FILE_CONTENT_SIZE ) {                            // Maximum file size
              SerialMon.println(F("Maximum filesize reached!; Emptying queue for now!"));
              break;
            }
            lastReadMoment = millis();
          }
          mqtt_loop();
     }
}
#endif

void storeStringAsFile(char * FileName, char * fileContent) {
      SerialMon.printf("Storing file: [%s]\n", FileName);
// Print the written length
      
      File storeFile = LITTLEFS.open(FileName, FILE_WRITE);
      storeFile.print(fileContent);                           // Output to logfile
      storeFile.close();
}

void serial_update() {
    while ( Serial2.available()  ) {
      IndicatorStatus(RECEIVING);
      
      char FileName[MAX_FILENAME_SIZE];
      getLogFileName(FileName);

      if (verboseSerialIn) {
        SerialMon.printf("*** Incoming message; opening file: %s ***\n", FileName);
      }

      File logFile = LITTLEFS.open(FileName, FILE_WRITE);
      processSerial(&logFile, verboseSerialIn);
      if (verboseSerialIn) SerialMon.print(F(" [finished]"));
      SerialMon.flush();
      logFile.close();
      if (verboseSerialIn) SerialMon.print(F(" [closed file]"));
      SerialMon.flush();
      Queue(FileName);  // Renaming file to .log
      if (verboseSerialIn) SerialMon.println(F(" [renamed file]"));
      SerialMon.flush();
    }
    
    mqtt_loop();
}

void init_serialsniffer() {
      SerialMon.println(F("Initializing serial sniffer..."));
      Serial2.begin(DEFAULT_SERIAL_BAUD, SERIAL_8N1, SERIAL_IN_RX_PIN, SERIAL_IN_TX_PIN);     // Start the serial for the defined port 
      Serial2.setRxBufferSize(LOG_CHARRAY_MAXBUFFER);                                         // Increase the default buffer size
      clearSerialInputBuffer();                                                               // Empty any existing buffer
}

void init_serialwriter() {
      SerialMon.println(F("Initializing serial writer..."));
      clearSerialOutputBuffer();                                                         // Empty any existing buffer
}
