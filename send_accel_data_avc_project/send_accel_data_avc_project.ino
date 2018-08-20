#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#include <SD.h>

//this logs to mico html
//#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define LOG_PORT SERIAL_PORT_HARDWARE
#define SERIAL_BAUD_RATE 57600 // Serial port baud
#define INTERRUPT_PIN 4 // MPU-9250 INT pin tied to D4
#define SAMPLE_RATE 200  //10Hz.
#define FIFO_CORRUPTION_CHECK 1

//#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h
MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class
float DEGREES_TO_RADIANS=0.01745329251; //multiplier PI/180
float GYRO_DPS =16.4;//https://cdn.sparkfun.com/assets/learn_tutorials/5/5/0/MPU9250REV1.0.pdf


typedef struct {
float x,y,z;                                                                              
} Vec;

typedef struct {                                                                              
float x=0;
float y=0;
float z=0;
float w=1;
} Quat;


double roll,pitch,yaw;
double dt;
double dt_p;

Vec accel;
Vec accel_euler;
Vec gyro;
Vec accel_gyro_delta;
Vec heading_delta;
Quat o;
Quat w;                                                                    
Quat nq;

String imuLog;


void setup()
{
Wire.begin(); // join i2c bus (address optional for master)
LOG_PORT.begin(SERIAL_BAUD_RATE);
if (imu.begin() != INV_SUCCESS){
   LOG_PORT.println("could not start");
    return;
}
pinMode(INTERRUPT_PIN, INPUT_PULLUP); // Set interrupt as an input w/ pull-up resistor
// Use enableInterrupt() to configure the MPU-9250's 
// interrupt output as a "data ready" indicator.
    imu.enableInterrupt();
    // The interrupt level can either be active-high or low. Configure as active-low.
    // Options are INT_ACTIVE_LOW or INT_ACTIVE_HIGH
    imu.setIntLevel(INT_ACTIVE_HIGH);

    // The interrupt can be set to latch until data is read, or as a 50us pulse.
    // Options are INT_LATCHED or INT_50US_PULSE
    imu.setIntLatched(INT_LATCHED);

	//more settings
	//imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors
	imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors
	//imu.setSensors(INV_XYZ_GYRO);// 
	// Gyro options are +/- 250, 500, 1000, or 2000 dps
	imu.setGyroFSR(2000); // Set gyro to 2000 dps
	// Accel options are +/- 2, 4, 8, or 16 g
	imu.setAccelFSR(2); // Set accel to +/-2g
	// setLPF() can be used to set the digital low-pass filter
	// of the accelerometer and gyroscope.
	// Can be any of the following: 188, 98, 42, 20, 10, 5
	// (values are in Hz).
	imu.setLPF(188); // Set LPF corner frequency to 5Hz
	// The sample rate of the accel/gyro can be set using
	// setSampleRate. Acceptable values range from 4Hz to 1kHz
	imu.setSampleRate(SAMPLE_RATE); // Set sample rate to 10Hz
	// Likewise, the compass (magnetometer) sample rate can be
	// set using the setCompassSampleRate() function.
	// This value can range between: 1-100Hz
	imu.setCompassSampleRate(100); // Set mag rate to 10Hz

  // Initialize the digital motion processor
/// imu.dmpBegin(DMP_FEATURE_SEND_RAW_ACCEL | // Send accelerometer data
///              DMP_FEATURE_GYRO_CAL       | // Calibrate the gyro data
///              DMP_FEATURE_SEND_CAL_GYRO  | // Send calibrated gyro data
///              DMP_FEATURE_6X_LP_QUAT     , // Calculate quat's with accel/gyro
///              SAMPLE_RATE);                  // Set update rate to 10Hz.


}
 

long acc_total_vector;
float last_micros=0;

