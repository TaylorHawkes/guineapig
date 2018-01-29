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

int front, right, left, back;
float yaw_thrust;
unsigned long front_timer,right_timer,left_timer,back_timer;
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

float acc_roll_calibrate_offset=0;
float acc_pitch_calibrate_offset=0;
double dt;//change in us
double dt_p;//change in percantge 

Vec gyro;
Vec accel;
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

float p_gain=400;//mult by radians
float d_gain=.01;//mult by raw velocity  
float i_gain=0;//.1;//mult by radians

int base_thrust=1150;
float desired_roll=0;
float desired_pitch=0;
float desired_yaw=0;

//SoftwareSerial mySerial(8, 9); // RX, TX 

void setup() {
   //todo, do this better
   pinMode(4,OUTPUT);
   pinMode(5,OUTPUT);
   pinMode(6,OUTPUT);
   pinMode(7,OUTPUT);

	Serial.begin(SERIAL_BAUD_RATE);
    //mySerial.begin(SERIAL_BAUD_RATE);
    //startTCPServer();

	Wire.begin();
	Wire.setClock(1000000);//this appears to be as fast as we can go

//	Wire.setClock(3400000);
  	//TWBR = 12;                                                                //Set the I2C clock speed to 400kHz.


//	imu_begin();
if (imu.begin() != INV_SUCCESS){
   Serial.println("could not start");
	return;
}

//Serial.println("sentting FSR stuff");
imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors
imu.setGyroFSR(500); // Set gyro to 500 dps
imu.setAccelFSR(8); // Set accel to +/-2g
imu.setLPF(42); // Set LPF corner frequency to 5Hz // id don't think we can do this with sample rate at 1000
imu.setSampleRate(1000); // Set LPF corner frequency to 5Hz


  get_gyro_data(0);

//set first run
    int cal_int=0;
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

	/*CALIBRATION*/	
	//take 5 seconds to calibrate
int total_loops=(40 * 250);
float gyro_roll_total=0;
float gyro_pitch_total=0;
float gyro_yaw_total=0;
float acc_roll_total=0;
float acc_pitch_total=0;

//   calibrating=true;


//   //Serial.println("starting");
//   for (int i=0; i<total_loops; i++){
//       delay(4);

//   //Serial.println("getting gyro data");
//       get_gyro_data(0);

//       gyro_roll_total+=gyro_roll_velocity;
//       gyro_pitch_total+=gyro_pitch_velocity;
//       gyro_yaw_total+=gyro_yaw_velocity;
//       acc_roll_total+=accel.x;
//       acc_pitch_total+=accel.y;
//   }

//   calibrating=false;

//   Serial.println("here");


//gyro_roll_calibrate_offset=gyro_roll_total/total_loops;
//gyro_pitch_calibrate_offset=gyro_pitch_total/total_loops;
//gyro_yaw_calibrate_offset=gyro_yaw_total/total_loops;
//acc_roll_calibrate_offset=acc_roll_total/total_loops;

//    acc_pitch_calibrate_offset=acc_pitch_total/total_loops;
//  Serial.print("gyro roll offset:"); Serial.println(gyro_roll_calibrate_offset);
//  Serial.print("gyro pitch offset:"); Serial.println(gyro_pitch_calibrate_offset);
//  Serial.print("gyro yaw offset:"); Serial.println(gyro_yaw_calibrate_offset);
//  Serial.print("acc roll offset:"); Serial.println(acc_roll_calibrate_offset);
//  Serial.print("acc pitch offset:"); Serial.println(acc_pitch_calibrate_offset);

  gyro_roll_calibrate_offset=-152.45;
  gyro_pitch_calibrate_offset=43.79;
  gyro_yaw_calibrate_offset=26;
  acc_roll_calibrate_offset=91.95;
  acc_pitch_calibrate_offset=-106.67;


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
loop_count++;

//100 = half second
   if(loop_count==10){// at 200hz this is 20hz refersh rate  every 50ms 
      // Serial.println("getting command");
       get_commands();
       loop_count=0;
   }
//Serial.println(gyro_roll);

dt=micros()-last_micros;
last_micros=micros();
dt_p=dt/1000000;
get_gyro_data(dt_p);//this takes < 3000us

//Serial.println(gyro_yaw);


//We will try to hit 200Hz and improve from there.
 while(micros() - loop_timer < 5000); 
   loop_timer = micros();
   PORTD |= B11110000;                                                 //Set digital outputs 4,5,6 and 7 high.

	//I think we have some timer here
    merge_accel_data();
	calc_pids();
	//print_angles();

   front_timer = front + loop_timer;                                   //Calculate the time of the faling edge of the esc-1 pulse.
   right_timer = right + loop_timer;                                   //Calculate the time of the faling edge of the esc-2 pulse.
   back_timer  = back + loop_timer;                                     //Calculate the time of the faling edge of the esc-3 pulse.
   left_timer  = left + loop_timer;                                     //Calculate the time of the faling edge of the esc-4 pulse.
	
   while(PORTD >= 16){                                                   //Stay in this loop until output 4,5,6 and 7 are low.
     esc_loop_timer = micros();                                           //Read the current time.
     if(front_timer <= esc_loop_timer)PORTD &= B11101111;                //Set digital output 4 to low if the time is expired.
     if(right_timer <= esc_loop_timer)PORTD &= B11011111;                //Set digital output 5 to low if the time is expired.
     if(back_timer <= esc_loop_timer)PORTD &= B10111111;                //Set digital output 6 to low if the time is expired.
     if(left_timer <= esc_loop_timer)PORTD &= B01111111;                //Set digital output 7 to low if the time is expired.
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

    //positive yaw thrust goes to left,right motors   
   yaw_thrust=(
     + (p_gain * (desired_yaw-gyro_yaw))
     + (d_gain * gyro_yaw_velocity)
    // + (i_gain * pid_i_total_yaw_error)
    ); 
//  front=clamp_thrust(base_thrust -yaw_thrust);
//  left=clamp_thrust(base_thrust + yaw_thrust);
//  back=clamp_thrust(base_thrust -yaw_thrust);
//  right=clamp_thrust(base_thrust+yaw_thrust);

 front=clamp_thrust(
 base_thrust 
     + (p_gain * -(desired_pitch-gyro_pitch)) 
     + (d_gain * gyro_pitch_velocity)
     + (i_gain * pid_i_total_pitch_error)
     - yaw_thrust
 );

 right=clamp_thrust(
 base_thrust 
     + (p_gain * (desired_roll-gyro_roll)) 
     + (d_gain * -gyro_roll_velocity)
     + (i_gain * -pid_i_total_roll_error)
   //  + yaw_thrust
 );

 back=clamp_thrust(
 base_thrust 
     + (p_gain * (desired_pitch-gyro_pitch)) 
     + (d_gain * -gyro_pitch_velocity)
     + (i_gain * -pid_i_total_pitch_error)
   //   - yaw_thrust
 );

 left=clamp_thrust(
     base_thrust  
     + (p_gain * -(desired_roll - gyro_roll))  //gyro roll is negative as left goes up, its oppostie on ios
     + (d_gain * gyro_roll_velocity)
     + (i_gain * pid_i_total_roll_error)
    // + yaw_thrust
 );
//   Serial.print("left:"); Serial.println(left);
//   Serial.print("right:"); Serial.println(right);
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
  imuLog += String(-gyro_roll) + ",";
  imuLog += String("0") + ",";
  Serial.println(imuLog);

}

int arduino_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char * data)                            
{                                                                                             
    Wire.beginTransmission(slave_addr);                                                       
    Wire.write(reg_addr);                                                                     
    for (unsigned char i = 0; i < length; i++)                                                
    {                                                                                         
        Wire.write(data[i]);                                                                  
    }                                                                                         
    Wire.endTransmission(true);                                                               
                                                                                              
    return 0;                                                                                 
}                                                                                             
                                                                                              
