//3rd party library: TinyGPS++ from: http://arduiniana.org/libraries/tinygpsplus/
#include <TinyGPS++.h>

TinyGPSPlus gps;

void init_gps() {

  Serial.println(F("Initializing GPS. Please wait..."));
  
  Serial1.begin(9600);

  Serial.println(F("GPS Initialized!"));
  

//  Serial1.println("
  unsigned int header = 0xB562;
  unsigned int ID = 0x0608;
  unsigned int msgSize = 6;
  unsigned int measRate = 200; //Measurement Rate, GPS measurements are taken every measRate milliseconds
  unsigned int navRate = 1; //Navigation Rate, in number of measurement cycles. On u-blox 5 and u-blox 6, this parameter cannot be changed, and is always equals 1.
  unsigned int timeRef = 1; //Alignment to reference time: 0 = UTC time, 1 = GPS time
  
  unsigned char msg[11];
  msg[0] = header>>8;
  msg[1] = header&0xFF;
  msg[2] = ID>>8;
  msg[3] = ID&0xFF;
  msg[4] = msgSize;
  msg[5] = measRate>>8;
  msg[6] = measRate&0xFF;
  msg[7] = navRate>>8;
  msg[8] = navRate&0xFF;
  msg[9] = timeRef>>8;
  msg[10] = timeRef&0xFF;

  //calculate 8-bit Fletcher Algorithm checksum
  unsigned char CK_A = 0;
  unsigned char CK_B = 0;
  for( int i=0; i<11; i++) {
    CK_A = CK_A + msg[i]; //THG: checksum calculation wrong
    CK_B = CK_B + CK_A;
  }

  for( int i=0; i<11; i++ ) {
    Serial1.write(msg[i]);
  }



    Serial1.write(msg, 11);
    Serial1.write(CK_A);
    Serial1.write(CK_B);

  Serial.print("MSG: ");
  for( int i=0; i<11; i++ ) {
    Serial.print(msg[i], HEX);
    Serial.print(" ");
  }
  Serial.print(CK_A, HEX);
  Serial.println(CK_B, HEX);

//while(true) {
//  while( Serial1.available() > 0 ) {
//    Serial.print(Serial1.read());
//  }
//}

}

void setup(){
Serial.begin(9600);
init_gps();
}
void loop(){

while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
}

if (gps.location.isUpdated()){
    Serial.print(gps.location.lat(),10);
    Serial.print(",");
    Serial.println(0-gps.location.lng(),10);
}
}


float convert_to_decimal_degrees(float f) {
  int d = (int) f / 100;
  f -= d * 100;
  return (f / 60.0) + d;
}


