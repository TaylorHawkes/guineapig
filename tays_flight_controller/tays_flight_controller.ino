#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#define SERIAL_BAUD_RATE 9600 // Serial port baud
#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h
//#include <SpeedTrig.h>
//#include <SoftwareSerial.h>

//Taylors Helper function
typedef struct {
    float x,y,z;                                                                              
} Vec;

typedef struct {                                                                              
    float x=0;
	float y=0;
	float z=0;
	float w=1;
} Quat;

//interupt pints

byte last_channel_1, last_channel_2, last_channel_3, last_channel_4;
unsigned long timer_1, timer_2, timer_3, timer_4, current_time;
int receiver_input[5];

Quat new_quat(float angle,Vec axis);
//Quat updateQuatByRotation(Quat& q, Vec& v,float seconds);
void updateQuatByRotation(Quat& q, Vec& v, float dt,Quat& w,Quat& nq);
void quat_mul(Quat& q1,Quat& q2,Quat& r);
void toQuaternion(Quat& q,double pitch, double roll, double yaw);
void v_mult_s(Vec& v,float m);
void quat_norm(Quat *q);
static void toEulerAngle(Quat& q, double& roll, double& pitch, double& yaw);
static void toEulerAngle2(Quat& q, double& roll, double& pitch, double& yaw);
//put the above into header file

int front, right, left, back,thrust;
float ccw_yaw_thrust;
float upward_accel;
unsigned long fr_timer,br_timer,bl_timer,fl_timer;
unsigned long loop_timer,esc_loop_timer;
int loop_count=0;

char udp_request[255];
//int correct_udp_line_length=48;
int correct_udp_line_length=19;//for bluetooth


double a,b,c;

MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class
unsigned short imu_update_rate=1000; //in hz
int gyro_address= 0x68; 
int temperature;
int acc_axis[4], gyro_axis[4], euler_axis[4];
double pid_i_total_roll_error = 0;
double pid_i_total_pitch_error = 0;
double pid_i_total_yaw_error = 0;



long acc_x, acc_y, acc_z, acc_total_vector;

float angle_yaw; 
float angle_roll, gyro_roll_input, pid_output_roll, pid_last_roll_d_error;
float angle_pitch, gyro_pitch_input, pid_output_pitch, pid_last_pitch_d_error;

float gyro_roll_calibrate_offset=0; //these can be hard coded 
float gyro_pitch_calibrate_offset=0; //this could be hard coded
float gyro_yaw_calibrate_offset=0; //this could be hard coded
float acc_total_vector_calibrate=0; //this could be hard coded
float acc_total_diff=0; 

float acc_roll_calibrate_offset=0;
float acc_pitch_calibrate_offset=0;
double dt;//change in us
double dt_p;//change in percantge 

Vec gyro;
Vec accel;
Vec pos;//x,y,z = y is height , x is left/right , z is front/back in meters
Vec vel;
Vec accel_gyro_delta;
Vec accel_euler;
Vec gyro_scaled;
Quat o;
Quat w;                                                                    
Quat nq;
Quat newOrientation;
Quat accelQuat;

float DEGREES_TO_RADIANS=0.01745329251; //multiplier PI/180
float GYRO_DPS =65.5;//https://cdn.sparkfun.com/assets/learn_tutorials/5/5/0/MPU9250REV1.0.pdf


double gyro_pitch_velocity, gyro_roll_velocity, gyro_yaw_velocity;
double gyro_pitch, gyro_roll, gyro_yaw;
double accel_pitch, accel_roll, accel_yaw;
bool calibrating=false;

float p_gain=500;//mult by radians 400
float d_gain=.025;//mult by raw velocity  
float i_gain=0;//.1;//mult by radians

float p_gain_thrust=2000;//.1;//mult by radians

float p_gain_yaw=100;//mult by radians
float d_gain_yaw=0;//mult by raw velocity  
float i_gain_yaw=0;//.1;//mult by radians



int base_thrust=1150;
float desired_roll=0;
float desired_pitch=0.00;
float desired_yaw=0;

//SoftwareSerial mySerial(8, 9); // RX, TX 

