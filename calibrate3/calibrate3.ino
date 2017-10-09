#include <Servo.h>

//we start at the top,then move to bottom within 2 seconds of turning on to set throttle range.
int value = 1000; // set values you need to zero
int input_value = 0;
int min=1000;
int max=2000;
Servo fl,fr,bl,br; //Create as much as Servoobject you want. You can controll 2 or more Servos at the same time

void setup() {

  fl.attach(7,min,max);   
  fr.attach(8,min,max);
  bl.attach(6,min,max);
  br.attach(10,min,max);

  Serial.begin(4800);    // start serial at 9600 baud
 
  fl.writeMicroseconds(value);
  fr.writeMicroseconds(value);
  bl.writeMicroseconds(value);
  br.writeMicroseconds(value);
 

}

void loop() {
  
//First connect your ESC WITHOUT Arming. Then Open Serial and follo Instructions
 
 // fl.write(value);
  //fr.writeMicroseconds(value);
 // bl.writeMicroseconds(value);
  //br.writeMicroseconds(val
  if(Serial.available()) {
    input_value = Serial.parseInt();    // Parse an Integer from Serial 
  
      if(input_value > 0){
      value=input_value;
      Serial.print("Send value:");
      Serial.println(value);
      fl.writeMicroseconds(value);
      fr.writeMicroseconds(value);
      bl.writeMicroseconds(value);
      br.writeMicroseconds(value); 
    }
  }
    
}
 
