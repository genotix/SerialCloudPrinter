/*
 * SHADOW
 * 
 * Manages the device status with AWS
 * 
 */


bool          status_report       = true;
unsigned long last_report_update  = 0;
unsigned long last_failed_update  = 0;

bool updateThing() {
  SerialMon.print(F("Updating AWS with our status... "));
  size_t maxMessageSize = 100;
  char publishPayload[maxMessageSize];
  
  snprintf(publishPayload, maxMessageSize, "{\"state\": {\"reported\": {\"IP\":\"%s\", \"ConnectionType\" : \"%s\"}}}\0", network_info.IP.toString().c_str(), String(network_info.Type).c_str());
  
  if ( MQTTPublish(publishShadowUpdate, publishPayload, strnlen(publishPayload, maxMessageSize) ) ){
    last_report_update = millis();
    SerialMon.println(F("OK"));
    Serial.flush();
  } else {
    last_failed_update = millis();
    SerialMon.println(F("FAILED"));    
    Serial.flush();
  }
  mqtt_loop();
}

void shadowUpdate(){
    if ( mqtt.connected() ) {
      if ( status_report && updateThing() ) status_report = false;
    }
}