int arduino_i2c_read(unsigned char slave_addr, unsigned char reg_addr,unsigned char length, unsigned char * data)                            
{                                                                                             
    Wire.beginTransmission(slave_addr);                                                       
    Wire.write(reg_addr);                                                                     
    Wire.endTransmission(false);                                                              
    Wire.requestFrom(slave_addr, length);                                                     
    for (unsigned char i = 0; i < length; i++)                                                
    {                                                                                         
        data[i] = Wire.read();                                                                
    }                                                                                         
                                                                                              
    return 0;                                                                                 
}                                                                                             

void imu_begin(){

    unsigned char data[6];
    /* Reset device. */
    data[0] = 0x80; arduino_i2c_write(gyro_address,0x6B, 1, data); delay(100);   

    /* Wake up chip. */
    data[0] = 0x00; arduino_i2c_write(gyro_address,0x6B, 1, data); delay(100);   

	//ehh dont thine we need this	
/// data[0] = 0x48; arduino_i2c_write(gyro_address,0x1D, 1, data); 
/// data[0] = 0x18; arduino_i2c_write(gyro_address,0x1B, 1, data); 
/// data[0] = 0x00; arduino_i2c_write(gyro_address,0x1C, 1, data); 
/// data[0] = 0x03; arduino_i2c_write(gyro_address,0x1A, 1, data); 
/// data[0] = 0x13; arduino_i2c_write(gyro_address,0x19, 1, data); 
/// data[0] = 0x04; arduino_i2c_write(gyro_address,0x1A, 1, data); 
/// data[0] = 0x00; arduino_i2c_write(gyro_address,0x38, 1, data); 
/// data[0] = 0xDF; arduino_i2c_write(gyro_address,0x68, 1, data); 
/// data[0] = 0x82; arduino_i2c_write(gyro_address,0x37, 1, data); 
/// data[0] = 0x04; arduino_i2c_write(gyro_address,0x34, 1, data); 
/// data[0] = 0x40; arduino_i2c_write(gyro_address,0x6B, 1, data); 
/// data[0] = 0x3F; arduino_i2c_write(gyro_address,0x6C, 1, data); 
/// data[0] = 0x10; arduino_i2c_write(gyro_address,0x64, 1, data); 
/// data[0] = 0x5F; arduino_i2c_write(gyro_address,0x6A, 1, data); 
/// data[0] = 0x01; arduino_i2c_write(gyro_address,0x6B, 1, data); 
/// data[0] = 0x00; arduino_i2c_write(gyro_address,0x6C, 1, data); 
/// data[0] = 0x11; arduino_i2c_write(gyro_address,0x64, 1, data); 
/// data[0] = 0x7F; arduino_i2c_write(gyro_address,0x6A, 1, data); 

	arduino_i2c_write(gyro_address,0x1B, 1,0x08); //GYRP FSR
	arduino_i2c_write(gyro_address,0x1C, 1,0x10); // 0x1C,10 accell FSR 8
	arduino_i2c_write(gyro_address,0x1A, 1,0x03); // 0x1A,3 lpf to 42
	//sample rate to 1000 = 0x19,0 - 0x34,63 - 0x1A,1
	arduino_i2c_write(gyro_address,0x19, 1,0x00); 
	arduino_i2c_write(gyro_address,0x34, 1,0x63); 
	arduino_i2c_write(gyro_address,0x1A, 1,0x01); 
}

