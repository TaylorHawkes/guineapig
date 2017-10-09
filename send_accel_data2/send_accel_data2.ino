

#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#include <SD.h>

#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define SERIAL_BAUD_RATE 4800 // Serial port baud
#define INTERRUPT_PIN 4 // MPU-9250 INT pin tied to D4
#define SAMPLE_RATE 100  //10Hz.
#define FIFO_CORRUPTION_CHECK 1

//#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h
MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class

float stablex=-26;//median
float close_enough_to_being_stable=10;//5 is one standard devation 
int fl_power_base=1200;
int br_power_base=1200;
float power_add_ratio=50;

float pitch_ratio=.5;//50% of pitch gives 25 percent or so power
float velocity_ratio=.001;//fast is 1000 degrees per secode, so at 1000 we give 20 power

float gyroAngleX=0;
float gyroAngley=0;

int new_br_power_add=0;
int fl_power_add=0;
int br_power_add=0;


//imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);

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
    imu.setIntLevel(INT_ACTIVE_LOW);
    // The interrupt can be set to latch until data is read, or as a 50us pulse.
    // Options are INT_LATCHED or INT_50US_PULSE
    imu.setIntLatched(INT_LATCHED);

	//more settings
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
	imu.setLPF(5); // Set LPF corner frequency to 5Hz
	// The sample rate of the accel/gyro can be set using
	// setSampleRate. Acceptable values range from 4Hz to 1kHz
	imu.setSampleRate(SAMPLE_RATE); // Set sample rate to 10Hz
	// Likewise, the compass (magnetometer) sample rate can be
	// set using the setCompassSampleRate() function.
	// This value can range between: 1-100Hz
	//imu.setCompassSampleRate(10); // Set mag rate to 10Hz

  // Initialize the digital motion processor
    imu.dmpBegin(DMP_FEATURE_SEND_RAW_ACCEL | // Send accelerometer data
                 DMP_FEATURE_GYRO_CAL       | // Calibrate the gyro data
                 DMP_FEATURE_SEND_CAL_GYRO  | // Send calibrated gyro data
                 DMP_FEATURE_6X_LP_QUAT     , // Calculate quat's with accel/gyro
                 SAMPLE_RATE);                         // Set update rate to 10Hz.


}


 

float qToFloat(long number, unsigned char q)
{
    unsigned long mask;
    for (int i=0; i<q; i++)
    {
        mask |= (1<<i);
    }
    return (number >> q) + ((number & mask) / (float) (2<<(q-1)));
}

float ROLL_ERROR_TOTAL_I;
float PITCH_ERROR_TOTAL_I;
float ROLL_ERROR_TOTAL_OUTER_I=0;
float PITCH_ERROR_TOTAL_OUTER_I=0;
float desired_roll_velocity=0;
float desired_pitch_velocity =0;
int master_loop_count=0;
float pitch_error_last=0;
float roll_error_last=0;