void setup() {
    thrust=0;
    pos.x=0;
    pos.y=0;
    pos.z=0;
    vel.x=0;
    vel.y=0;
    vel.z=0;

  Serial.begin(57600);
   //todo, do this better
   pinMode(4,OUTPUT);
   pinMode(5,OUTPUT);
   pinMode(6,OUTPUT);
   pinMode(7,OUTPUT);

//set the pin chnage interupts
  PCICR |= (1 << PCIE0);                                                    //Set PCIE0 to enable PCMSK0 scan.
  PCMSK0 |= (1 << PCINT0);                                                  //Set PCINT0 (digital input 8) to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT1);                                                  //Set PCINT1 (digital input 9)to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT2);                                                  //Set PCINT2 (digital input 10)to trigger an interrupt on state change.
  PCMSK0 |= (1 << PCINT3);                                                  //Set PCINT3 (digital input 11)to trigger an interrupt on state change.

	Wire.begin();
	Wire.setClock(1000000);//this appears to be as fast as we can go
	set_gyro_registers();
   // get_gyro_data(0);

int cal_int=0;

//for (cal_int = 0; cal_int < 1000 ; cal_int ++){                           //Wait 5 seconds before continuing.
//    PORTD |= B11110000;                                                     //Set digital poort 4, 5, 6 and 7 high.
//    delayMicroseconds(1000);                                                //Wait 1000us.
//    PORTD &= B00001111;                                                     //Set digital poort 4, 5, 6 and 7 low.
//    delayMicroseconds(4000);                                                //Wait 3000us.
//}

//cal_int=0;
////we then go into 1100 for five seconds
//for (cal_int = 0; cal_int < 1000 ; cal_int ++){                           //Wait 5 seconds before continuing.
//    PORTD |= B11110000;                                                     //Set digital poort 4, 5, 6 and 7 high.
//    delayMicroseconds(1100);                                                //Wait 1000us.
//    PORTD &= B00001111;                                                     //Set digital poort 4, 5, 6 and 7 low.
//    delayMicroseconds(3900);                                                //Wait 3000us.
//}

	/*CALIBRATION*/	
	//take 5 seconds to calibrate
int total_loops=(4 * 250);
float gyro_roll_total=0;
float gyro_pitch_total=0;
float gyro_yaw_total=0;
float acc_roll_total=0;
float acc_pitch_total=0;
float acc_total_vector_sum=0;
   calibrating=true;
   for (int i=0; i<total_loops; i++){
       delay(4);
       get_gyro_data(0);
       gyro_roll_total+=gyro_roll_velocity;
       gyro_pitch_total+=gyro_pitch_velocity;
       gyro_yaw_total+=gyro_yaw_velocity;
       acc_total_vector_sum+=acc_total_vector;
       //acc_roll_total+=accel.x;
       //acc_pitch_total+=accel.y;
   }
   calibrating=false;

	gyro_roll_calibrate_offset=gyro_roll_total/total_loops;
	gyro_pitch_calibrate_offset=gyro_pitch_total/total_loops;
	gyro_yaw_calibrate_offset=gyro_yaw_total/total_loops;
    acc_total_vector_calibrate = acc_total_vector_sum/total_loops;



	//acc_roll_calibrate_offset=acc_roll_total/total_loops;

     cal_int=0;
    //after calibrating we set default to 1000 to calibrate motors
    for (cal_int = 0; cal_int < 1000 ; cal_int ++){                           //Wait 5 seconds before continuing.
        PORTD |= B11110000;                                                     //Set digital poort 4, 5, 6 and 7 high.
        delayMicroseconds(1000);                                                //Wait 1000us.
        PORTD &= B00001111;                                                     //Set digital poort 4, 5, 6 and 7 low.
        delayMicroseconds(4000);                                                //Wait 3000us.
    }

    cal_int=0;
    //we then go into 1100 for five seconds
    for (cal_int = 0; cal_int < 1000 ; cal_int ++){                           //Wait 5 seconds before continuing.
        PORTD |= B11110000;                                                     //Set digital poort 4, 5, 6 and 7 high.
        delayMicroseconds(1100);                                                //Wait 1000us.
        PORTD &= B00001111;                                                     //Set digital poort 4, 5, 6 and 7 low.
        delayMicroseconds(3900);                                                //Wait 3000us.
    }
}

float last_micros=0;

int input_value = 0;