void loop(){
//we are going digital usine fifo

//  if (!imu.fifoAvailable() ) { return; }
//  if (imu.dmpUpdateFifo() != INV_SUCCESS ){ return; }
//  if (imu.updateCompass() != INV_SUCCESS) {return;} //update compas 

if ( imu.dataReady() ) // If new IMU data is available
{

    imu.update(); 
    dt=micros()-last_micros;
    last_micros=micros();
    dt_p=dt/1000000;

    
	accel.x=imu.ax;
	accel.y=imu.ay;
	accel.z=imu.az;

	acc_total_vector = sqrt((accel.x*accel.x)+(accel.y*accel.y)+(accel.z*accel.z));       //Calculate the total accelerometer vector.
	
    //this is =gyro.y
	if(abs(accel.x) < acc_total_vector){
        accel_euler.y = asin((float)accel.x/acc_total_vector);
	}

    //this is =gyro.x
	if(abs(accel.y) < acc_total_vector){
		accel_euler.x = -asin((float)accel.y/acc_total_vector);
	}

	gyro.x=imu.gx/GYRO_DPS * DEGREES_TO_RADIANS;//x is roll
	gyro.y=imu.gy/GYRO_DPS * DEGREES_TO_RADIANS;
	gyro.z=imu.gz/GYRO_DPS* DEGREES_TO_RADIANS;


	updateQuatByRotation(o,gyro,dt_p,w,nq);//need to be exact 300us

	toEulerAngle(o,roll,pitch,yaw);//500 us 

    //complimentary accell
    accel_gyro_delta.x=accel_euler.x-roll;
    accel_gyro_delta.y=accel_euler.y-pitch;
    accel_gyro_delta.z=0;//gyro.z;//0;//to bad yaw will drift...
    updateQuatByRotation(o,accel_gyro_delta,.005,w,nq);//1% correct with accell,may bet to high 300us
	toEulerAngle(o,roll,pitch,yaw);//500 us 

    

    imuLog="";
 

    //imu.computeEulerAngles2(false);
//    float header=imu.computeCompassHeading();

///  imuLog += String(imu.mx) + ",";
///  imuLog += String(imu.my) + ",";
///  imuLog += String(imu.mz) + ",";

//int xmax=890;
//int xmin=172;
//int ymax=466;
//int ymin=-250;
//int zmax=-26;
//int zmin=-750;

    int xmax=1042;
    int xmin=322;
    int ymax=614;
    int ymin=-110;
    int zmax=-79;
    int zmin=-915;



  int offset_x = (xmax + xmin) / 2;
  int offset_y = (ymax + ymin) / 2;
  int offset_z = (zmax + zmin) / 2;

  int hard_fix_x=imu.mx-offset_x;
  int hard_fix_y=imu.my-offset_y;
  int hard_fix_z=imu.mz-offset_z;

   //let scale too
   int avg_delta_x = (xmax - xmin) / 2;
   int avg_delta_y = (ymax - ymin) / 2;
   int avg_delta_z = (zmax - zmin) / 2;

   float avg_delta = (avg_delta_x + avg_delta_y + avg_delta_z) / 3;

   float scale_x = avg_delta / avg_delta_x;
   float scale_y = avg_delta / avg_delta_y;
   float scale_z = avg_delta / avg_delta_z;


   float mx=hard_fix_x*scale_x; 
   float my=hard_fix_y*scale_y; 
   float mz=hard_fix_z*scale_z; 

/// imuLog += String(mx) + ",";
/// imuLog += String(my) + ",";
/// imuLog += String(mz) + ",";

   float heading;
   //compute compas heading
	if (my == 0){
		heading = (mx < 0) ? 3.14 : 0;
	}else{
		heading = atan2(mx, my);
    }

    heading = heading*-1;

	float a = heading - yaw;
	if(a>3.14){
		a-=6.28;
	}else{
		if(a<-3.14){
			a+=6.28;
		}
	}

	//a += (a>180) ? -360 : (a<-180) ? 360 : 0
		
    //float diff_angle = 3.14 - abs(abs(heading - yaw ) - 3.14); 
//  imuLog += String(heading);
    
    //3.11
    //-3.14 =
    
   // yaw = -heading;
   //update the quat by headking 
heading_delta.x=0;
heading_delta.y=0;
heading_delta.z=a;
updateQuatByRotation(o,heading_delta,.005,w,nq);//1% correct with accell,may bet to high 300us

toEulerAngle(o,roll,pitch,yaw);//500 us 

LOG_PORT.println(yaw);
//imuLog += String(yaw)+",";
// imuLog += String(yaw) + ",";
// imuLog += String(pitch) + ",";
// imuLog += String(roll) + ",";



////if (heading > PI){ 
////    heading -= (2 * PI);
////} else if (heading < -PI){ 
////    heading += (2 * PI);
////} else if (heading < 0) {
////    heading += 2 * PI;
////}
////
////heading*= 180.0 / PI;
////
////imuLog += String(heading) + ",";




//  imuLog += String(header) + ",";
  //imuLog += String(imu.pitch) + ",";
  //imuLog += String(imu.roll) + ",";
  //imuLog += String(imu.normal) + ",";

    //imuLog += String(imu.ax) + ",";
    //imuLog += String(imu.ay) + ",";
    //imuLog += String(imu.az) + ",";

    //imuLog += String(imu.gx) + ",";
    //imuLog += String(imu.gy) + ",";
    //imuLog += String(imu.gz) + ",";

    //imuLog += String(imu.dqxr) + ",";
    //imuLog += String(imu.dqyr) + ",";
    //imuLog += String(imu.dqzr) + ",";
    //imuLog += String(imu.dqwr) + ",";

    ////normalized
    //imuLog += String(imu.dqx) + ",";
    //imuLog += String(imu.dqy) + ",";
    //imuLog += String(imu.dqz) + ",";
    //imuLog += String(imu.dqw) + ",";


	//  imuLog += String(imu.roll, 2) + ", ";
	//  imuLog += String(imu.yaw, 2) + ", ";

	//write_to_ardunio(imuLog);

}
}

void write_to_ardunio(String tstring){
//TAYLOR THIS BREAKS IF STRING IS TOO LONG
  LOG_PORT.println(tstring);
//tstring += "\r\n"; // Add a new line
//int str_len = tstring.length() + 1; 
////convert string to char array
//char __imuLog[str_len];
//  tstring.toCharArray(__imuLog, str_len);
//  Wire.beginTransmission(4); // transmit to device #4
//  Wire.write(__imuLog);          // sends one byte  
//  Wire.endTransmission();    // stop transmitting
}

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





