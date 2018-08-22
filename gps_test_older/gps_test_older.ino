//3rd party library: TinyGPS++ from: http://arduiniana.org/libraries/tinygpsplus/
#include <TinyGPS++.h>

TinyGPSPlus gps;


// U-blox NMEA text commands

const char disableRMC[] PROGMEM = "PUBX,40,RMC,0,0,0,0,0,0";
const char disableGLL[] PROGMEM = "PUBX,40,GLL,0,0,0,0,0,0";
const char disableGSV[] PROGMEM = "PUBX,40,GSV,0,0,0,0,0,0";
const char disableGSA[] PROGMEM = "PUBX,40,GSA,0,0,0,0,0,0";
const char disableGGA[] PROGMEM = "PUBX,40,GGA,0,0,0,0,0,0";
const char disableVTG[] PROGMEM = "PUBX,40,VTG,0,0,0,0,0,0";
const char disableZDA[] PROGMEM = "PUBX,40,ZDA,0,0,0,0,0,0";

const char enableRMC[] PROGMEM = "PUBX,40,RMC,0,1,0,0,0,0";
const char enableGLL[] PROGMEM = "PUBX,40,GLL,0,1,0,0,0,0";
const char enableGSV[] PROGMEM = "PUBX,40,GSV,0,1,0,0,0,0";
const char enableGSA[] PROGMEM = "PUBX,40,GSA,0,1,0,0,0,0";
const char enableGGA[] PROGMEM = "PUBX,40,GGA,0,1,0,0,0,0";
const char enableVTG[] PROGMEM = "PUBX,40,VTG,0,1,0,0,0,0";
const char enableZDA[] PROGMEM = "PUBX,40,ZDA,0,1,0,0,0,0";

const char baud9600  [] PROGMEM = "PUBX,41,1,3,3,9600,0";
const char baud38400 [] PROGMEM = "PUBX,41,1,3,3,38400,0";
const char baud57600 [] PROGMEM = "PUBX,41,1,3,3,57600,0";
const char baud115200[] PROGMEM = "PUBX,41,1,3,3,115200,0";

//--------------------------

const uint32_t COMMAND_DELAY = 250;

void changeBaud( const char *textCommand, unsigned long baud )
{
//gps.send_P( &tee, (const __FlashStringHelper *) disableRMC );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableGLL );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableGSV );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableGSA );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableGGA );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableVTG );
//delay( COMMAND_DELAY );
//gps.send_P( &tee, (const __FlashStringHelper *) disableZDA );
//delay( 500 );
//gps.send_P( &tee, (const __FlashStringHelper *) textCommand );
//Serial1.flush();
//Serial1.end();

//Serial.print( F("All sentences disabled for baud rate ") );
//Serial.print( baud );
//Serial.println( F(" change.  Enter '1' to reenable sentences.") );
//delay( 500 );
//Serial1.begin( baud );

} // changeBaud



void sendUBX( const unsigned char *progmemBytes, size_t len )
{
  Serial1.write( 0xB5 ); // SYNC1
  Serial1.write( 0x62 ); // SYNC2

  uint8_t a = 0, b = 0;
  while (len-- > 0) {
	uint8_t c = pgm_read_byte( progmemBytes++ );
	a += c;
	b += a;
	Serial1.write( c );
  }

  Serial1.write( a ); // CHECKSUM A
  Serial1.write( b ); // CHECKSUM B

} // sendUBX


void init_gps() {

  Serial.println(F("Initializing GPS. Please wait..."));
  

  Serial1.begin(9600);

  Serial.println(F("GPS Initialized!"));
//////  Serial1.println("
////  unsigned int header = 0xB562;
////  unsigned int ID = 0x0608;
////  unsigned int msgSize = 6;
////  unsigned int measRate = 200; //Measurement Rate, GPS measurements are taken every measRate milliseconds
////  unsigned int navRate = 1; //Navigation Rate, in number of measurement cycles. On u-blox 5 and u-blox 6, this parameter cannot be changed, and is always equals 1.
////  unsigned int timeRef = 1; //Alignment to reference time: 0 = UTC time, 1 = GPS time
////  
////  unsigned char msg[11];
////  msg[0] = header>>8;
////  msg[1] = header&0xFF;
////  msg[2] = ID>>8;
////  msg[3] = ID&0xFF;
////  msg[4] = msgSize;
////  msg[5] = measRate>>8;
////  msg[6] = measRate&0xFF;
////  msg[7] = navRate>>8;
////  msg[8] = navRate&0xFF;
////  msg[9] = timeRef>>8;
////  msg[10] = timeRef&0xFF;

////  //calculate 8-bit Fletcher Algorithm checksum
////  unsigned char CK_A = 0;
////  unsigned char CK_B = 0;

////  for( int i=0; i<11; i++) {
////	CK_A = CK_A + msg[i]; //THG: checksum calculation wrong
////	CK_B = CK_B + CK_A;
////  }

////  for( int i=0; i<11; i++ ) {
////	Serial1.write(msg[i]);
////  }

////	Serial1.write(msg, 11);
////	Serial1.write(CK_A);
////	Serial1.write(CK_B);

////  Serial.print("MSG: ");
////  for( int i=0; i<11; i++ ) {
////	Serial.print(msg[i], HEX);
////	Serial.print(" ");
////  }
////  Serial.print(CK_A, HEX);
////  Serial.println(CK_B, HEX);

//while(true) {
//  while( Serial1.available() > 0 ) {
//    Serial.print(Serial1.read());
//  }
//}

const unsigned char ubxRate1Hz[] PROGMEM = { 0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00 };
const unsigned char ubxRate5Hz[] PROGMEM = { 0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00 };
//const unsigned char ubxRate5Hz[] PROGMEM = { 0x06,0x08,0x06,0x00,200,0x00,0x01,0x00,0x01,0x00 };
//const unsigned char ubxRate5Hz[] PROGMEM = { 0x06,0x08,0x06,0x00,200,0x00,0x01,0x00,0x01,0x00 };
//const unsigned char ubxRate5Hz[] PROGMEM = { 0x06,0x08,0x06,0x00,200,0x00,0x01,0x00,0x01,0x00 };

////  gpsPort.flush();
////  gpsPort.end();

////  DEBUG_PORT.print( F("All sentences disabled for baud rate ") );
////  DEBUG_PORT.print( baud );
////  DEBUG_PORT.println( F(" change.  Enter '1' to reenable sentences.") );

//delay( 500 );
//gpsPort.begin(115200UL);

//sendUBX(ubxRate5Hz, sizeof(ubxRate5Hz));




}

void setup(){
Serial.begin(9600);
init_gps();
}
void loop(){

//      const char *gpsStream =
//   // "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
//    "$GPGGA,045104.000,3014.1998,N,11705.8173,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n";

//    while (*gpsStream){
//          if (gps.encode(*gpsStream++)){
//              Serial.print(gps.location.lat(),10);
//              Serial.print(",");
//              Serial.println(gps.location.lng(),10);

//          }
//      }
//  delay(10000000);

while (Serial1.available() > 0) {
	//Serial.write(Serial1.read());
 // gps.encode(Serial1.read());
}
  
    if (gps.location.isUpdated()){
                Serial.print("--------------");
                Serial.print(gps.location.lat(),10);
                Serial.print(",");
                Serial.println(gps.location.lng(),10);
    }
}


float convert_to_decimal_degrees(float f) {
  int d = (int) f / 100;
  f -= d * 100;
  return (f / 60.0) + d;
}


