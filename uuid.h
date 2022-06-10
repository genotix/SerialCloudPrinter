/*
 * UUID
 * 
 * Manages some translations of special types and the usage of GUID for messages
 */

//#define DEBUG_GUID


#define THINGNAME_SIZE  13
char DeviceGUIDBuffer[THINGNAME_SIZE];
char * DeviceGUIDptr = DeviceGUIDBuffer;


typedef uint8_t uuid_t[16];
#define UUID_STR_LEN 37

void uuid_generate(uuid_t out)
{
    esp_fill_random(out, sizeof(uuid_t));

    /* uuid version */
    out[6] = 0x40 | (out[6] & 0xF);

    /* uuid variant */
    out[8] = (0x80 | out[8]) & ~0x40;
}

static uint8_t unhex_char(unsigned char s)
{
    if (0x30 <= s && s <= 0x39) { /* 0-9 */
        return s - 0x30;
    } else if (0x41 <= s && s <= 0x46) { /* A-F */
        return s - 0x41 + 0xa;
    } else if (0x61 <= s && s <= 0x69) { /* a-f */
        return s - 0x61 + 0xa;
    } else {
        /* invalid string */
        return 0xff;
    }
}

static int unhex(unsigned char *s, size_t s_len, unsigned char *r)
{
    int i;

    for (i = 0; i < s_len; i += 2) {
        uint8_t h = unhex_char(s[i]);
        uint8_t l = unhex_char(s[i + 1]);
        if (0xff == h || 0xff == l) {
            return -1;
        }
        r[i / 2] = (h << 4) | (l & 0xf);
    }

    return 0;
}

int uuid_parse(const char *in, uuid_t uu)
{
    const char *p = in;
    uint8_t *op = (uint8_t *)uu;

    if (0 != unhex((unsigned char *)p, 8, op)) {
        return -1;
    }
    p += 8;
    op += 4;

    for (int i = 0; i < 3; i++) {
        if ('-' != *p++ || 0 != unhex((unsigned char *)p, 4, op)) {
            return -1;
        }
        p += 4;
        op += 2;
    }

    if ('-' != *p++ || 0 != unhex((unsigned char *)p, 12, op)) {
        return -1;
    }
    p += 12;
    op += 6;

    return 0;
}


String uuid_to_string(const uuid_t uu) {
    String tempString = "";
    for (int i = 0 ; i < 16 ; i++ ) {
      if ( i == 4 || i == 6 || i == 8 || i == 10 ) tempString = tempString + "-";
      tempString = tempString + uu[i];
    }
    return tempString;
}


char * DeviceGUID() {
  uint8_t baseMac[6];
  if ( esp_read_mac(baseMac, ESP_MAC_WIFI_STA) == ESP_OK ) {
   // Get MAC address for WiFi
    snprintf(DeviceGUIDBuffer, THINGNAME_SIZE, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);        
    return DeviceGUIDBuffer;
  } 

  return NULL;

}

#define MacAddressSize  18

String DeviceMAC(esp_mac_type_t sourceCard=ESP_MAC_WIFI_STA) {
  String DeviceID;
  uint8_t baseMac[6];

  if ( esp_read_mac(baseMac, sourceCard ) == ESP_OK ) {

   // Get MAC address for WiFi
    char baseMacChr[MacAddressSize] = { 0 };
    snprintf(baseMacChr, MacAddressSize, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    
    DeviceID = String(baseMacChr);    
    return DeviceID;
  } else {
    return String("unknown");
  }
}

void uuid_unparse(const uuid_t uu, char *out)
{
    snprintf(out, UUID_STR_LEN,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
             "%02x%02x-%02x%02x%02x%02x%02x%02x",
             uu[0], uu[1], uu[2], uu[3], uu[4], uu[5], uu[6], uu[7], uu[8], uu[9], uu[10], uu[11],
             uu[12], uu[13], uu[14], uu[15]);
}


String get_uuid() {
  char string[40];
  
  uuid_t uuidBuffer;
  uuid_generate(uuidBuffer);
  uuid_unparse(uuidBuffer,string);
//  printf("%s", string); //null terminated string

  #ifdef DEBUG_GUID
    SerialMon.print(F("Generated GUID "));
    SerialMon.println(string);
  #endif
  
  return string;
}
