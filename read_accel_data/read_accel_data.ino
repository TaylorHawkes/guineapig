// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

#include <Wire.h>
#include <Servo.h>

Servo fl,fr,bl,br; 

int min=1000;
int max=2000;
float value=1000;
int input_value = 0;
void setup()
{
  fl.attach(7,min,max);   
 // fr.attach(8,min,max);
 // bl.attach(9,min,max);
  br.attach(10,min,max);
  
  fl.writeMicroseconds(value);
  //fr.writeMicroseconds(value);
  //bl.writeMicroseconds(value);
  br.writeMicroseconds(value);
  Serial.begin(9600);           // start serial for output 
  Serial.println("Write 666 to begin:");
  while(Serial.available() == 0){}
  
  input_value = Serial.parseInt();    // Parse an Integer from Serial 
    if(input_value==666){
      Serial.println("Starting:");
      Wire.begin(4);       
      Wire.onReceive(receiveEvent); // register even  
    }

}

void loop()
{
  //delay(10);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    String c = Wire.readStringUntil('\n'); // receive byte as a character
    
    // Serial.println(c);
    int commaIndex = c.indexOf(',');
    int secondCommaIndex = c.indexOf(',', commaIndex + 1);
    int thirdCommaIndex = c.indexOf(',', secondCommaIndex + 1);

    
    String time = c.substring(0, commaIndex);
    String fl_power = c.substring(commaIndex + 1, secondCommaIndex);
    String br_power = c.substring(secondCommaIndex + 1,thirdCommaIndex); 
    //Serial.println(bl_power);
    Serial.print((int) fl_power.toFloat());
    Serial.print('|');
    Serial.print((int) br_power.toFloat());
    Serial.println(' ');

    int fl_power_int=(int) fl_power.toFloat();
    int br_power_int= (int) br_power.toFloat();
      if(br_power_int > 1400 || br_power_int < 1170 || fl_power_int > 1400 || fl_power_int < 1170){
        Serial.print("- ---- Error -----");
        Serial.println(c);
      }else{
         fl.writeMicroseconds(fl_power_int);
         br.writeMicroseconds(br_power_int);
      }
    
  }
   
}
