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

ServoTimer2 front,right,back,left; 

int min=1000;//set in ~/Documents/Arduino/libraries/ServoTimer2/ServoTimer2.h
int max=2000;//set in ~/Documents/Arduino/libraries/ServoTimer2/ServoTimer2.h

int input_value = 0;
int hover_speed=1000;
char p_gain[5];
char d_gain[5];

int stop=0;
int start=0;
char cc;
char udp_request[255];

int front_power_int=0;
int right_power_int=0;
int back_power_int=0;
int left_power_int=0;

int old_front=0;
int old_right=0;
int old_back=0;
int old_left=0;
int read_line_count=0;


void setup()
{

p_gain[0]='0';
p_gain[1]='0';
p_gain[2]='0';
p_gain[3]='0';
p_gain[4]='\0';


d_gain[0]='0';
d_gain[1]='0';
d_gain[2]='0';
d_gain[3]='0';
d_gain[4]='\0';



  Serial.begin(4800);           // start serial for output 
  Serial.println("starting");
  gsm.begin(4800);

    front.attach(10);   
    right.attach(8);
    back.attach(7);
    left.attach(6);

    front.write(hover_speed);
    right.write(hover_speed);
    back.write(hover_speed);
    left.write(hover_speed);

     startTCPServer();

     Wire.begin(4);       
     Wire.onReceive(receiveEvent); // register even  
     Wire.onRequest(requestEvent); // register even  
}

//using the SIM5320A - (with antena) and hologram
void startTCPServer(){

delay(20000);
gsm.write("AT+CIPMODE=1\r"); res();
gsm.write("AT+NETOPEN\r"); res();
gsm.write("AT+CIPOPEN=0,\"UDP\",\"23.92.20.100\",9999,9000\r"); res();
}

void res(){
  delay(5000);
  while (gsm.available() > 0){
     Serial.write(gsm.read());
  };
}

int loop_count=0;
int read_input=0;
int read_input_count=0;
char new_hover_rate[5];

