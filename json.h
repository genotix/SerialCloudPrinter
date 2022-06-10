/*
 * JSON
 * 
 * Manage MQTT reporting content
 * Also the PSRAM is managed in this function; this is extra RAM that can and MUST be used ; might be splitup in the future
 * 
 */

bool PSRAM_AVAILABLE=false;

void checkPSRAM() {
    if( psramInit() ){
            SerialMon.println(F(" The PSRAM is correctly initialized"));
            if ( FLUSH ) SerialMon.flush();
            PSRAM_AVAILABLE=true;
    }else{
            SerialMon.println(F(" PSRAM not found! Make sure to enable it!"));
            if ( FLUSH ) SerialMon.flush();
            PSRAM_AVAILABLE=false;            
    }
}

bool is_deduplicate_char(char Char) {
  for ( int i = 0 ; i < (sizeof deduplicate / sizeof deduplicate[0] ) ; i ++ ) {
    if ( deduplicate[i] == Char ) return true;
  }
  return false;
}

bool is_duplicate(char previousChar, char readChar){
  if ( previousChar == readChar && is_deduplicate_char(previousChar) ) return true;
  return false;
}


const static int Base64_resultSize = MAX_MESSAGE_SIZE;  // This should be the max message size!
// NOTE THAT THE BASE64 encoded result WILL BE LARGER THAN THE ORIGINAL STRING! DUNNNO HOW MUCH
rBase64generic<Base64_resultSize> mybase64; // Reserve enough space for the Base64 to be stored in