void loop(){
//we are going digital usine fifo

if (!imu.fifoAvailable() ) { return; }
if (imu.dmpUpdateFifo() != INV_SUCCESS ){ return; }

//NOTE THAT GYRO DATA IS ANGULAR VELOCITY  
//ACCELORMATOR IS ACCELERATION

// if ( digitalRead(INTERRUPT_PIN) == LOW ) // If MPU-9250 interrupt fires (active-low)
// imu.update();// Update all sensor's
	String imuLog = ""; // Create a fresh line to log
	//imuLog += ","; // Add time to log string
//	imuLog += String(imu.time) + ","; // Add time to log string
    //THIS IS EULAR LOOP, RUNS 1/10 times and outputs desired_roll/pitch
   // master_loop_count++;
   // if(master_loop_count==1){
      imu.computeEulerAngles2(false);
        //2000,1000 is not bad
        //2500/400 is better
        float P_PITCH=2500;//this is gain
        float P_ROLL=2500;//this gain

        float I_OUTER = 0;
        float II_OUTER= 0;

        //we get steady small ossilation in the system when this is too high
        float D_OUTER = 500;
        float DD_OUTER= 500;


        float desired_roll=M_PI;
        float desired_pitch=0;
        float pitch_error=-(desired_pitch-imu.pitch);
        float roll_error= (desired_roll- abs(imu.roll)) * (abs(imu.roll)/imu.roll);
        //Roll is weirdly 3.14 or -3.14 if we go over it then gets closer to 0 either way if we are over or under so 
        //we multiply by 1,or -1

        ROLL_ERROR_TOTAL_OUTER_I+=(I_OUTER*roll_error)/SAMPLE_RATE;
        PITCH_ERROR_TOTAL_OUTER_I+=  (II_OUTER*pitch_error)/SAMPLE_RATE;


        float d_roll_error = D_OUTER * ((roll_error - roll_error_last) / ((float) 1/SAMPLE_RATE));
        float d_pitch_error = DD_OUTER * ((pitch_error - pitch_error_last) / ((float) 1/SAMPLE_RATE));


        desired_pitch_velocity=(pitch_error*P_PITCH)+PITCH_ERROR_TOTAL_OUTER_I + d_pitch_error;
        desired_roll_velocity=(roll_error*P_ROLL)+ROLL_ERROR_TOTAL_OUTER_I + d_roll_error;

      // imuLog += String(imu.pitch) + ",";
      // imuLog += String(imu.roll) + ",";
    //  master_loop_count=0;
   //}

    //BELOW IS RATE LOOP//
    //we have desired roll velocity as input and we try to get as close to that as possible
    float base_value=0;
    float T=desired_roll_velocity;
    float TT=desired_pitch_velocity;

    float P=.05;
    float I=0;
    float PP=.05;
    float II=0;

    
    //imu.gx(-2000 through 2000)
    //X does seem to be pitch roll access

    float roll_error_velocity= T ;//- imu.gx;
    //Y does seem to be pitch access
    //there is lots of noise?
    float pitch_error_velocity= TT ;//- imu.gy;

    ROLL_ERROR_TOTAL_I+=I*(roll_error_velocity)*(1/SAMPLE_RATE);
    PITCH_ERROR_TOTAL_I+=II*(pitch_error_velocity)*(1/SAMPLE_RATE);

    float right_thrust;
    float left_thrust;
    float back_thrust;
    float front_thrust;

   right_thrust=((roll_error_velocity * P) + ROLL_ERROR_TOTAL_I);
   left_thrust=-(roll_error_velocity * P) + ROLL_ERROR_TOTAL_I ;
   front_thrust=-((pitch_error_velocity * PP) + ROLL_ERROR_TOTAL_I);
   back_thrust= (pitch_error_velocity * PP) + ROLL_ERROR_TOTAL_I ;



   int fr=min_0(base_value+right_thrust + front_thrust);
   int fl=min_0(base_value+left_thrust + front_thrust);
   int br=min_0(base_value+right_thrust + back_thrust);
   int bl=min_0(base_value+left_thrust + back_thrust);

   imuLog += String(fr) + ",";
   imuLog += String(fl) + ",";
   imuLog += String(br) + ",";
   imuLog += String(bl) + ",";



  // imu.computeEulerAngles2();

    //imuLog += String(imu.yaw) + ",";
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

//    LOG_PORT.println(desired_pitch);
	write_to_ardunio(imuLog);

    pitch_error_last=pitch_error;
    roll_error_last=roll_error;
}

void write_to_ardunio(String tstring){

//TAYLOR THIS BREAKS IF STRING IS TOO LONG
  LOG_PORT.println(tstring);
  tstring += "\r\n"; // Add a new line
  int str_len = tstring.length() + 1; 
  //convert string to char array
  char __imuLog[str_len];
    tstring.toCharArray(__imuLog, str_len);
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(__imuLog);          // sends one byte  
    Wire.endTransmission();    // stop transmitting
}


float t_abs(float tnumber){
    if(tnumber >= 0){
        return tnumber;
    }else{
        return tnumber * -1;
    }
}

float min_0(float t){
return t;    
////if(t<1170){
////       return 1170; 
////}
////if(t>1700){
////    return  1700;    
////}
////return t;
}



