/*
 * CMD_OS; MINI OPERATING SYSTEM TO CONTROL ACTIVITY MANUALLY USING THE SERIAL CONNECTION
 * 
 * Code that manages commands coming from the serial input
 * 
 */

void help();                              // Show all commands
void displayFreeHeap();                   // Show the free heap memory
bool catchSpecialCommand(String command); // Command handler
void cmd_os_update();                     // Detect command input

void help() {
   SerialMon.println(F("==========================================="));
   SerialMon.println(F("                   HELP"));
   SerialMon.println(F("==========================================="));
   SerialMon.println(F("== COMMON INFO ON SYSTEM =="));
   SerialMon.println(F("SHOW               - Show file contents"));
   SerialMon.println(F("SHOWB64            - Show Base64 file contents"));
   SerialMon.println(F("LOG                - Show the last log records"));      
   SerialMon.println(F("QUEUE              - Update & show the number of queued messages")); 
   SerialMon.println(F("SENT               - Update & show the number of sent messages stored"));
   SerialMon.println(F("VERBOSE            - Show the incoming serial data through monitor")); 
   SerialMon.println(F("INFO               - Show the device specific information"));
   SerialMon.println(F("STATUS             - Show the status of the connectivity & device"));
   SerialMon.println(F("STORAGE            - Show the LittleFS filesystem usage"));
   SerialMon.println(F("MEM                - Show the active memory usage"));
   SerialMon.println(F("UPTIME             - Show the boot time"));
   SerialMon.println(F("TIME               - Show the UTC time"));
   SerialMon.println(F("UPDATETIME         - Update the time using our connection"));
   SerialMon.println(F("AWS                - Show AWS settings"));   
   SerialMon.println(F("*CONNECTION        - Connection details (NOT IMPLEMENTED YET!!!)"));
   SerialMon.println(F("*WIFI               - WIFI DETAILS (NOT IMPLEMENTED YET!!!"));
   SerialMon.println(F("MODEM              - MODEM DETAILS"));  
   SerialMon.println(F("CRASHDATA          - Show the backtrace of last crash"));
   SerialMon.println(F("== MANAGE FILESYSTEM =="));
   SerialMon.println(F("LIST               - List all files in LittleFS"));
   SerialMon.println(F("NEWFILE            - Create new file"));
   SerialMon.println(F("*AUTH              - Read fresh KEY and CERTIFICATE (NOT IMPLEMENTED!!!)"));
   SerialMon.println(F("RENAME             - Rename a file"));
   SerialMon.println(F("DELETE             - Delete a file"));
   SerialMon.println(F("REBOOT             - Reboot the system"));        
   SerialMon.println(F("TRASHBIN           - Empty entire filesystem! WARNING!!!"));
   SerialMon.println(F("FORMAT             - Format the filesystem"));  
   SerialMon.println(F("== MANAGE PROCESSES =="));
   SerialMon.println(F("SERIALCMD          - Write a serial text transmission"));
   SerialMon.println(F("HEALTH             - Report health to broker"));
   SerialMon.println(F("TRANSMIT           - Transmit available files NOW"));
   SerialMon.println(F("MARKTRANSMITTED    - Mark file as transmitted"));
   SerialMon.println(F("REQUEUE            - Requeue file"));
   SerialMon.println(F("GPS                - Update GPS Position (Modem ONLY!)"));   
   SerialMon.println(F("_MQTT              - Connect to MQTT server"));  
   SerialMon.println(F("PAUSE              - PAUSE transmission [ON/OFF] "));   
   SerialMon.println(F("_NETWORK           - NETWORK [ON/OFF] "));
   SerialMon.println(F("_WIFI              - WIFI    [ON/OFF] "));
   SerialMon.println(F("_MODEM             - MODEM   [ON/OFF] "));   
}

void toggleBool(const char parameter[], bool * valuePtr) {
  *valuePtr = ! *valuePtr;
  SerialMon.printf("Changing %s to %s ", parameter, ( *valuePtr == true ) ? "ON" : "OFF" );
}

