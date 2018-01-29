#include <SoftwareSerial.h>

SoftwareSerial mySerial(8, 9); // RX, TX  
// Connect HM10      Arduino Uno
//     Pin 1/TXD          Pin 7
//     Pin 2/RXD          Pin 8

double dt;//change in us
float last_micros=0;
void setup() {  
  Serial.begin(9600);
  // If the baudrate of the HM-10 module has been updated,
  // you may need to change 9600 by another value
  // Once you have found the correct baudrate,
  // you can update it using AT+BAUDx command 
  // e.g. AT+BAUD0 for 9600 bauds
  mySerial.begin(9600);
}

void loop() {  
  char c;
  if (Serial.available()) {
    c = Serial.read();
    //mySerial.print(c);
  }
  if (mySerial.available()){
    c = mySerial.read();
    Serial.print(c);    
     if(c=='<'){
        Serial.println("");    
        //Serial.println((micros()-last_micros)/1000);    
        //Serial.println((micros()-last_micros)/1000);    
        last_micros=micros();
    }
  }
}

