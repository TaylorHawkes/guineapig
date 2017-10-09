// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

//#include <Servo.h>
#include <Wire.h> //It think this uses Timer2
#include <ServoTimer2.h>
#include <SoftwareSerial.h> //I think this uses Timer1

SoftwareSerial gsm(2, 3);
ServoTimer2 fl,fr,bl,br; 
//Servo fl,fr,bl,br; 

int min=1000;//set in ~/Documents/Arduino/libraries/ServoTimer2/ServoTimer2.h
int max=2000;//set in ~/Documents/Arduino/libraries/ServoTimer2/ServoTimer2.h

int input_value = 0;
int hover_speed=1000;
int stop=0;
int start=0;
char cc;


int fr_power_int=0;
int fl_power_int=0;
int br_power_int=0;
int bl_power_int=0;

int old_fr=0;
int old_fl=0;
int old_bl=0;
int old_br=0;



void setup()
{
  Serial.begin(4800);           // start serial for output 
  Serial.println("starting");
  gsm.begin(4800);

    fl.attach(7);   
    fr.attach(8);
    bl.attach(10);//pin 9 seems broke
    br.attach(6);

    fl.write(hover_speed);
    fr.write(hover_speed);
    bl.write(hover_speed);
    br.write(hover_speed);

     startTCPServer();
//   while(Serial.available() == 0){}
//    input_value = Serial.parseInt();    // Parse an Integer from Serial 
//      if(input_value==666){
//        Serial.println("Starting:");
//        Wire.begin(4);       
//       Wire.onReceive(receiveEvent); // register even  
//  }
}

//using the SIM5320A - (with antena) and hologram
void startTCPServer(){
  gsm.write("AT+CGDCONT=1,\"IP\",\"hologram\",\"0.0.0.0\"\r"); res();
  Serial.println("starting socket server");
  gsm.write("AT+CGSOCKCONT=1,\"IP\",\"hologram\"\r"); res();
  gsm.write("AT+CSOCKSETPN=1\r"); res();
  gsm.write("AT+CIPMODE=0\r"); res();
  gsm.write("AT+NETOPEN\r"); res();
  //this is for tcp
  //	gsm.write("AT+SERVERSTART=8080,0\r"); res();
  //I think this (udp) may be a little quicker
  gsm.write("AT+CIPOPEN=0,\"UDP\",,,9000\r"); res();
}

void res(){
  delay(5000);
  while (gsm.available() > 0){
     Serial.write(gsm.read());
  };
}

void loop()
{
if(start){
     int new_fr=set_range(hover_speed+fr_power_int);
     if(new_fr!=old_fr){
        fr.write(new_fr);
        old_fr=new_fr;
     }

     int new_fl=set_range(hover_speed+fl_power_int);
     if(new_fl!=old_fl){
        fl.write(new_fl);
        old_fl=new_fl;
     }

     int new_bl=set_range(hover_speed+bl_power_int);
     if(new_bl!=old_bl){
        bl.write(new_bl);
        old_bl=new_bl;
     }

     int new_br=set_range(hover_speed+br_power_int);
     if(new_br!=old_br){
        br.write(new_br);
        old_br=new_br;
     }
}

 //Taylor consider removing htis not sure if it helps , hurts does, does anything
 //1000fps max
 delayMicroseconds(1000);

 while (gsm.available() > 0){
   String c=gsm.readString();
   Serial.println("--read something---");
   Serial.println(c);
   int ipIndex = c.indexOf("+IPD");
   int nlIndex = c.indexOf("\n",ipIndex);
   String command = c.substring(nlIndex+1);
   Serial.println("command:"+command);

   if(command=="start"){
     Serial.println("Starting:");
        start=1;
      if(stop){
          hover_speed=1000;
          stop=0;
      }else{
        Wire.begin(4);       
        Wire.onReceive(receiveEvent); // register even  
      }
   }else if(command=="stop"){
       hover_speed=1000;
       fr.write(hover_speed);
       fl.write(hover_speed);
       br.write(hover_speed);
       bl.write(hover_speed);
       stop=1;
       start=0;
   }else{
     int new_speed=command.toInt(); 
     if(new_speed > 100 && new_speed < 2000){
       hover_speed = new_speed;
       fr.write(hover_speed);
       fl.write(hover_speed);
       br.write(hover_speed);
       bl.write(hover_speed);
     }
   }
 }
   //delay(10);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
//it may the serial.print that is messing us up???
void receiveEvent(int howMany){

char fr_power[5]; 
char fl_power[5];
char br_power[5];
char bl_power[5];

int l=0;
int p=0;
while(1 < Wire.available())
{
char c = Wire.read(); 
 if(c==','){
     if(p==0){
       fr_power[l]='\0';
     }else if(p==1){
       fl_power[l]='\0';
     }else if(p==2){
       br_power[l]='\0';
     }else if(p==3){
       bl_power[l]='\0';
     }
     l=0;
     p++;
  continue;
 }else if(p==0){
   fr_power[l]=c;
 }else if(p==1){
   fl_power[l]=c;
 }else if(p==2){
   br_power[l]=c;
 }else if(p==3){
   bl_power[l]=c;
 }
 l++;
}

fr_power_int=atoi(fr_power);
fl_power_int=atoi(fl_power);
br_power_int=atoi(br_power);
bl_power_int=atoi(bl_power);


int x = Wire.read();// receive byte as an integer
   
}

int set_range(int r){
	if(r > 2000) {
			 return 2000;
	}
	if(r < 1070) {
		return 1070;
	}
	return r;
}