bool catchSpecialCommand(String command) {
  int length_log = command.length();
  
  if (  length_log > 2 ) {

    command.toUpperCase();

    char timeArray[ta_Size];
    
      SerialMon.printf("\n*** Executing command: %s\n", command);
        if ( FLUSH ) SerialMon.flush();
// Scope ; Return False naar boven & Return true achter de if
      if ( command == F("HELP"))       help();
      if ( command == F("QUEUE"))      updateTransmitQueue(true);
      if ( command == F("UPTIME"))     SerialMon.printf("Device booted and time-synced since %s\n", bootTime);
      if ( command == F("TIME"))       SerialMon.printf("The current UTC device time is %s\n", getCurrentTime(timeArray));
      if ( command == F("STATUS"))     {
        updateHealth(false);
        showHealth(true);
      }      
      if ( command == F("MEM"))        displayFreeHeap();
      if ( command == F("INFO"))       ESP32DeviceInfo();      
      if ( command == F("SHOW"))       {
        char FileName[MAX_FILENAME_SIZE];
        snprintf(FileName, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, (const char *) F("Enter the filename to show: ")));
        show_file(FileName);
      }
      if ( command == F("SHOWB64"))    {
        char FileName[MAX_FILENAME_SIZE];
        snprintf(FileName, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1,(const char *) F("Enter the filename to show: ")));  
        show_base64(FileName);
      }
      if ( command == F("LIST"))          list_files();
      if ( command == F("TRASHBIN"))      clear_files();
      if ( command == F("TRANSMIT"))      processQueue(true);
#ifndef USE_WIFI
      if ( command == F("MODEM"))         test_Modem();
      if ( command == F("GPS"))           updateGPS();