void loop(){
//loop_count++;

dt=micros()-last_micros;
last_micros=micros();
dt_p=dt/1000000;

base_thrust = convert_receiver_channel(3);//Convert the actual receiver signals for throttle to the standard 1000 - 2000us.
//max pitch is +-.10
desired_pitch = -((1500.00 - convert_receiver_channel(2)) / 500.00) * 00.20 ;//.10 is max angle 
desired_roll = -((1500.00 - convert_receiver_channel(1)) / 500.00) * 00.20 ;//.10 is max angle 
//desired_roll = convert_receiver_channel(1);
//+-500
//1000 is full up , 2000 is full down (i think) 

get_gyro_data(dt_p);//this takes < 3000us



//We will try to hit 200Hz and improve from there.
 while(micros() - loop_timer < 5000); 
   loop_timer = micros();
   PORTD |= B11110000;                                                 //Set digital outputs 4,5,6 and 7 high.

	//I think we have some timer here
    merge_accel_data();
	calc_pids();


	//fr CCW
	fr_timer =  base_thrust + thrust + front + right +ccw_yaw_thrust + loop_timer;   //Calculate the time of the faling edge of the esc-1 pulse.
	//CW
	br_timer = base_thrust + thrust + back  + right -ccw_yaw_thrust + loop_timer;   //Calculate the time of the faling edge of the esc-2 pulse.
	//CCW
	bl_timer  = base_thrust + thrust + back + left  +ccw_yaw_thrust + loop_timer;   //Calculate the time of the faling edge of the esc-3 pulse.
	//CW
	fl_timer  = base_thrust + thrust + front + left -ccw_yaw_thrust + loop_timer;     //Calculate the time of the faling edge of the esc-4 pulse.

   while(PORTD >= 16){                                                   //Stay in this loop until output 4,5,6 and 7 are low.
     esc_loop_timer = micros();                                           //Read the current time.
     if(fr_timer <= esc_loop_timer)PORTD &= B11101111;                //Set digital output 4 to low if the time is expired.
     if(br_timer <= esc_loop_timer)PORTD &= B11011111;                //Set digital output 5 to low if the time is expired.
     if(bl_timer <= esc_loop_timer)PORTD &= B10111111;                //Set digital output 6 to low if the time is expired.
     if(fl_timer <= esc_loop_timer)PORTD &= B01111111;                //Set digital output 7 to low if the time is expired.
   }
}

//currently assuming 0 as setpoint
void calc_pids(){

    //I think increasing left and right motors will torque counter clockwise
    //positive yaw is counter clockwise
    //so as yaw increases we need to increase left right motors
    pid_i_total_roll_error+=-(desired_roll-gyro_roll);
    pid_i_total_pitch_error+=(desired_pitch-gyro_pitch);
    pid_i_total_yaw_error+=gyro_yaw;

	//left yaw, increases yaw thrust
	//so 
   ccw_yaw_thrust=(
     + (p_gain_yaw * (desired_yaw-gyro_yaw))
     + (d_gain_yaw * gyro_yaw_velocity)
    // + (i_gain * pid_i_total_yaw_error)
    ); 

 front = (p_gain * -(desired_pitch-gyro_pitch)) 
     +  (d_gain * gyro_pitch_velocity)
     +  (i_gain * pid_i_total_pitch_error);

 back = -front;

 right = (p_gain * -(desired_roll - gyro_roll))  //gyro roll is negative as left goes up, its oppostie on ios
     + (d_gain * gyro_roll_velocity)
     + (i_gain * pid_i_total_roll_error);

  left= -right;

  //thrust = -(p_gain_thrust * vel.y);


}

int clamp_thrust(int t){
    if(t < 1100){
        return 1100;    
    }
    if(t > 2000){
        return 2000;    
    }
    return t;
}

void print_angles(){
  String imuLog = ""; // Create a fresh line to log
  imuLog += ",";
  imuLog += String(-gyro_yaw) + ",";
  imuLog += String(gyro_pitch) + ",";
  imuLog += String(gyro_roll) + ",";
  imuLog += String("0") + ",";
  Serial.println(imuLog);

}
                                                                                       

