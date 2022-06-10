/*
 * MQTT
 * 
 * Manages the message delivery to the AWS MQTT environment
 * 
 * SOURCE: https://github.com/knolleary/pubsubclient
 */

// MAKE SURE TO USE: https://github.com/arjenhiemstra/PubSubClientStatic
 
#include <PubSubClient.h>

int ledStatus = LOW;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  IndicatorStatus(RECEIVING);
  SerialMon.print(F("Message arrived ["));
  SerialMon.print(topic);
  SerialMon.print(F("] "));
  for (int i=0;i<length;i++) {
    SerialMon.print((char)payload[i]);
  }
  SerialMon.println();
}

PubSubClient mqtt(ClientSSL);

void mqtt_loop() {
    if ( mqtt.connected() ) mqtt.loop();
    yield();
}

void mqttInit(bool verboseOutput=false) {   // 1. Connect to the SERVER
    #ifdef MQTT_KEEPALIVE
      mqtt.setKeepAlive( MQTT_KEEPALIVE );  // Added this to see if it brings stability
      if ( verboseOutput ) SerialMon.printf("MQTT_KEEPALIVE is set at: %i\n", MQTT_KEEPALIVE);
    #endif  
    #ifdef MQTT_PACKET_SIZE
      mqtt.setBufferSize(MQTT_PACKET_SIZE); // Not sure but guessing this might just be influencing the amount of TCP packages; we don't care about that.
      if ( verboseOutput ) SerialMon.printf("MQTT_PACKET_SIZE is set at: %i\n", MQTT_PACKET_SIZE);
    #endif
    #ifdef MQTT_SOCKET_TIMEOUT
      mqtt.setSocketTimeout(MQTT_SOCKET_TIMEOUT); // Default is set to 15 ; value is in seconds
      if ( verboseOutput ) SerialMon.printf("MQTT_SOCKET_TIMEOUT is set at: %i\n", MQTT_SOCKET_TIMEOUT);
    #endif

    mqtt.setServer(broker, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    if ( verboseOutput ) SerialMon.printf("BROKER is set at: %s\n", broker);
    if ( verboseOutput ) SerialMon.printf("MQTT_PORT is set at: %i\n", MQTT_PORT);
    
    #ifdef  USE_CERTIFICATE_AUTH
      ClientSSL.setMutualAuthParams(Certificate);
      #ifdef SSL_TIMEOUT
        ClientSSL.setTimeout(SSL_TIMEOUT);    // Setting this a bit higher for Modem connection hoping to keep the connection alive
        if ( verboseOutput ) SerialMon.printf("SSL_TIMEOUT is set at: %i\n", SSL_TIMEOUT);
      #endif
    #endif  
}

uint32_t lastReconnectAttempt = millis();

boolean mqttConnect() {
    SerialMon.print(F("Connecting to BROKER: "));
    SerialMon.print(broker);

    // Connect to MQTT Broker
    boolean ConnectStatus = mqtt.connect(thingName);
    //boolean status = mqtt.connect("ClientId", mqtt_user, mqtt_pass);    // Or, if you want to authenticate MQTT:

    SerialMon.println(ConnectStatus ? F(" success") : F(" FAILED!"));
    
    if ( FLUSH ) SerialMon.flush();

    // Subscribes at this stage seem to be a reason for the system to break
    SerialMon.print(F("Subscribing to topics"));
    if ( mqtt.subscribe(sub_command_topic) ) {
      SerialMon.println(" done");   // Subscribe to the command topic
    } else {      
      SerialMon.println(" FAILED!!");   // Subscribe to the command topic
    }

    mqtt_loop();

//    mqtt_loop();  // Seems to cause a disconnect???
    //mqtt.subscribe(sub_status_topic);    // Subscribe to the status request topic

    return mqtt.connected();
}

void mqttDisconnect() {
    SerialMon.print(F("DISCONNECTING MQTT from broker "));
    SerialMon.print(String(broker) + "... ");

// Subscribes at this stage seem to be a reason for the system to break
    mqtt.unsubscribe(sub_command_topic);   // Subscribe to the command topic
    mqtt.disconnect();
    ClientSSL.removeSession(broker);
    ClientSSL.stop();
}

bool mqtt_manage_connection(bool firstboot=false) {   // Making sure we are connected before attempting to send messages  
  // Reconnect postpone to run max every 10 seconds
  uint32_t t = millis();
  
  if ( ! mqtt.connected() ) {
    if (t - lastReconnectAttempt > 10000L || firstboot) {
      lastReconnectAttempt = t;
      
      if ( NetworkConnected() ) {
        // Inform on SSL; don't deal with it; the ClientSSL library should be OK with it.
        SerialMon.println( ClientSSL.connected() ? F("SSL appears to be OK") : F("SSL appears to be DISCONNECTED") ); 
        SerialMon.println(F("CONNECTION: No MQTT connection; reconnecting..."));
        if ( ! ClientSSL.connected() ) {
          mqttDisconnect();
          mqttInit();
          mqttConnect();
        }
        // We could cleanup some ClientSSL stuff here if needed / possible!
        
        if ( mqttConnect() ) lastReconnectAttempt = 0;
      } else {
        SerialMon.println(F("CONNECTION: Network connection appears to be missing!"));
      }
    }
  }
  return mqtt.connected();
}

// https://github.com/OPEnSLab-OSU/SSLClient/issues/9

int mqtt_PreviousState = -1;

void mqttVerboseState() {
  static int state = 0;

  if ( state == mqtt.state() ) {
    return;
  } else {
    state = mqtt.state();
  }
  
  if ( state != 0 ) {
    SerialMon.print(F("MQTT ERROR: "));
 
    switch ( state ) {
    case MQTT_CONNECTION_TIMEOUT:
      SerialMon.print(F("TIMED OUT"));
      break;
    case MQTT_CONNECTION_LOST:
      SerialMon.print(F("CONNECTION LOST"));
      break;
    case MQTT_CONNECT_FAILED:
      SerialMon.print(F("CONNECTION FAILED"));
      break;
    case MQTT_DISCONNECTED:
      SerialMon.print(F("DISCONNECTED"));
      break;
    case MQTT_CONNECTED:
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      SerialMon.print(F("BAD PROTOCOL"));
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      SerialMon.print(F("BAD CLIENT_ID"));
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      SerialMon.print(F("UNAVAILABLE"));
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      SerialMon.print(F("BAD CREDENTIALS"));
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      SerialMon.print(F("UNAUTHORIZED"));
      break;
    default: 
      SerialMon.print(F("UNKNOWN"));
      // Print value of state
      break;
    }
   }
   SerialMon.flush();

}

/*
bool MQTTPublish(char *topic, char *payload, size_t payload_length)
{
  bool success = ( mqtt.connected() && mqtt.publish( (const char *) topic, (const char *) payload) );   // Seems this function has errors occurring!
  //  https://github.com/knolleary/pubsubclient/issues/832 implemented fix for checking memory  
  
  if ( success ) {
    mqtt_loop();
    ClientSSL.flush(); 
    mqtt_loop();

    messagesTransmitted++;    // Increase total number of messages transmitted
  }

  // We currently can't handle the errors while in transmission so our message sending will need to become a retry
  if ( ClientSSL.getWriteError() != SSLClient::SSL_OK ) {
    SerialMon.println(F("******** We have got a ClientSSL writerror! *********"));
    success = false;
  }
  if ( ! ClientSSL.connected() )                         success = false;
  
  //SerialMon.printf("[MQTT OUT %zu bytes]", payload_length);
  return success;
}
*/

#define MQTT_CHUNK_PACKETS

bool MQTTPublish(char *topic, char *payload, size_t payload_length, bool verboseChunks=false)
{
  bool success = false;
  if ( verboseChunks ) SerialMon.printf("[MQTT OUT %zu bytes]", payload_length);
  if ( verboseChunks ) SerialMon.flush();

  if ( mqtt.connected() ) {

#ifndef MQTT_CHUNK_PACKETS   // Do this only for smaller packets (restrictions apply!)
    if ( verboseChunks ) {
      SerialMon.printf("%s\n",payload); 
      SerialMon.flush();
      SerialMon.print(F(" -= One go =- "));
    }
    
    success = mqtt.publish((const char *)  topic, (const char *) payload);   // Seems this function has errors occurring!
#else
    if ( verboseChunks ) {
      SerialMon.print(F(" -= Chunked =- "));
    }
    
    mqtt.beginPublish((const char *) topic, payload_length, false);
    // WARNING; DO NOT PUT AN mqtt.loop() BETWEEN BEGIN AND END PUBLISH!
    
//    #ifdef MQTT_PACKET_SIZE
//      int chunkSize = MQTT_PACKET_SIZE;
//      SerialMon.printf("Chunksize set to %i", chunkSize);
//    #else
//      SerialMon.println("Chunksize set to 80 (Default)");
    int chunkSize = 80;
//    #endif

    int chunks    = payload_length / chunkSize; // 
    int remainder = payload_length % chunkSize; // Should give an int?
    
    if ( verboseChunks ) SerialMon.printf("Payload: %i ChunkSize %i ; Number of chunks :%i; Remainder %i\n", payload_length, chunkSize, chunks, remainder);
    
    for ( int i = 0 ; i <= chunks ; i++ ) {
      int offSet = chunkSize * i;
      if ( i == chunks ) {
        if ( remainder > 0 ) { chunkSize = remainder; } else { break; } // We are done; this would be the last chunk if there was a remainder
      }

      char buff[chunkSize + 1];
      strncpy(buff, payload + ( offSet ), chunkSize);
      buff[chunkSize] = '\0';
      
      if ( verboseChunks ) {
        SerialMon.printf("Chunk %i size: %i Offset: %i\n", i, chunkSize, offSet); 
        SerialMon.printf("%s\n",buff); 
        SerialMon.flush();
      }

      if ( ! mqtt.print(buff) ) return false;
    }
    
    mqtt.endPublish();
    success = true;
#endif
  } else {
    SerialMon.print("Missing MQTT connection on publish!!");
    SerialMon.flush();
  }

  if ( success ) {
    mqtt_loop();
//    ClientSSL.flush();        // Disabled due to FIXED SSLClient library
//    mqtt_loop();
    if ( verboseChunks ) SerialMon.println(" SUCCESS!");
    messagesTransmitted++;    // Increase total number of messages transmitted
  } else {
    if ( verboseChunks ) SerialMon.println(" FAILED!");
  }
  
  // We currently can't handle the errors while in transmission so our message sending will need to become a retry
  if ( ! ClientSSL.connected() ) success = false;
  if ( ClientSSL.getWriteError() != SSLClient::SSL_OK ) {
    SerialMon.println("******** ClientSSL write error!! ********");
    success = false;
  }

  return success;
}
