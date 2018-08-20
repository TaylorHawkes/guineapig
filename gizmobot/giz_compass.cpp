#include "giz_compass.h"
#include <AltSoftSerial.h>

AltSoftSerial mySerial;

 char temp_heading[200];
 int c=0; 

void GizCompass::init(){
    mySerial.begin(57600);

}

//Update to most recent value, but dont block
//(may want to consider a little blocking)
//should be ran at 50hz or greater
void GizCompass::update(){
    if(int l=mySerial.available()){
            for (int i = 0; i < l; i++){
               char t= mySerial.read();
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