int read_input_p=0;
int read_input_count_p=0;
int correct_udp_line_length=16;
void loop()
{
   loop_count++;

  if(hover_speed > 900){
      int new_front=set_range(hover_speed+front_power_int);
      if(new_front!=old_front){
        front.write(new_front);
        old_front=new_front;
      }

      int new_right=set_range(hover_speed+right_power_int);
      if(new_right!=old_right){
        right.write(new_right);
        old_right=new_right;
      }

      int new_back=set_range(hover_speed+back_power_int);
      if(new_back!=old_back){
        back.write(new_back);
        old_back=new_back;
      }

      int new_left=set_range(hover_speed+left_power_int);
      if(new_left!=old_left){
        left.write(new_left);
        old_left=new_left;
      }
  }

  delayMicroseconds(1000);//micros sends in 1 millions of a second

  if(loop_count > 2)//loopcount will be in 1 thousnad of a second 100=1/10nth of second 
  {
    gsm.write("ok"); 
//    int bytes = gsm.available();
//    int i;
//    for(i = 0; i< bytes; i++){
//  	udp_request[i]= gsm.read();
//    }
//  if(bytes){
//    udp_request[i+1]='\0';
//   char * udp_line;
//udp_line=strtok(udp_request,"|");
//while (udp_line != NULL)
//{
//  if(strlen(udp_line)==correct_udp_line_length){
//  		//the gets Throttle
//  		if(udp_line[3]=='t' && udp_line[4]==':' && 
//  		is_number_char(udp_line[5]) &&
//  		is_number_char(udp_line[6]) &&
//  		is_number_char(udp_line[7]) &&
//  		is_number_char(udp_line[8])  &&
//  		//verify duplicate number no bad data
//  		udp_line[10]==udp_line[3] &&
//  		udp_line[12]==udp_line[5] &&
//  		udp_line[13]==udp_line[6] &&
//  		udp_line[14]==udp_line[7] &&
//  		udp_line[15]==udp_line[8] 
//  		){
//  			char new_hover_rate[5];
//  			new_hover_rate[0]=udp_line[5];
//  			new_hover_rate[1]=udp_line[6];
//  			new_hover_rate[2]=udp_line[7];
//  			new_hover_rate[3]=udp_line[8];
//  			new_hover_rate[4]='\0';
//  			int new_hover_rate_int=atoi(new_hover_rate);
//  			if(new_hover_rate_int != hover_speed){
//  				 hover_speed=new_hover_rate_int;
//  				 Serial.print("hover speed changed:");
//  				 Serial.println(hover_speed);
//  			 }
//  		}

//  		//the gets P gain
//  		if(udp_line[3]=='p' && udp_line[4]==':' && 
//  		is_number_char(udp_line[5]) &&
//  		is_number_char(udp_line[6]) &&
//  		is_number_char(udp_line[7]) &&
//  		is_number_char(udp_line[8])  &&
//  		//verify duplicate number no bad data
//  		udp_line[10]==udp_line[3] &&
//  		udp_line[12]==udp_line[5] &&
//  		udp_line[13]==udp_line[6] &&
//  		udp_line[14]==udp_line[7] &&
//  		udp_line[15]==udp_line[8] 
//  		){
//  			char new_p_gain[5];
//  			new_p_gain[0]=udp_line[5];
//  			new_p_gain[1]=udp_line[6];
//  			new_p_gain[2]=udp_line[7];
//  			new_p_gain[3]=udp_line[8];
//  			new_p_gain[4]='\0';
//  			if(strcmp(p_gain, new_p_gain) != 0){
//                  strncpy(p_gain, new_p_gain, 5);
//  			//	 p_gain=new_p_gain;
//  				 Serial.print("p_gain changed:");
//  				 Serial.println(p_gain);
//  			 }
//  		}

//  		//the gets D gain
//  		if(udp_line[3]=='d' && udp_line[4]==':' && 
//  		is_number_char(udp_line[5]) &&
//  		is_number_char(udp_line[6]) &&
//  		is_number_char(udp_line[7]) &&
//  		is_number_char(udp_line[8])  &&
//  		//verify duplicate number no bad data
//  		udp_line[10]==udp_line[3] &&
//  		udp_line[12]==udp_line[5] &&
//  		udp_line[13]==udp_line[6] &&
//  		udp_line[14]==udp_line[7] &&
//  		udp_line[15]==udp_line[8] 
//  		){
//  			char new_d_gain[5];
//  			new_d_gain[0]=udp_line[5];
//  			new_d_gain[1]=udp_line[6];
//  			new_d_gain[2]=udp_line[7];
//  			new_d_gain[3]=udp_line[8];
//  			new_d_gain[4]='\0';
//  			//int new_d_gain_int=atoi(new_d_gain);
//  			if(strcmp(d_gain, new_d_gain) != 0){
//                  strncpy(d_gain, new_d_gain, 5);
//  				// d_gain=new_d_gain;
//  				 Serial.print("d_gain changed:");
//  				 Serial.println(d_gain);
//  			 }
//  		}
//   }

//      udp_line = strtok (NULL, "|");
//	  }
	

//	}
      loop_count=0;
  }

return;
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
//it may the serial.print that is messing us up???
void receiveEvent(int howMany){

char front_power[5]; 
char right_power[5];
char back_power[5];
char left_power[5];

int l=0;
int p=0;
while(1 < Wire.available())
{
char c = Wire.read(); 
 if(c==','){
     if(p==0){
       front_power[l]='\0';
     }else if(p==1){
       right_power[l]='\0';
     }else if(p==2){
       back_power[l]='\0';
     }else if(p==3){
       left_power[l]='\0';
     }
     l=0;
     p++;
  continue;
 }else if(p==0){
   front_power[l]=c;
 }else if(p==1){
   right_power[l]=c;
 }else if(p==2){
   back_power[l]=c;
 }else if(p==3){
   left_power[l]=c;
 }
 l++;
}

front_power_int=atoi(front_power);
right_power_int=atoi(right_power);
back_power_int=atoi(back_power);
left_power_int=atoi(left_power);


int x = Wire.read();// receive byte as an integer
   
}

int set_range(int r){
	if(r > 2000) {
         return 2000;
	}
	if(r < 1000) {
		return 1000;
	}
	return r;
}
bool is_number_char(char a){
return ( a=='0'|| a=='1'|| a=='2'|| a=='3'|| a=='4'|| a=='5'|| a=='6'|| a=='7'|| a=='8'|| a=='9');
}

void requestEvent() 
{
//String p_gain_string="";
//if(p_gain < 10 ){
//    p_gain_string+="000"+String(p_gain);
//}else if(p_gain < 100){
//    p_gain_string+="00"+String(p_gain);
//}else if(p_gain < 1000){
//    p_gain_string+="0"+String(p_gain);
//}else{
//    p_gain_string+=String(p_gain);
//}
//String d_gain_string="";
//if(d_gain < 10 ){
//    d_gain_string+="000"+String(d_gain);
//}else if(d_gain < 100){
//    d_gain_string+="00"+String(d_gain);
//}else if(d_gain < 1000){
//    d_gain_string+="0"+String(d_gain);
//}else{
//    d_gain_string+=String(d_gain);
//}

//  String p_gain_string_final=p_gain_string+","+d_gain_string+"\r\n";
// Serial.println(p_gain_string_final);
//  int str_len = p_gain_string_final.length() + 1; 
//  char __p_gain[str_len];
//  p_gain_string_final.toCharArray(__p_gain,str_len);
    char gain[9];
    gain[0]=p_gain[0];
    gain[1]=p_gain[1];
    gain[2]=p_gain[2];
    gain[3]=p_gain[3];
    gain[4]=',';
    gain[5]=d_gain[0];
    gain[6]=d_gain[1];
    gain[7]=d_gain[2];
    gain[8]=d_gain[3];

    Wire.write(gain);
 //  Wire.write("ok");
}