void get_gyro_data(double dt_p){

    Wire.beginTransmission(gyro_address);                                   //Start communication with the gyro.
    Wire.write(0x3B);                                                       //Start reading @ register 43h and auto increment with every read.
    Wire.endTransmission(false);                                             //End the transmission.
    Wire.requestFrom(gyro_address,14);                                      //Request 14 bytes from the gyro.
    
    while(Wire.available() < 14);                                           //Wait until the 14 bytes are received.
	acc_axis[1] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_x variable.
	acc_axis[2] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_y variable.
	acc_axis[3] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_z variable.
	temperature = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the temperature variable.
	gyro_axis[1] = Wire.read()<<8|Wire.read();//x rol                             //Read high and low part of the angular data.
	gyro_axis[2] = Wire.read()<<8|Wire.read();//y                              //Read high and low part of the angular data.
	gyro_axis[3] = Wire.read()<<8|Wire.read();//z  


	gyro_roll_velocity=-gyro_axis[1];
	gyro_pitch_velocity=-gyro_axis[2];
	gyro_yaw_velocity= -gyro_axis[3];

	accel.x=acc_axis[2];
	accel.y=acc_axis[1];
	accel.z=acc_axis[3];
	
    //this is hypotinous
	acc_total_vector = sqrt((accel.x*accel.x)+(accel.y*accel.y)+(accel.z*accel.z));       //Calculate the total accelerometer vector.
    //accel.y average:105;
    //accel.x average:112;
    //accel.z average:4222 
//	acc_total_vector = accel.z;       //Calculate the total accelerometer vector.


    if(calibrating){
        return;
	}    

    gyro_roll_velocity-= gyro_roll_calibrate_offset;
    gyro_pitch_velocity-= gyro_pitch_calibrate_offset;
    gyro_yaw_velocity-= gyro_yaw_calibrate_offset;


    //Serial.println(accel.x);
    //calculate acc eulers
    //4200 roughly g force of gravity

    //booo this is sucks... use tranlation matrix 
    acc_total_diff = acc_total_vector_calibrate-acc_total_vector;
    upward_accel=-(acc_total_diff / acc_total_vector_calibrate);
    
    //we should ge dot product as it may be at tilt ... booo
    // go to 0 to prevent drift
    
    vel.y= (vel.y*.995) + (upward_accel * dt_p);
    

   // pos.y+=(vel.y * dt_p);
     // Serial.println(vel.y);
    //pos.y+=(vel.y * dt_p);


    //picture a weight in the middle of all the axis, each axis is a string with a weight in the middle, the weight pulling on each string is the value we get here
    //so when completely level at rest only z (roll) axis has weight

    //Serial.println(accel.z);// x is acces around roll 
    //Serial.println(accel.y);// y is acces around pitch 
    //Serial.println(accel.z);// z is acces around yaw  

	if(abs(accel.x) < acc_total_vector){                                        //Prevent the asin function to produce a NaN
        //taylor undo the negateive
        accel_euler.x = -asin((float)accel.x/acc_total_vector);

	}
	if(abs(accel.y) < acc_total_vector){                                        //Prevent the asin function to produce a NaN
		accel_euler.y = asin((float)accel.y/acc_total_vector);
	}


    //1.14319066006 = 1 radian per second
	//65.5 = 1degree per second
	//23580 = 360 degrees per second
	//degrees to radions = degress* PI/180 or degrees * 0.01745329251  
	gyro.x=gyro_roll_velocity/GYRO_DPS * DEGREES_TO_RADIANS;//x is roll
	gyro.y=gyro_pitch_velocity/GYRO_DPS * DEGREES_TO_RADIANS;
	gyro.z=gyro_yaw_velocity/GYRO_DPS* DEGREES_TO_RADIANS;

	updateQuatByRotation(o,gyro,dt_p,w,nq);//need to be exact 300us
	//don't need to do this here//toEulerAngle(o,gyro_roll,gyro_pitch,gyro_yaw); //500us

	//now we apply complementary filter by getting diff in accel-gryo eulers and correct gyro by some measure	
	accel_gyro_delta.x=accel_euler.x-gyro_roll;
	accel_gyro_delta.y=accel_euler.y-gyro_pitch;
	accel_gyro_delta.z=gyro.z;//0;//to bad yaw will drift...
	updateQuatByRotation(o,accel_gyro_delta,.005,w,nq);//1% correct with accell,may bet to high 300us
	toEulerAngle(o,gyro_roll,gyro_pitch,gyro_yaw);//500 us 
}


//this needs to stay < 1000k
//we mostly just want to have this seperate for time reasons
void merge_accel_data(){
	toEulerAngle(o,gyro_roll,gyro_pitch,gyro_yaw);//500 us 
}

