/*
 * QUEUE_FS
 * Manages the queue on the filesystem
 * 
 */
int transmitQueue = 0;

void listFirstFileWithExtension(char const * extension, int * FileCount, char * FirstFile) {  // Defined incoming string array, defined incoming integer pointers
   //SerialMon.println("Starting list files");
   //if ( ! heap_caps_check_integrity_all(true) ) SerialMon.println(F("Heap corrupt! @ listFirstFileWithExtension"));
   //_delay(100);
   File root = LITTLEFS.open("/", FILE_READ);
   //SerialMon.println("Opened root");
   if (!root) {
       SerialMon.println(F("- failed to open directory"));
       return;
   }
   
   if ( !root.isDirectory() ) {
       SerialMon.println(F(" - not a directory"));
       return;
   }

   *FileCount = 0;  // Changed to *FileCount ; this is a pointer!
   bool fileFound = false;
   
   File file = root.openNextFile();
   //SerialMon.println("Opened next file");

   while ( file ) {
       if (file.isDirectory()) {
           SerialMon.print(F("  DIR : "));
           SerialMon.println(file.name());
       } else {

           if ( ( HIDE_CERTIFICATES && is_hidden(file.name())) ) continue;  
           
           if ( String(file.name()).endsWith(extension) ) {   

             if ( ! fileFound ) {
               #ifdef VERBOSE_QUEUE
                 SerialMon.print(*FileCount);
                 SerialMon.print(F("  FILE: ["));
                 SerialMon.print(file.name());
                 SerialMon.print(F("]\tSIZE: "));
                 SerialMon.println(file.size());
               #endif
               
               //strlcpy(FirstFile,file.name(), strnlen(file.name()), MAX_FILENAME_SIZE);   // We should close the array here!!!
               if ( strnlen(file.name(), MAX_FILENAME_SIZE) >= MAX_FILENAME_SIZE ) {
                 SerialMon.println(F("Filename size exceeds maximum!"));
               } else {
                snprintf(FirstFile, MAX_FILENAME_SIZE, "%s", file.name());
//                 strcpy(FirstFile,file.name()); // strlcpy IS null terminated but NOT advised; strcpy and strncpy ARE NOT
                                                  // Also c_str() is null terminated! also snprintf is NULL terminated
                                                  // strcpy can flow over the target size though (buffer overflow)!
                fileFound = true; // We have got the first file...
               }
             }
             *FileCount = *FileCount + 1;
           }
       }
       file.close();  // Might not be needed

/*
       if ( fileFound ) {
        while ( root.openNextFile() ) { // We have got the file; just count the remaining number of files for now...
          *FileCount = *FileCount + 1;
          SerialMon.println("Opened next file");
          yield();
        }
       } else {
         file = root.openNextFile();
         SerialMon.println("Opened next file");
       }
*/
       if ( fileFound ) break;  // Speed up the queue handling; having an idea of the files with .log extension would be nice though!
       file = root.openNextFile();
       yield(); // Disabled in attempt to speed things up
    }
  root.close();       // Might not be needed
  //SerialMon.println("Done");
}


void showSentMessages() {
  SerialMon.printf("Sent %lu messages\n", messagesTransmitted);
}

void updateTransmitQueue(bool verboseOutput=false) { // Not using the file content here!
  int returnedCount = 0;  // Needs to obey the MAX_DIRLIST_COUNT
  char FileToBetransMitted[MAX_FILENAME_SIZE]; 

  listFirstFileWithExtension(SERIAL_LOG_EXT, &returnedCount, FileToBetransMitted);
  transmitQueue = returnedCount;
  
  if (verboseOutput) SerialMon.printf("Queue holds %i items\n", returnedCount);
}
 
 void updateSentMessages() {
  int returnedCount   = 0;
  char FileToBetransMitted[MAX_FILENAME_SIZE]; 

  listFirstFileWithExtension(FILE_SENT_EXT, &returnedCount, FileToBetransMitted);
  SerialMon.printf("Sent history holds %i items\n ", returnedCount);
 }

 bool reQueue(char * FileName) {
  char ToName[MAX_FILENAME_SIZE];
  
//  SerialMon.printf("From %s\n", FileName);
  if (        String(FileName).indexOf(FILE_SENT_EXT,0) > 0 ) {
    snprintf(ToName, MAX_FILENAME_SIZE, "%s%s", (String(FileName).substring(0, String(FileName).indexOf(FILE_SENT_EXT, 0))).c_str(), SERIAL_LOG_EXT);
  } else {
    snprintf(ToName, MAX_FILENAME_SIZE, "%s%s", (String(FileName).substring(0, String(FileName).indexOf(SEND_FAIL_EXT, 0))).c_str(), SERIAL_LOG_EXT);
  }
  
//  SerialMon.printf("To %s'n", ToName);
  return rename_file(FileName, ToName);
 }

 bool Queue(char * FileName) {
  char ToName[MAX_FILENAME_SIZE];
  //SerialMon.printf("From %s\n", FileName);
  snprintf(ToName, MAX_FILENAME_SIZE, "%s%s", (String(FileName).substring(0, String(FileName).indexOf(TEMP_EXT, 0))).c_str(), SERIAL_LOG_EXT);
  //SerialMon.printf("To %s\n", ToName);
  return rename_file(FileName, ToName);
 }

 bool markFailed(char * FileName) {
  char ToName[MAX_FILENAME_SIZE];
  snprintf(ToName, MAX_FILENAME_SIZE, "%s%s", (String(FileName).substring(0, String(FileName).indexOf(SERIAL_LOG_EXT, 0))).c_str(), SEND_FAIL_EXT);

  return rename_file(FileName, ToName);
  }

 bool markTransmitted(char * FileName) {
  char ToName[MAX_FILENAME_SIZE];
  //if ( ! heap_caps_check_integrity_all(true) ) SerialMon.println(F("Heap corrupt! @ markTransmitted"));

  snprintf(ToName, MAX_FILENAME_SIZE, "%s%s", (String(FileName).substring(0, String(FileName).indexOf(SERIAL_LOG_EXT, 0))).c_str(), FILE_SENT_EXT);

  switch ( ON_TRANSMIT_SUCCESS ) {
    case RENAME:
      return rename_file(FileName, ToName);
      break;
    case DELETE:
      return remove_file(FileName);
      break;
    case NOACTION:
      return true;
      break;
    default:
      SerialMon.println(F("No action after transmitting"));
      return true;
    break;  
  }
  
 } 
