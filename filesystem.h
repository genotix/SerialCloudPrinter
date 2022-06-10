/*
 * Filesystem
 * 
 * Manages the storage of (temporary) files
 * 
 * SOURCE: https://github.com/joltwallet/esp_littlefs/blob/master/src/esp_littlefs.c
 * (Note that LittleFS is now part of the default IDF suite)
 * 
 * Perhaps we should consider using FFAT https://www.mischianti.org/2021/04/06/esp32-integrated-ffat-fat-exfat-filesystem-6/
 * 
 */

bool init_littlefs();                                   // Initialize the filesystem for usage
void list_files();                                      // Show a full list of the files in the filesystem
bool remove_file(char * logFile, bool verboseOutput);   // Remove a file from the filesystem
bool remove_file(bool verboseOutput);                   // Remove a file from the filesystem
bool rename_file(char * oldname, char * newname, bool verboseOutput);  // Rename a file
void clear_files();                                     // Remove ALL files in the filesystem!
void formatLittleFS();                                  // Format the LittleFS filesystem (Required when the size has been modified)

//#include "LittleFS.h" 
#include <LITTLEFS.h>
#include "b64writer.h"                                  // Use for streaming Base64 encoding
// SOURCE: https://forum.arduino.cc/index.php?topic=201007.0

void show_base64(char * logFile){
  File fileToRead = LITTLEFS.open(logFile,FILE_READ);
  SerialMon.println(String(F("Retrieve Base64 for: ")) + logFile);
  Base64Writer b64(Serial);
  
  if(!fileToRead){
    SerialMon.println(F("Failed to open file for reading"));
    if ( FLUSH ) SerialMon.flush();
    return;
  }
  
  while(fileToRead.available()){
   b64.write(fileToRead.read());
   yield();
  }
  fileToRead.close();

  SerialMon.println(F("\n[EOF]"));
}

bool is_hidden(String FileName) {
  String hidden[] = { ".crt", ".cert", ".pem", ".key" };
  int hidden_elements = sizeof(hidden) / sizeof(hidden[0]);
  
  for ( int i = 0 ; i < hidden_elements ; i++ ) {
    if ( FileName.endsWith(hidden[i]) ) return true;
  }
  return false;
}
 
void list_files() {
     // list LITTLEFS contents
   SerialMon.println(F("======================================================"));
   SerialMon.println(F("                  LittleFS LISTING"));  
   SerialMon.println(F("======================================================"));

   File root = LITTLEFS.open("/",FILE_READ);
   if (!root) {
       SerialMon.println(F("- failed to open directory"));
       for  ( int i = 0 ; i < 20 ; i ++ ) {
        IndicatorStatus(INDICATION_OFF);
        _delay(1000);
        IndicatorStatus(DISCONNECTED);
        _delay(1000);
        yield();
       }
       return;
   }
   
   if (!root.isDirectory()) {
       SerialMon.println(F(" - not a directory"));
       return;
   }
   
   File file = root.openNextFile();
   while (file) {
       if (file.isDirectory()) {
           SerialMon.print(F("  DIR : "));
           SerialMon.println(file.name());
       } else {
           if ( ! ( HIDE_CERTIFICATES && is_hidden(file.name())) ) {            
             SerialMon.print(F("  FILE: "));
             SerialMon.print(file.name());
             SerialMon.print(F("\tSIZE: "));
             SerialMon.println(file.size());
           }
       }
       file = root.openNextFile();
    yield();       
   }
   root.close();
   file.close();
   SerialMon.println(F("-----------------------------------------------------"));
}

bool remove_file(char * deleteFile, bool verboseOutput=false) {
  if ( verboseOutput ) {
    SerialMon.print(F("rm "));   
    SerialMon.print(deleteFile);
  }
  
  if ( ! LITTLEFS.exists(deleteFile)) {
    if ( verboseOutput ) SerialMon.println(F(" file not found!"));      
    return false;
  }

  if ( LITTLEFS.remove(deleteFile) ) {
    if ( verboseOutput ) SerialMon.println(F(" succeeded!"));  
    return true;
  } else {
  if ( verboseOutput ) SerialMon.println(F(" failed!"));     
    return false;     
  }
}


