#include <AltSoftSerial.h>

AltSoftSerial gpsSerial;
const int sentenceSize = 80;

char sentence[sentenceSize];

void setup()
{

Serial.begin(57600);
gpsSerial.begin(57600);

}
void loop(){

  if (gpsSerial.available()){
      Serial.write(gpsSerial.read());
  }
    
}


