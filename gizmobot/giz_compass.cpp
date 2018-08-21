#include "HardwareSerial.h"
#include "giz_compass.h"


static char temp_heading[200];
static int c=0; 

void GizCompass::init(){
    Serial1.begin(57600);
}

void GizCompass::update(){
    if(int l=Serial1.available()){
            for (int i = 0; i < l; i++){
               char t= Serial1.read();
               if(t=='\n'){
                   temp_heading[c]='\0';
                   heading=atof(temp_heading);
                   c=0;
              }
          temp_heading[c]=t;
          c++;
        }
    }
}
