
//we start at the top,then move to bottom within 2 seconds of turning on to set throttle range.
int value = 0; // set values you need to zero
int input_value = 0;
int min=0;
int max=255;

int br=10;
int fl=7;  

void setup() {

//  int fl=7;  
 // int fr=8;
 // int bl=6;

  Serial.begin(4800);    // start serial at 9600 baud
 
//analogWrite(fl,value);
//analogWrite(fr,value);
//analogWrite(bl,value);
  analogWrite(br,value);
  analogWrite(fl,value);

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
  //  fl.write(value);
  //  fr.write(value);
  //  bl.write(value);
      analogWrite(br,value);
      analogWrite(fl,value);
    }
  }
    
}
 
