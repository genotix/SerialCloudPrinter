/*
 * Health Helper
 * Provides some useful functions to keep health.h smaller
 * 
 */

const char sOK[] PROGMEM        = "OK";       // Just labels for CMD_OS indication
const char sWARNING[] PROGMEM   = "WARNING";
const char sERROR[] PROGMEM     = "ERROR";

const char * okError(State ok, bool warning=false) {
  if ( ok == GOOD ) {
    return sOK;
  } else {
    return sERROR;
  }
}

// MEMORY
int memoryFree() {
  int memSize     = ( PSRAM_AVAILABLE ) ? ESP.getHeapSize() + ESP.getFreePsram() : ESP.getHeapSize();
  int memFree     = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  int memFreePerc = int( (100 / float(memSize)) * memFree );
  return memFreePerc;  
}

#define HEAP_OK_ABOVE 10  // Above 10% free heap all OK
bool memOk(bool verboseLog=false) {
  if ( verboseLog ) SerialMon.printf("Memory Free : %i%%\n", memoryFree());
  return ( memoryFree() > HEAP_OK_ABOVE );
}

#define HEAP_WARN_BELOW 20  // Warn when below 20% heap free
bool memWarning() { return ( memoryFree() < HEAP_WARN_BELOW ); }

// STORAGE
int storageFree() {
  int littlefsSize      = LITTLEFS.totalBytes(); 
  int littlefsFree      = (LITTLEFS.totalBytes() - LITTLEFS.usedBytes());
  int littlefsFreePerc  = int( (100 / float(littlefsSize)) * littlefsFree );

  return littlefsFreePerc;
}

#define STORAGE_OK_ABOVE 10  // Above 10% free storage all OK
bool storageOk(bool verboseLog=false) {
  if ( verboseLog ) SerialMon.printf("Storage Free: %i%%\n", storageFree());
  return ( storageFree() > STORAGE_OK_ABOVE );
}

#define STORAGE_WARN_BELOW 20  // Warn when below 20% storage free
bool storageWarning() { return ( storageFree() < STORAGE_WARN_BELOW ); }

void clearHealth() {
  Health[_SIM]        = NA;
  Health[_PROVIDER]   = NA;
  Health[_GPRS]       = NA;
  Health[_WIFI]       = NA;
  
  Health[_NETWORK]    = NA;
  Health[_CONNECTION] = NA;
  
  Health[_TLS]        = NA;
  Health[_MQTT]       = NA;
  
  Health[_MEM]        = NA;  
  Health[_STORAGE]    = NA;
}