Quat new_quat(float angle,Vec axis){
	Quat q;
	q.w=cos(angle/2);
	q.x=axis.x*sin(angle/2);
	q.y=axis.y*sin(angle/2);
	q.z=axis.z*sin(angle/2);
	return q;
}

//note that the vector should be angular velocity vector                                      
void updateQuatByRotation(Quat& q, Vec& v, float dt,Quat& w,Quat& nq)
{                                                                                             
        w.w=0;                                                                                
        w.x=v.x * dt;                                                                         
        w.y=v.y * dt;                                                                         
        w.z=v.z * dt ;                                                                        
                    
        quat_mul(q,w,nq);
                                                                                              
        q.w += nq.w * 0.5;                                                    
        q.x += nq.x * 0.5;                                                    
        q.y += nq.y * 0.5;                                                    
        q.z += nq.z * 0.5;                                                    
                                                                                              
     //   quat_norm(&q);//not sure you need this
}


void quat_mul(Quat& q1,Quat& q2,Quat &r){
    r.w = (q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
    r.x = (q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y);
    r.y = (q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x);
    r.z = (q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w);
}

void quat_norm(Quat * q){

 float d= (q->w*q->w) + (q->x*q->x) + (q->y*q->y) + (q->z*q->z);
 if(d==0){
     q->w = 1;
     q->x = 0;
     q->y = 0;
     q->z = 0;
     return ;
 }

 float m  = sqrt(d);
 q->w = q->w / m;
 q->x = q->x / m;
 q->y = q->y / m;
 q->z = q->z / m ;
}

void v_mult_s(Vec& v,float m){
    v.x=v.x*m;
    v.y=v.y*m;
    v.z=v.z*m;
}

void toQuaternion(Quat& q, double roll, double pitch,double yaw)
{
        // Abbreviations for the various angular functions
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);

	q.w = cy * cr * cp + sy * sr * sp;
	q.x = cy * sr * cp - sy * cr * sp;
	q.y = cy * cr * sp + sy * sr * cp;
	q.z = sy * cr * cp - cy * sr * sp;
}


static void toEulerAngle(Quat& q, double& roll, double& pitch, double& yaw)
{
	// roll (x-axis rotation)
	double sinr = +2.0 * (q.w * q.x + q.y * q.z);
	double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
	roll = atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (q.w* q.y - q.z * q.x);
        if (fabs(sinp) >= 1)
            pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
	    pitch = asin(sinp);
	// yaw (z-axis rotation)
	double siny = +2.0 * (q.w * q.z + q.x * q.y);
	double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);  
	yaw = atan2(siny, cosy);
}