void get_gyro_data(double dt_p){

    Wire.beginTransmission(gyro_address);                                   //Start communication with the gyro.
    Wire.write(0x3B);                                                       //Start reading @ register 43h and auto increment with every read.
    //Wire.endTransmission();                                                 //End the transmission.
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



	gyro_roll_velocity=gyro_axis[1];
	gyro_pitch_velocity=gyro_axis[2];
	gyro_yaw_velocity= gyro_axis[3];

	accel.x=acc_axis[2];
	accel.y=acc_axis[1];
	accel.z=acc_axis[3];
	
    if(calibrating){
        return;
	}    

    gyro_roll_velocity-= gyro_roll_calibrate_offset;
    gyro_pitch_velocity-= gyro_pitch_calibrate_offset;
    gyro_yaw_velocity-= gyro_yaw_calibrate_offset;

    accel.x-=acc_roll_calibrate_offset;
    accel.y-=acc_pitch_calibrate_offset;
    //accel.z-=accel

    //calculate acc eulers
	acc_total_vector = sqrt((accel.x*accel.x)+(accel.y*accel.y)+(accel.z*accel.z));       //Calculate the total accelerometer vector.
	if(abs(accel.x) < acc_total_vector){                                        //Prevent the asin function to produce a NaN
		accel_euler.x = -asin((float)accel.x/acc_total_vector);
	}
	if(abs(accel.y) < acc_total_vector){                                        //Prevent the asin function to produce a NaN
		accel_euler.y = asin((float)accel.y/acc_total_vector);
	}

    //1.14319066006 = 1 radian per second
	//65.5 = 1degree per second
	//23580 = 360 degrees per second
	//degrees to radions = degress* PI/180 or degrees * 0.01745329251  
	gyro.x=gyro_roll_velocity/GYRO_DPS * DEGREES_TO_RADIANS;
	gyro.y=gyro_pitch_velocity/GYRO_DPS * DEGREES_TO_RADIANS;
	gyro.z=gyro_yaw_velocity/GYRO_DPS* DEGREES_TO_RADIANS;

	updateQuatByRotation(o,gyro,dt_p,w,nq);//need to be exact 300us
	toEulerAngle(o,gyro_roll,gyro_pitch,gyro_yaw); //500us

	//now we apply complementary filter by getting diff in accel-gryo eulers and correct gyro by some measure	
	accel_gyro_delta.x=accel_euler.x-gyro_roll;
	accel_gyro_delta.y=accel_euler.y-gyro_pitch;
	accel_gyro_delta.z=0;//to bad yaw will drift...
	updateQuatByRotation(o,accel_gyro_delta,.005,w,nq);//1% correct with accell,may bet to high 300us
	//toEulerAngle(o,gyro_roll,gyro_pitch,gyro_yaw);//500 us 
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

//using the SIM5320A - (with antena) and hologram
void startTCPServer(){
delay(20000);
Serial.write("AT+CIPMODE=1\r"); res();
Serial.write("AT+NETOPEN\r"); res();
Serial.write("AT+CIPOPEN=0,\"UDP\",\"23.92.20.100\",9999,9000\r"); res();
}

void res(){
  delay(5000);
  while (Serial.available() > 0){
        Serial.read();
     //Serial.write();
  };
}

void get_commands(){
     // mySerial.write("ok"); 
	 int bytes = Serial.available();

    if(!bytes){
        return;
    }
	  int i;
	  for(i = 0; i< bytes; i++){
		udp_request[i]= Serial.read();
	  }

		//this is hack
       //if(bytes){
        //remove this if getting from elseware
///      udp_request[i+1]='\0';
///     int new_hover_rate_int=atoi(udp_request);
///     if(new_hover_rate_int != base_thrust){
///          base_thrust=new_hover_rate_int;
///     }
///     return;
        //end remove this if getting from elseware

          udp_request[i+1]='\0';
         char * udp_line;
      udp_line=strtok(udp_request,"<");
      //OK 1000|1000,-0.00|-0.00,+0.01|+0.01,+1.22|+1.22
      while (udp_line != NULL)
      {
		if(strlen(udp_line)==correct_udp_line_length){

        char base_thrust_char[5];
        base_thrust_char[0]=udp_line[0];
        base_thrust_char[1]=udp_line[1];
        base_thrust_char[2]=udp_line[2];
        base_thrust_char[3]=udp_line[3];
        base_thrust_char[4]='\0';
        base_thrust=atoi(base_thrust_char);

        char desired_roll_char[6];
        desired_roll_char[0]=udp_line[4];
        desired_roll_char[1]=udp_line[5];
        desired_roll_char[2]=udp_line[6];
        desired_roll_char[3]=udp_line[7];
        desired_roll_char[4]=udp_line[8];
        desired_roll_char[5]='\0';
        desired_roll=atof(desired_roll_char);
        desired_roll=0;

        char desired_pitch_char[6];
        desired_pitch_char[0]=udp_line[9];
        desired_pitch_char[1]=udp_line[10];
        desired_pitch_char[2]=udp_line[11];
        desired_pitch_char[3]=udp_line[12];
        desired_pitch_char[4]=udp_line[13];
        desired_pitch_char[5]='\0';
        desired_pitch=atof(desired_pitch_char);
        desired_pitch=0;

        char desired_yaw_char[6];
        desired_yaw_char[0]=udp_line[14];
        desired_yaw_char[1]=udp_line[15];
        desired_yaw_char[2]=udp_line[16];
        desired_yaw_char[3]=udp_line[17];
        desired_yaw_char[4]=udp_line[18];
        desired_yaw_char[5]='\0';
        desired_yaw=atof(desired_yaw_char);
        desired_yaw=0;


			//the gets Throttle
////		if(
////            //verify thrust 
////            udp_line[3]==udp_line[8] &&
////            udp_line[4]==udp_line[9] &&
////            udp_line[5]==udp_line[10] &&
////            udp_line[6]==udp_line[11] &&
////            //verify roll 
////             udp_line[13]==udp_line[19] &&
////             udp_line[14]==udp_line[20] &&
////             udp_line[15]==udp_line[21] &&
////             udp_line[16]==udp_line[22] &&
////             udp_line[17]==udp_line[23] &&
////            //verify pitch 
////             udp_line[25]==udp_line[31] &&
////             udp_line[26]==udp_line[32] &&
////             udp_line[27]==udp_line[33] &&
////             udp_line[28]==udp_line[34] &&
////             udp_line[29]==udp_line[35] &&
////            //verify roll 
////             udp_line[37]==udp_line[43] &&
////             udp_line[38]==udp_line[44] &&
////             udp_line[39]==udp_line[45] &&
////             udp_line[40]==udp_line[46] &&
////             udp_line[41]==udp_line[47] 
////		){


////            
////            char base_thrust_char[5];
////			base_thrust_char[0]=udp_line[3];
////			base_thrust_char[1]=udp_line[4];
////			base_thrust_char[2]=udp_line[5];
////			base_thrust_char[3]=udp_line[6];
////			base_thrust_char[4]='\0';
////            base_thrust=atoi(base_thrust_char);


////            char desired_roll_char[6];
////            desired_roll_char[0]=udp_line[13];
////			desired_roll_char[1]=udp_line[14];
////			desired_roll_char[2]=udp_line[15];
////			desired_roll_char[3]=udp_line[16];
////			desired_roll_char[4]=udp_line[17];
////			desired_roll_char[5]='\0';
////            desired_roll=atof(desired_roll_char);
////            Serial.println(desired_roll);

////            char desired_pitch_char[6];
////            desired_pitch_char[0]=udp_line[25];
////			desired_pitch_char[1]=udp_line[26];
////			desired_pitch_char[2]=udp_line[27];
////			desired_pitch_char[3]=udp_line[28];
////			desired_pitch_char[4]=udp_line[29];
////			desired_pitch_char[5]='\0';
////            desired_pitch=atof(desired_pitch_char);

////            char desired_yaw_char[6];
////            desired_yaw_char[0]=udp_line[37];
////			desired_yaw_char[1]=udp_line[38];
////			desired_yaw_char[2]=udp_line[39];
////			desired_yaw_char[3]=udp_line[40];
////			desired_yaw_char[4]=udp_line[41];
////			desired_yaw_char[5]='\0';
////            desired_yaw=atof(desired_yaw_char);

////		}
	 }

        udp_line = strtok (NULL, "<");
	  }
}
	

bool is_number_char(char a){
return ( a=='0'|| a=='1'|| a=='2'|| a=='3'|| a=='4'|| a=='5'|| a=='6'|| a=='7'|| a=='8'|| a=='9');
}


 

