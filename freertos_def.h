/*
 * FreeRTOS_def
 * Setup FreeRTOS task definitions
 */



// FreeRTOS tasks:
// Determining GPS position
// Cleaning up the filesystem
// Listing the number of files to be processed
// Transmitting messages with .log extension
// Retrieving single Serial file (only one at a time)


TaskHandle_t SERIAL_HANDLER;
TaskHandle_t CONNECTION_HANDLER;
TaskHandle_t HEALTH_HANDLER;
TaskHandle_t BUTTON_HANDLER;

void serial_handler (void * parameter ) {
  UBaseType_t uxHighWaterMark;

  handleButtonPressed();
  /* Inspect our own high water mark on entering the task. */
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

  while (true) {
    if ( Serial2.available() || SerialMon.available() ) {
      serial_update();    // Handle Serial stream coming in
      cmd_os_update();    // Handle commands coming in
    }

    if ( mqtt.connected() ) {
      updateTransmitQueue();                                                      // Check the serial queue

#ifdef TRANSMIT_ON_RECEIVE
      if ( transmitQueue > 0 && !pauseTransmission) processQueue(true);       // Transmit immediately after receival
else      
      if ( !pauseTransmission ) processQueue();                                // Handle sending outgoing messages
#endif
    }

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    //    SerialMon.print(F("WM Serial: "));
    //    SerialMon.println(uxHighWaterMark);
    _delay(100);
    yield();
  }
}

void connection_handler (void * parameter) {
  UBaseType_t uxHighWaterMark;

  _delay(2000); // A little grace time before trying to manage the connection
  /* Inspect our own high water mark on entering the task. */
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

  while (true) {
    if ( ! NetworkTimeSet && Health[_CONNECTION] ) updateTime();  // Try to update the time because it isn't set yet
    mqtt_loop();  //    Looping      // Start connection to MQTT server the first time

    if ( millis() % 3600000 == 0 ) {
      uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
      SerialMon.print(F("WM Connection: "));
      SerialMon.println(uxHighWaterMark);
      _delay(1000);
    }

    _delay(100);
    yield();
  }
}

void health_handler (void * parameter) {
  UBaseType_t uxHighWaterMark;

  /* Inspect our own high water mark on entering the task. */
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

  while ( true ) {
    if ( enableNetwork ) manageLifeLine();                       // The full IoT connection to the cloud

    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    //SerialMon.print(F("WM Health: "));
    //SerialMon.println(uxHighWaterMark);
    _delay(500);
    yield();
  }
}

void queue_handler (void * parameter) {
  UBaseType_t uxHighWaterMark;

  /* Inspect our own high water mark on entering the task. */
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

  while ( true ) {
    updateTransmitQueue();                                                  // Check the file queue
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    //SerialMon.print(F("WM Queue: "));
    //SerialMon.println(uxHighWaterMark);
    _delay(2000);
  }
}

void button_handler (void * parameter) {
  while ( true ) {
    if ( FLASH.update() || PRINT.update() ) ButtonPressed();
    _delay(100);
    yield();
  }
}

void FreeRTOS_inittasks() {
    xTaskCreatePinnedToCore (
    serial_handler,
    "SERIAL_HANDLER",
    24000,
    NULL,
    2,
    &SERIAL_HANDLER,
    1
  );

  xTaskCreatePinnedToCore (
    connection_handler,
    "CONNECTION_HANDLER",
    24000,
    NULL,
    0,
    &CONNECTION_HANDLER,
    0
  );

  xTaskCreatePinnedToCore (
    health_handler,
    "HEALTH_HANDLER",
    4000,
    NULL,
    2,
    &HEALTH_HANDLER,
    1
  );

  xTaskCreatePinnedToCore (
    button_handler,
    "BUTTON_HANDLER",
    4000,
    NULL,
    2,
    &BUTTON_HANDLER,
    1
  );
}