// Large buffer read seems to be less stable
#ifdef LARGE_BUFFER_READ
bool FileToBase64(char * FileName, char * output, size_t * outputSize, bool verboseConversion=false) {
  // CRC32 carriage return check
  
  //if ( ! heap_caps_check_integrity_all(true) ) {
  //  SerialMon.println(F("Heap corrupt!  FileToBase64"));
  //  if ( FLUSH ) SerialMon.flush();
  //}

  File    fileToRead;
  size_t  fileSize;
  
  if ( LITTLEFS.exists(FileName) ) {
    fileToRead = LITTLEFS.open(FileName,FILE_READ);
    
    if ( ! fileToRead ) {
      SerialMon.println(F("Unable to read the file; retrying later..."));
      if ( FLUSH ) SerialMon.flush();
      return false;
    }
    
    fileSize   = fileToRead.size();
    if ( fileSize == 0 ) {
      fileToRead.close();
      SerialMon.println(F("Empty file; removing and not transmitting!"));
      remove_file(FileName);    // We have nothing to send; remove the file!
      return false;             // Make sure the file isn't considered sent
    }
    
  } else {
    SerialMon.println(F("File not found!"));
    if ( FLUSH ) SerialMon.flush();
    return false;
  }

  if ( verboseConversion ) SerialMon.printf("Opened file %s, with size %zu\n", FileName, fileSize);
    
  if ( fileSize > MESSAGE_SIZE )  {               // 2000 > 2000 no
    SerialMon.print(F("Warning; file size ")) ;
    SerialMon.print(fileSize);
    SerialMon.print(F(" EXCEEDS maximum of "));
    SerialMon.print(MESSAGE_SIZE);
    SerialMon.println(F(" message will be cut-off!"));
    if ( FLUSH ) SerialMon.flush();
    fileSize = MESSAGE_SIZE;
  }

  char *input = (char *) ps_calloc((MESSAGE_SIZE + 1), sizeof(char));         // PSRAM!
//  char *input = (char *) calloc((MESSAGE_SIZE + 1), sizeof(char));         // NO PSRAM!
//  char *input = (char *) calloc((fileSize + 1), sizeof(char));         // NO PSRAM!

  
  char previousChar ='@';
  int  character    = 0 ;

  if ( verboseConversion ) SerialMon.println(F("<BEGIN>"));

  fileToRead.readBytes(input, fileSize);
  fileToRead.close(); // Close the file

  input[fileSize] = '\0';

  mybase64.encode(input);
  free(input);
  
  *outputSize = strnlen(mybase64.result(), Base64_resultSize);

  if ( verboseConversion ) SerialMon.println(F("<BEGIN>"));
  if ( verboseConversion ) SerialMon.println(mybase64.result());
  if ( verboseConversion ) SerialMon.println(F("<END>"));
  if ( verboseConversion ) SerialMon.printf("Size: %i", Base64_resultSize);

  if ( *outputSize == 0 ) { 
      SerialMon.printf(" <Base64 buffer size is EMPTY! [%zu]> ", *outputSize );
      remove_file(FileName);    // We have nothing to send; remove the file!
      return false;             // Make sure the file isn't considered sent
  } else if ( *outputSize <= Base64_resultSize - 1 ) {             // Output fits in message
    if ( verboseConversion ) SerialMon.printf("Output size is OK with [%zu] bytes ", *outputSize);
    strncpy(output, mybase64.result(), *outputSize);          // We can copy the max size here actually...
    if ( FLUSH ) SerialMon.flush();
  } else {
    SerialMon.print(F("Warning; result size "));
    SerialMon.print(*outputSize);    
    SerialMon.print(F(" exceeds message size of "));
    SerialMon.print(Base64_resultSize);
    SerialMon.println(F(" bytes which is the Maximum output size admitted! Message could become corrupt"));  
    strncpy(output, mybase64.result(), Base64_resultSize);    // Maximum filesize...    
  }
  
  return true;
}
#else
bool FileToBase64(char * FileName, char * output, size_t * outputSize, bool verboseConversion=false) {
  // CRC32 carriage return check
  File    fileToRead;
  size_t  fileSize;

  if ( LITTLEFS.exists(FileName) ) {
    fileToRead = LITTLEFS.open(FileName,FILE_READ);
    
    if ( ! fileToRead ) {
      SerialMon.println(F("Unable to read the file; retrying later..."));
      if ( FLUSH ) SerialMon.flush();
      return false;
    }
    
    fileSize   = fileToRead.size();

    if ( fileSize == 0 ) {
      fileToRead.close();
      SerialMon.println(F("Empty file; removing and not transmitting!"));
      remove_file(FileName);    // We have nothing to send; remove the file!
      return false;             // Make sure the file isn't considered sent
    }

  } else {
    SerialMon.println(F("File not found!"));
    if ( FLUSH ) SerialMon.flush();
    return false;
  }
  
  if ( verboseConversion ) SerialMon.printf("Opened file %s, with size %zu\n", FileName, fileSize);
    
  if ( fileSize > MESSAGE_SIZE )  { // Removed + 1
    SerialMon.print(F("Warning; file size ")) ;
    SerialMon.print(fileSize);
    SerialMon.print(F(" EXCEEDS maximum of "));
    SerialMon.print(MESSAGE_SIZE);
    SerialMon.println(F(" message will be cut-off!"));
    if ( FLUSH ) SerialMon.flush();
    fileSize = MESSAGE_SIZE;
  } else {
    fileSize = fileSize + 1;
  }

//  char *input = (char *) calloc((fileSize + 1), sizeof(char));         // NO PSRAM!
  char *input = (char *) ps_calloc((fileSize + 1), sizeof(char));         // NO PSRAM!
  char previousChar ='@';
  int  character    = 0 ;

  if ( verboseConversion ) SerialMon.println(F("<BEGIN>"));
  while( character < fileSize - 1 ) {             // We will read until maximum the message size is reached
    if ( ! fileToRead.available() ) break;        // We have reached the end of the file
    
    char readChar = fileToRead.read();
    
    if ( !deduplicateSpecialChars || ! is_duplicate(previousChar, readChar) ) {
     if ( verboseConversion ) SerialMon.print(readChar);
     input[character] = readChar;       // Make sure to read the content as char
     character++;
    }
    previousChar = readChar;
    yield();
  }
  fileToRead.close(); // Close the file
  
  if ( verboseConversion ) SerialMon.println(F("<END>"));
  if ( verboseConversion ) Serial.printf("Characters: %i", character);
  
  input[character] = '\0'; // Close the last character of the array  

  mybase64.encode(input);
  free(input);
  *outputSize = strnlen(mybase64.result(), Base64_resultSize);

  if ( verboseConversion ) SerialMon.println(F("<BEGIN>"));
  if ( verboseConversion ) SerialMon.println(mybase64.result());
  if ( verboseConversion ) SerialMon.println(F("<END>"));
  if ( verboseConversion ) SerialMon.printf("Size: %i", Base64_resultSize);

  if ( *outputSize > Base64_resultSize ) SerialMon.println(F("*** PROBLEM; OUR OUTPUT SIZE IS LARGER THAN THE BASE64 BUFFER! ***"));

  if ( *outputSize == 0 ) { 
      SerialMon.printf(" <Base64 buffer size is EMPTY! [%zu]> ", *outputSize );
      remove_file(FileName);    // We have nothing to send; remove the file!
      return false;             // Make sure the file isn't considered sent
  } else if ( *outputSize <= Base64_resultSize - 1 ) {             // Output fits in message
    if ( verboseConversion ) SerialMon.printf("Output size is OK with [%zu] bytes ", *outputSize);
    strncpy(output, mybase64.result(), *outputSize);          // We can copy the max size here actually...
    if ( FLUSH ) SerialMon.flush();
  } else {
    SerialMon.print(F("Warning; result size "));
    SerialMon.print(*outputSize);    
    SerialMon.print(F(" exceeds "));
    SerialMon.print(Base64_resultSize);
    SerialMon.println(F(" which is the Maximum output size admitted! Message will be cut-off"));  
    strncpy(output, mybase64.result(), *outputSize);    // Maximum filesize...    
  }
    
  return true;
}
#endif
