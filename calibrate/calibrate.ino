#include <ServoTimer2.h> 

//we start at the top,then move to bottom within 2 seconds of turning on to set throttle range.
int value = 1000; // set values you need to zero
int input_value = 0;

ServoTimer2 fl;
ServoTimer2 fr;
ServoTimer2 bl;
ServoTimer2 br;

void setup() {

  fl.attach(4);   
  fr.attach(5);
  bl.attach(6);
  br.attach(7);

  Serial.begin(9600);    // start serial at 9600 baud
 
  fl.write(value);
  fr.write(value);
  bl.write(value);
  br.write(value);
 

}

void loop() {
  
//First connect your ESC WITHOUT Arming. Then Open Serial and follo Instructions
 
 // fl.write(value);
  //fr.write(value);
 // bl.write(value);
  //br.write(val
  if(Serial.available()) {
    input_value = Serial.parseInt();    // Parse an Integer from Serial 
  
      if(input_value > 0){
      value=input_value;
      Serial.print("Send value:");
      Serial.println(value);
      fl.write(value);
      fr.write(value);
      bl.write(value);
      br.write(value); 
    }
  }
    
}
 
