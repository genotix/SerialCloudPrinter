/*
 * QUEUE
 * 
 * Manages the transmission and renaming of .log files containing weight information
 * 
 */

//#define VERBOSE_QUEUE

bool TransmitMessage(char * FileName, bool showTransmit=false, bool verboseTransmit=false) {  
  bool result = false;

  //if ( ! heap_caps_check_integrity_all(true) ) SerialMon.println(F("Heap corrupt! @ TransmitMessage"));

  char * outputCharArray = (char *) ps_calloc((Base64_resultSize + 1), sizeof(char));        // RESERVED MEMORY IN PSRAM!!
  //char * outputCharArray = (char *) calloc((Base64_resultSize + 1), sizeof(char));        // RESERVED MEMORY IN PSRAM!!

  size_t outputsize=0;
  
  if ( ! FileToBase64(FileName, outputCharArray, &outputsize) ) {  // Retrieve file content -in base64-
    SerialMon.println(F("Error converting file to Base64!"));
    if ( FLUSH ) SerialMon.flush();
    return false;
  } else {
    if ( verboseTransmit ) SerialMon.println(F("Base64 conversion OK"));
    if ( FLUSH ) SerialMon.flush();
  }

  if ( outputsize > Base64_resultSize ) SerialMon.println(F("HOUSTON, we have a problem!!"));

  unsigned long sizeJsonMessage = strnlen(outputCharArray, Base64_resultSize) + strnlen(thingName, THINGNAME_SIZE) + strnlen(FileName, MAX_FILENAME_SIZE) + ESTIMATE_HEADER_SIZE;

  if ( sizeJsonMessage >= MAX_MESSAGE_SIZE ) {
    SerialMon.print(F("JSON array size ["));
    SerialMon.print(sizeJsonMessage);
    SerialMon.print(F("] EXCEEDS boundaries of ["));
    SerialMon.print(MAX_MESSAGE_SIZE);
    SerialMon.println("] and will be cutoff and likely corrupt!");
    sizeJsonMessage = MAX_MESSAGE_SIZE;
  } else {
    if ( verboseTransmit ) {
      SerialMon.print(F("JSON array size "));
      SerialMon.print(sizeJsonMessage);
      SerialMon.println(F(" within boundaries"));
    }
  }

  if ( verboseTransmit ) {
    SerialMon.printf("Reserving JSON memory [%lu]\n", sizeJsonMessage);
    if ( FLUSH ) SerialMon.flush();
  }

  char * JsonMessage = (char *) ps_calloc((sizeJsonMessage + 1), sizeof(char));        // RESERVED MEMORY IN PSRAM!!
  //char *JsonMessage = (char *) calloc((sizeJsonMessage + 1), sizeof(char));      // RESERVED MEMORY
  mqtt_loop();

  if ( verboseTransmit ) {
    SerialMon.println(F("Filling JSON String"));
    if ( FLUSH ) SerialMon.flush();
  }
    
  snprintf(JsonMessage, sizeJsonMessage, "{\"GUID\": \"%s\", \"FileName\": \"%s\",\"data\": \"%s\"}", thingName, FileName, outputCharArray);                       // Just spit out the raw data at this stage
  free(outputCharArray);
  mqtt_loop();

  if ( verboseTransmit ) {
    SerialMon.println(F("Done filling JSON String"));
    if ( FLUSH ) SerialMon.flush();
  }

  if ( showTransmit ) {
    SerialMon.printf("Transmitting : [%s] ; %zu bytes (%zu = MAX_MESSAGE_SIZE) ", FileName, sizeJsonMessage, MAX_MESSAGE_SIZE);
    if ( FLUSH ) SerialMon.flush();
  }

  if ( MQTTPublish(pub_sensor_topic, JsonMessage, strnlen(JsonMessage, sizeJsonMessage)) ) { 
    mqtt_loop();
    ClientSSL.flush();
    SerialAT.flush();

/*
    if ( ! ClientSSL.available() ) {    
      while ( ClientSSL.connected() && ! ClientSSL.available() ) {
              }
    }
*/
    
    if ( ! ClientSSL.connected()  ) {
      IndicatorStatus(FAILED);
      if ( showTransmit ) SerialMon.println(F(" SSL CONNECTION DROPPED"));
    } else {
      IndicatorStatus(SUCCESS);
      if ( showTransmit ) SerialMon.println(F(" SUCCESS"));
      result = true;
    }
    
   } else { 
    IndicatorStatus(FAILED);
    if ( showTransmit ) SerialMon.println(F(" FAILED"));
  }

  free(JsonMessage);
  yield();
  return result;
}

unsigned long lastSendAttempt = 0;
bool informedAboutLastSend   = false;

#ifdef TRANSMIT_ON_RECEIVE
  void processQueue(bool forceTransmit = true)
#else
  void processQueue(bool forceTransmit = false)
#endif
  {
  int returnedCount;              // Needs to obey the MAX_DIRLIST_COUNT

  if ( ! forceTransmit && lastSendAttempt != 0 ) {              // First time and Forced skip this check
    if ( millis() - lastSendAttempt < SEND_DELAY_TIME_MS ) {    // Wait until enough time has passed
      if ( ! informedAboutLastSend ) {
        SerialMon.printf("Last transmission was at millis %lu waiting for the next one...",lastSendAttempt);
        informedAboutLastSend = true;
      }
      return; // Only transmit the queue every SEND_DELAY_TIME_MS
    }
  }

  informedAboutLastSend = false;

  
  char FileToBetransMitted[MAX_FILENAME_SIZE]; 
  listFirstFileWithExtension(SERIAL_LOG_EXT, &returnedCount, FileToBetransMitted);

  #ifdef VERBOSE_TRANSMIT
    if ( returnedCount > 0 ) SerialMon.printf("%i message(s) waiting to be transmitted.\n", returnedCount);
  #endif
  
  if ( returnedCount == 0 ) return;

  IndicatorStatus(TRANSMITTING);
  
    if ( strnlen(FileToBetransMitted, MAX_FILENAME_SIZE) > 0 ) {
        if( TransmitMessage( FileToBetransMitted, show_transmit, verbose_transmit ) ) {
          markTransmitted(FileToBetransMitted);                     // Suppress the rename message˜
          FileToBetransMitted[0] = '\0';                            // Clear array
        } else {
          SerialMon.printf("We have issues transmitting %s\n", FileToBetransMitted);

          #ifndef RETRY_FAILED_MESSAGES
            bool success = markFailed(FileToBetransMitted);
            if ( success ) {
              SerialMon.printf("Marked failed %s\n", FileToBetransMitted);
            } else {
              SerialMon.printf("ISSUES marking failed %s\n", FileToBetransMitted);            
            }
            // If failed; rename to .failed
            // Add a numbering file to the filesystem
          #endif
        }
        listFirstFileWithExtension(SERIAL_LOG_EXT, &returnedCount, FileToBetransMitted);  // Check for the next available file
    } else {
      lastSendAttempt = millis(); // Place the lastSendAttempt ONLY when there is nothing more to send
    }
}
