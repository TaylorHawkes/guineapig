#include <SoftwareSerial.h>
#include <ServoTimer2.h>
//#include <AltSoftSerial.h>

SoftwareSerial gsm(2, 3);//RX,TX

//AltSoftSerial gsm;

void setup() {
  Serial.begin(4800);
  gsm.begin(4800);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (gsm.available() > 0){
    Serial.write(gsm.read());
  }

  while (Serial.available() > 0) {
      
   // Serial.write(Serial.read());
    //so before we write we detach then we reattch
      //detach();
      gsm.write(Serial.read());
      //attach();
     // gsm.write("AT\n");
  }
}

