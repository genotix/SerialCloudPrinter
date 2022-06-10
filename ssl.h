/*
 * SSL
 * 
 * Manages the TLS / SSL connection
 * 
 * SOURCE: https://github.com/OPEnSLab-OSU/SSLClient  (Working OK with broken SSLClient)
 * 
 * This might be a proper ALTERNATIVE: https://github.com/govorox/SSLClient
 * 
 */
 
#include <SSLClient.h>
#include "certificates.h"   // Our trust anchors ; normally the public CA certificate; e.g. Amazon Root CA 1

// Choose the analog pin to get semi-random data from for SSL
// Pick a pin that's not connected or attached to a randomish voltage source
//const int simultaneous_connections = 1; // Using just a single connection

const int simultaneous_connections = 2;
const int rand_pin = ANALOG_RNG_PIN;

#ifdef USE_WIFI   // This makes MQTT chose either path
  Client& ConnectionClient = WIFI_GW;
#else
  Client& ConnectionClient = GPRS_GW;
#endif

#define MAX_CERT_SIZE 2048
SSLClientParameters Certificate = SSLClientParameters::fromPEM(my_cert, strnlen(my_cert,MAX_CERT_SIZE), my_key, strnlen(my_key,MAX_CERT_SIZE));

#ifdef VERBOSE_SSL
  SSLClient ClientSSL(ConnectionClient, TAs, (size_t)TAs_NUM, ANALOG_RNG_PIN, simultaneous_connections, SSLClient::SSL_INFO); // 1 is the number of concurrent connections
#else
  SSLClient ClientSSL(ConnectionClient, TAs, (size_t)TAs_NUM, ANALOG_RNG_PIN, simultaneous_connections);  
#endif

bool SSLconnected() {
  return ClientSSL.connected();
}
