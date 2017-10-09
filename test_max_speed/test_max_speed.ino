#include <ServoTimer2.h> 

//we start at the top,then move to bottom within 2 seconds of turning on to set throttle range.
int value = 1000; // set values you need to zero
int input_value = 0;
int min=1000;
int max=2000;

ServoTimer2 fl;
ServoTimer2 fr;
ServoTimer2 bl;
ServoTimer2 br;

void setup() {

  fl.attach(7);   
  fr.attach(8);
  bl.attach(6);
  br.attach(10);

  Serial.begin(4800);    // start serial at 9600 baud
 
  fl.write(value);
  fr.write(value);
  bl.write(value);
  br.write(value);
 

}

void loop() {

  if(Serial.available()) {
    input_value = Serial.parseInt();    // Parse an Integer from Serial 
      if(input_value > 0){
      value=input_value;
      Serial.print("Send value:");
      Serial.println(value);
   
    }
  }
 value =random(1070, 1180);
  fl.write(value);
  fr.write(value);
  bl.write(value);
  br.write(value); 
  //delayMicroseconds(1000);
    
}
 
