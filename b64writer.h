/*
 * B64writer
 * 
 * Manages information conversion to a BASE64 output format -streamed-
 * We can remove this if we can use enough internal memory
 * 
 * SOURCE: https://forum.arduino.cc/index.php?topic=201007.0
 */
 
//#include "Arduino.h"
  
class Base64Writer : public Print{
  public:
  
    Base64Writer( Print &p_Out ) : Output( p_Out ), Cursor( 0x00 ) {}
    ~Base64Writer( void ) { flush(); }
    
    void flush();
    size_t write( uint8_t u_Data );
      
  protected:
  
    void Step( void );
    void Convert( void );
  
    uint8_t Data[ 0x04 ];
    Print &Output;
    char Cursor;
};

char b64[ 64 ] PROGMEM = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};  

void Base64Writer::flush()
{
  if( !Cursor ) return;
  Convert();
  switch( Cursor ){
    case 1:  Data[ 0x02 ] = '=';
    case 2:  Data[ 0x03 ] = '=';
  }
  Step();
}

size_t Base64Writer::write( uint8_t u_Data )
  {
    if( Cursor == 0x03 ){
      Convert();
      Step();
    }
    Data[ Cursor++ ] = u_Data;
    return 0x01;
  }
  
void Base64Writer::Step( void )
  {
    Output.write( Data, 0x04 );
    memset( Data, 0x00, 0x04 );
    Cursor = 0x00;
  }

void Base64Writer::Convert( void )
{
  union{
    uint8_t Input[ 0x03 ];
    struct{
      unsigned int D : 0x06;
      unsigned int C : 0x06;
      unsigned int B : 0x06;
      unsigned int A : 0x06;
    } Output;
  } B64C = { { Data[ 2 ], Data[ 1 ], Data[ 0 ] } };

  Data[ 0x00 ] = pgm_read_byte( &b64[ B64C.Output.A ] );
  Data[ 0x01 ] = pgm_read_byte( &b64[ B64C.Output.B ] );
  Data[ 0x02 ] = pgm_read_byte( &b64[ B64C.Output.C ] );
  Data[ 0x03 ] = pgm_read_byte( &b64[ B64C.Output.D ] );
}    