bool rename_file(char * oldname, char * newname, bool verboseOutput=false) {

  if ( verboseOutput ) SerialMon.printf("Renaming [%s] to [%s]", oldname, newname);

  if ( LITTLEFS.exists(newname) ) {
    SerialMon.println(F("New file already exists!; deleting!"));
    remove_file(newname);
  }
  // Check if old filename exists

  SerialMon.print("<");
  if ( LITTLEFS.rename(oldname, newname) ) {
    if ( verboseOutput ) SerialMon.println(F("Rename successful!"));
    SerialMon.print(">");
    return true;
  } else {

// Verbose output enum?
    if ( verboseOutput ) SerialMon.println(F("Rename failed!"));
    SerialMon.print("!");
    return false;
  }

}

void formatLittleFS() {
  bool formatted = LITTLEFS.format();

  if(formatted){
    SerialMon.println(F("*** Success formatting filesystem ***"));
  }else{
    SerialMon.println(F("!!! Error formatting filesystem !!!"));
  }
}

void printFromFile(char * FileName){
  // Check print file exists > 0
  char TmpFileName[MAX_FILENAME_SIZE];
  if ( FileName[0] != '/' ) {
    snprintf(TmpFileName, MAX_FILENAME_SIZE, "/%s", FileName);
  } else {
    snprintf(TmpFileName, MAX_FILENAME_SIZE, "%s", FileName);
  }
  
  File fileToRead = LITTLEFS.open(TmpFileName,FILE_READ);
  SerialMon.print(F("Printing file: "));
  SerialMon.println(TmpFileName);

  if(!fileToRead){
    SerialMon.println(F("Failed to open file for printing"));
  }
  
  while(fileToRead.available()){
    byte ReadByte = fileToRead.read();
    #ifdef ENABLE_RX_OUT
      IndicatorStatus(FORWARDING);
      SWSerial3.write(ReadByte);
    #endif  
    SerialMon.write(ReadByte);
    yield();
  }

  fileToRead.close();  
}

void show_file(char * FileName){
  if ( ! LITTLEFS.exists(FileName) ) {
    SerialMon.println(F("File does not exist"));
    return;
  }
  
  File fileToRead = LITTLEFS.open(FileName,FILE_READ);
  SerialMon.println(FileName);

  if(!fileToRead){
    SerialMon.println(F("Failed to open file for reading"));
    return;
  }

  SerialMon.println(F("\n[BEGIN]"));

  while(fileToRead.available()){
   SerialMon.write(fileToRead.read());
   if ( fileToRead.peek() == SPLIT_CHAR ) SerialMon.println();
   yield();
  }
  fileToRead.close();
  SerialMon.println(F("\n[END]"));
}

void clear_files() {
   File root = LITTLEFS.open("/",FILE_READ);
   if (!root) {
       SerialMon.println(F("- failed to open directory"));
       return;
   }
   
   if (!root.isDirectory()) {
       SerialMon.println(F(" - not a directory"));
       return;
   }


   File file = root.openNextFile();
   while (file) {
    if (file.isDirectory()) {
       SerialMon.print(F("  DIR : "));
       SerialMon.println(file.name());
    } else {
       char tempremove[MAX_FILENAME_SIZE];
       snprintf(tempremove, MAX_FILENAME_SIZE, "%s", file.name());
       file.close();
       remove_file(tempremove);
    }
    
    file = root.openNextFile();
    yield();
   }

   file.close();
   root.close();
}

bool init_LittleFS() {   
   if (!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
       SerialMon.println(F("Failed to mount file system"));
       return false;
   }
   return true;
}