static void toEulerAngle2(Quat& q, double& roll, double& pitch, double& yaw)
{


    float ysqr = q.y * q.y;
    float t0 = -2.0f * (ysqr + q.z * q.z) + 1.0f;
    float t1 = +2.0f * (q.x * q.y - q.w * q.z);
    float t2 = -2.0f * (q.x * q.z + q.w * q.y);
    float t3 = +2.0f * (q.y * q.z - q.w * q.x);
    float t4 = -2.0f * (q.x * q.x + ysqr) + 1.0f;

    // Keep t2 within range of asin (-1, 1)
    t2 = t2 > 1.0f ? 1.0f : t2;
    t2 = t2 < -1.0f ? -1.0f : t2;

    pitch = asin(t2) * 2;
    roll = atan2(t3, t4);
    yaw = atan2(t1, t0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This routine is called every time input 8, 9, 10 or 11 changed state. This is used to read the receiver signals. 
//More information about this subroutine can be found in this video:
//https://youtu.be/bENjl1KQbvo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(PCINT0_vect){
  current_time = micros();
  //Channel 1=========================================
  if(PINB & B00000001){                                                     //Is input 8 high?
    if(last_channel_1 == 0){                                                //Input 8 changed from 0 to 1.
      last_channel_1 = 1;                                                   //Remember current input state.
      timer_1 = current_time;                                               //Set timer_1 to current_time.
    }
  }
  else if(last_channel_1 == 1){                                             //Input 8 is not high and changed from 1 to 0.
    last_channel_1 = 0;                                                     //Remember current input state.
    receiver_input[1] = current_time - timer_1;                             //Channel 1 is current_time - timer_1.
  }
  //Channel 2=========================================
  if(PINB & B00000010 ){                                                    //Is input 9 high?
    if(last_channel_2 == 0){                                                //Input 9 changed from 0 to 1.
      last_channel_2 = 1;                                                   //Remember current input state.
      timer_2 = current_time;                                               //Set timer_2 to current_time.
    }
  }
  else if(last_channel_2 == 1){                                             //Input 9 is not high and changed from 1 to 0.
    last_channel_2 = 0;                                                     //Remember current input state.
    receiver_input[2] = current_time - timer_2;                             //Channel 2 is current_time - timer_2.
  }
  //Channel 3=========================================
  if(PINB & B00000100 ){                                                    //Is input 10 high?
    if(last_channel_3 == 0){                                                //Input 10 changed from 0 to 1.
      last_channel_3 = 1;                                                   //Remember current input state.
      timer_3 = current_time;                                               //Set timer_3 to current_time.
    }
  }
  else if(last_channel_3 == 1){                                             //Input 10 is not high and changed from 1 to 0.
    last_channel_3 = 0;                                                     //Remember current input state.
    receiver_input[3] = current_time - timer_3;                             //Channel 3 is current_time - timer_3.

  }
  //Channel 4=========================================
  if(PINB & B00001000 ){                                                    //Is input 11 high?
    if(last_channel_4 == 0){                                                //Input 11 changed from 0 to 1.
      last_channel_4 = 1;                                                   //Remember current input state.
      timer_4 = current_time;                                               //Set timer_4 to current_time.
    }
  }
  else if(last_channel_4 == 1){                                             //Input 11 is not high and changed from 1 to 0.
    last_channel_4 = 0;                                                     //Remember current input state.
    receiver_input[4] = current_time - timer_4;                             //Channel 4 is current_time - timer_4.
  }
}

void set_gyro_registers(){
    Wire.beginTransmission(gyro_address);                                      //Start communication with the address found during search.
    Wire.write(0x6B);                                                          //We want to write to the PWR_MGMT_1 register (6B hex)
    Wire.write(0x00);                                                          //Set the register bits as 00000000 to activate the gyro
    Wire.endTransmission();                                                    //End the transmission with the gyro.

    Wire.beginTransmission(gyro_address);                                      //Start communication with the address found during search.
    Wire.write(0x1B);                                                          //We want to write to the GYRO_CONFIG register (1B hex)
    Wire.write(0x08);                                                          //Set the register bits as 00001000 (500dps full scale)
    Wire.endTransmission();                                                    //End the transmission with the gyro

    Wire.beginTransmission(gyro_address);                                      //Start communication with the address found during search.
    Wire.write(0x1C);                                                          //We want to write to the ACCEL_CONFIG register (1A hex)
    Wire.write(0x10);                                                          //Set the register bits as 00010000 (+/- 8g full scale range)
    Wire.endTransmission();                                                    //End the transmission with the gyro

    //Let's perform a random register check to see if the values are written correct
    Wire.beginTransmission(gyro_address);                                      //Start communication with the address found during search
    Wire.write(0x1B);                                                          //Start reading @ register 0x1B
    Wire.endTransmission();                                                    //End the transmission
    Wire.requestFrom(gyro_address, 1);                                         //Request 1 bytes from the gyro
    while(Wire.available() < 1);                                               //Wait until the 6 bytes are received
    if(Wire.read() != 0x08){                                                   //Check if the value is 0x08
      digitalWrite(12,HIGH);                                                   //Turn on the warning led
      while(1)delay(10);                                                       //Stay in this loop for ever
    }

    Wire.beginTransmission(gyro_address);                                      //Start communication with the address found during search
    Wire.write(0x1A);                                                          //We want to write to the CONFIG register (1A hex)
    Wire.write(0x03);                                                          //Set the register bits as 00000011 (Set Digital Low Pass Filter to ~43Hz)
    Wire.endTransmission();                                                    //End the transmission with the gyro    

}


//The stored data in the EEPROM is used.
float convert_receiver_channel(byte function){
  byte channel, reverse;                                                       //First we declare some local variables
  int low, center, high, actual;
  int difference;
  actual = receiver_input[function];                                            //Read the actual receiver value for the corresponding function
  if(actual < 1000){                                                         //The actual receiver value is lower than the center value
	return 1000;
  } else if(actual > 2000){                                                                        //The actual receiver value is higher than the center value
	return 2000;
  }
  //basically center
  if(actual > 1485 && actual < 1515){
    return 1500;    
  }

   return actual;
}

