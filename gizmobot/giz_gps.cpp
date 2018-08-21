#include "HardwareSerial.h"
#include "giz_gps.h"

static char temp_heading[200];
static int c=0; 

void GizGps::init(){
    Serial2.begin(9600);
    Serial.begin(57600);

}

void GizGps::update(){

  if (Serial2.available()){
      Serial.write(Serial2.read());
  }

//  if(int l=Serial2.available()){
//          for (int i = 0; i < l; i++){
//             char t= Serial2.read();
//             if(t=='\n'){
//                 temp_heading[c]='\0';
//                 heading=atof(temp_heading);
//                 c=0;
//            }
//        temp_heading[c]=t;
//        c++;
//      }
//  }
}