#endif
      if ( command == F("_NETWORK"))      toggleBool((const char*) command.c_str(), &enableNetwork);
      if ( command == F("_CONNECT"))      mqttConnect();
      if ( command == F("_DISCONNECT"))   mqttDisconnect();
      
      if ( command == F("VERBOSE"))       toggleBool((const char*) command.c_str(), &verboseSerialIn);
      if ( command == F("UPDATETIME"))    updateTime();
      if ( command == F("PAUSE"))         toggleBool((const char*) command.c_str(), &pauseTransmission);
      if ( command == F("DELETE"))        {
        char deleteFile[MAX_FILENAME_SIZE];
        const char qDel[] PROGMEM = "Enter the filename to delete: ";

        // Move serial_read + snprintf to separate function to deal with MAX_FILENAME_SIZE proplerly
        snprintf(deleteFile, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qDel));
        if ( strlen(deleteFile) > 0 ) remove_file(deleteFile);
      }
      if ( command == F("REBOOT"))        ESP.restart();
      if ( command == F("FORMAT"))        formatLittleFS();
      if ( command == F("HEALTH"))        sendHealthStatus();
      if ( command == F("SENT"))          showSentMessages();

      if ( command == F("AWS")){
        SerialMon.println(F("============================================"));
        SerialMon.println(F("           AWS Connection details           "));
        SerialMon.println(F("============================================"));
        SerialMon.print(F("Thing name: \t"));    SerialMon.println(thingName);
        SerialMon.print(F("Device ID : \t"));    SerialMon.println(thingName);
        SerialMon.print(F("Broker: \t"));        SerialMon.println(broker);        
        SerialMon.print(F("Port: \t"));          SerialMon.println(MQTT_PORT);
        SerialMon.print(F("Shadow Topic: \t"));  SerialMon.println(publishShadowUpdate);
        SerialMon.print(F("Sensor Topic: \t"));  SerialMon.println(pub_sensor_topic);  
        #ifdef USE_CERTIFICATE_AUTH
          SerialMon.println(F("TLS :\t\tENABLED; Using certificate authentication"));
        #else  
          SerialMon.println(F("TLS :\t\tDISABLED"));
        #endif
      }

      if ( command == F("PRINT")) {
        char FileName[MAX_FILENAME_SIZE];
        const char qPrintFile[] PROGMEM  = "Please provide Filename to print:";
        snprintf(FileName, MAX_FILENAME_SIZE, "%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qPrintFile));
        printFromFile(FileName);
      }

      if ( command == F("SERIALCMD")) {         
          char Message[100];
          const char qSerialData[] PROGMEM  = "Please provide serial data (1 line only):";
          snprintf(Message, MESSAGE_SIZE, "%", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qSerialData));
          
          SerialMon.println(Message);
          #ifdef ENABLE_RX_OUT
            IndicatorStatus(FORWARDING);
            SWSerial3.println(Message);
          #else
            SerialMon.println(F("Not transmitting for ENABLE_RX_OUT is defined"));  
          #endif
      }

      #ifdef USE_WIFI
      if ( command == F("_WIFI")) {
        if ( WiFi.status() == WL_CONNECTED ) {
          SerialMon.println(F("WiFi CONNECTED; disconnecting!"));
          stop_WIFI();
        } else {
          SerialMon.println(F("WiFi DISCONNECTED; connecting!"));
          start_WIFI();
        }
      }
      #else
      if ( command == F("_MODEM")) {
        if ( digitalRead(MODEM_POWER_ON) ) {
          SerialMon.println(F("Modem ON; turning off!"));
          stop_MODEM();
        } else {
          SerialMon.println(F("Modem OFF; turning on!"));
          init_MODEM();
        }
      }
      #endif

      if ( command == F("_MQTT")) {
         mqtt_manage_connection();    
      }

      if ( command == "RENAME" ) {
        char oldname[MAX_FILENAME_SIZE];
        char newname[MAX_FILENAME_SIZE];

        const char qFileTobeRenamed[] PROGMEM  = "Provide file to be renamed:";
        snprintf(oldname, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qFileTobeRenamed));
        const char qNewFile[] PROGMEM = "Provide new file:";
        snprintf(newname, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qNewFile));
        rename_file( oldname, newname );
      }

      if ( command == "NEWFILE" ) {
        char FileName[MAX_FILENAME_SIZE];
        char fileContent[FILE_CONTENT_SIZE];
        const char qnewName[] PROGMEM = "Provide name of new file:";
        snprintf(FileName   , MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput   , (sizeof SerialReadInput) - 1, qnewName));
        const char qcontent[] PROGMEM  = "Provide content of new file:";
        snprintf(fileContent, FILE_CONTENT_SIZE, "%s", serial_read_file(SerialReadFile, (sizeof SerialReadFile) - 1 , qcontent));
        storeStringAsFile(FileName, fileContent);
      }

      if ( command == "AUTH" ) {
        char myNewKey [FILE_CONTENT_SIZE];
        char myNewCert[FILE_CONTENT_SIZE];

        const char qPkey[] PROGMEM  = "Provide a new PRIVATE KEY (xxxxxxxxxx-private.pem.key)     [ENTER TO CANCEL]:";
        char fKey[] = "/my.key";
        snprintf( myNewKey , FILE_CONTENT_SIZE, serial_read_file(SerialReadFile, (sizeof SerialReadFile) - 1, qPkey)); 
        storeStringAsFile(fKey, myNewKey);        
        const char qCert[] PROGMEM  = "Provide a new CERTIFICATE (xxxxxxxxxx-certificate.pem.crt) [ENTER TO CANCEL]:";
        char fCrt[] = "/my.crt";
        snprintf( myNewCert, FILE_CONTENT_SIZE, serial_read_file(SerialReadFile, (sizeof SerialReadFile) - 1, qCert));
        storeStringAsFile(fCrt, myNewCert);
      }
      
      if ( command == "REQUEUE" ) {
        char FileName[MAX_FILENAME_SIZE];
        const char qRetr[] PROGMEM  = "Please provide file to be retransmitted:";
        snprintf(FileName, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qRetr));
        reQueue(FileName);
      }
      
      if ( command == "MARKTRANSMITTED" ) {
        char FileName[MAX_FILENAME_SIZE];
        const char qTrn[] PROGMEM  = "Please provide file to be marked as transmitted:";
        snprintf(FileName, MAX_FILENAME_SIZE, "/%s", serial_read(SerialReadInput, (sizeof SerialReadInput) - 1, qTrn));
        markTransmitted(FileName);
      }

      
      if ( command == "STORAGE" ) {
        unsigned long tBytes = LITTLEFS.totalBytes(); 
        unsigned long uBytes = LITTLEFS.usedBytes();

        unsigned long percentage = ( 100 * uBytes ) / tBytes;
        SerialMon.println(F("==========================================="));
        SerialMon.printf("LittleFS space usage %lu%% (%lu of %lu)\n", percentage, uBytes, tBytes);
        SerialMon.println(F("==========================================="));
      }

      SerialMon.flush();
      return true;
  }
  SerialMon.flush();
  return false; 
}

void cmd_os_update() {
  String serialcmd="";
  
  if ( SerialMon.available() ) {
    while ( SerialMon.available() ) {
      if ( SerialMon.peek() == NEWLINE || SerialMon.peek() == CARRIAGERETURN ) { 
        while ( SerialMon.available() ){
        SerialMon.read();  // Eliminate last character(s)
        yield();
        }
        break;
      }
      serialcmd += char(SerialMon.read());
      yield();
    }

    if ( serialcmd.length() > 0 ) {
      if ( catchSpecialCommand(serialcmd) ) {
        SerialMon.println(F("Ok"));
      } else {
        SerialMon.println(F("Syntax error"));
        SerialMon.println(F("Ok"));
      }
      return;
    }
  }
}
