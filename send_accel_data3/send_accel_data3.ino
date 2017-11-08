
#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#include <SD.h>

#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define SERIAL_BAUD_RATE 4800 // Serial port baud
#define INTERRUPT_PIN 4 // MPU-9250 INT pin tied to D4
#define SAMPLE_RATE 200  //10Hz.
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

int p_gain=0;
int d_gain=0;

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
    imu.setIntLevel(INT_ACTIVE_HIGH);

    // The interrupt can be set to latch until data is read, or as a 50us pulse.
    // Options are INT_LATCHED or INT_50US_PULSE
    imu.setIntLatched(INT_LATCHED);

	//more settings
	//imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors
	imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL ); // Enable all sensors
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
	//imu.setCompassSampleRate(10); // Set mag rate to 10Hz

  // Initialize the digital motion processor
    imu.dmpBegin(DMP_FEATURE_SEND_RAW_ACCEL | // Send accelerometer data
                 DMP_FEATURE_GYRO_CAL       | // Calibrate the gyro data
                 DMP_FEATURE_SEND_CAL_GYRO  | // Send calibrated gyro data
                 DMP_FEATURE_6X_LP_QUAT     , // Calculate quat's with accel/gyro
                 SAMPLE_RATE);                  // Set update rate to 10Hz.


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
float desired_roll_error=0;
float desired_pitch_error =0;
int master_loop_count=0;
int get_ardunio_info_count=0;
float pitch_error_last=0;
float roll_error_last=0;

float pitch_error_inner_last=0;
float roll_error_inner_last=0;



float last_time=0;
float last_roll=0;


void loop(){
//we are going digital usine fifo

if (!imu.fifoAvailable() ) { return; }
if (imu.dmpUpdateFifo() != INV_SUCCESS ){ return; }

//NOTE THAT GYRO DATA IS ANGULAR VELOCITY  
//ACCELORMATOR IS ACCELERATION
//TAYLOR I BELIEVE THE QUAT/ORIENATION IS CALCULATED ON THE IMU ITSELF(in the firmware) using the GYRO & Accelorbometer
//TAYLOR this setting "DMP_FEATURE_6X_LP_QUAT" imean use both gyro+accell there is also a setting to use just accel .... As far as i know you can't see the code that figures out orientation so you may need to test precision'

// if ( digitalRead(INTERRUPT_PIN) == LOW ) // If MPU-9250 interrupt fires (active-low)
// imu.update();// Update all sensor's
	String imuLog = ""; // Create a fresh line to log


	//imuLog += ","; // Add time to log string

	//imuLog += String(imu.time) + ","; // Add time to log string
	
    //THIS IS EULAR LOOP, RUNS 1/10 times and outputs desired_roll/pitch
  //  master_loop_count++;
   // if(master_loop_count==1){
     imu.computeEulerAngles2(false);
     LOG_PORT.println(imu.time-last_time); last_time=imu.time; return;
   // LOG_PORT.println(imu.roll); 

        //2000,1000 is not bad
        //2500/400 is better
//    float P_PITCH=1500;//(float) p_gain;//this is gain
//    float P_ROLL=1500;//(float) p_gain;//this gain

         float P_PITCH=30;//(float) p_gain;//this is gain
         float P_ROLL=30;//(float) p_gain;//this gain


        float I_OUTER = 0;
        float II_OUTER= 0;

        //we get steady small ossilation in the system when this is too high
        float D_ROLL = 30;
        float D_PITCH= 30;

        float desired_roll=M_PI + .02;//we want to lean slight to the right to offset natural left tendecy
        float desired_pitch=.04;

        float pitch_error=-(desired_pitch-imu.pitch);
        float roll_error= (desired_roll- abs(imu.roll)) * (abs(imu.roll)/imu.roll);

        //Roll is weirdly 3.14 or -3.14 if we go over it then gets closer to 0 either way if we are over or under so 
        //we multiply by 1,or -1
        //negative roll error means left is down
        ROLL_ERROR_TOTAL_OUTER_I+=(I_OUTER*roll_error)/SAMPLE_RATE;
        PITCH_ERROR_TOTAL_OUTER_I+= (II_OUTER*pitch_error)/SAMPLE_RATE;

        //the bigger the delta the bigger the d_roll_error
        //it should act in opposite directoin
        float d_roll_error = D_ROLL * ((roll_error - roll_error_last) / ((float) 1/SAMPLE_RATE));
        float d_pitch_error = D_PITCH * ((pitch_error - pitch_error_last) / ((float) 1/SAMPLE_RATE));

		d_roll_error=clamp_d(d_roll_error);
		d_pitch_error=clamp_d(d_pitch_error);


        desired_pitch_error=(pitch_error*P_PITCH) + d_pitch_error;
        desired_roll_error=(roll_error*P_ROLL) + d_roll_error;

        //LOG_PORT.print("roll thrust"); LOG_PORT.println(desired_roll_error);

     int ff=min_0(0-desired_pitch_error);
     int rr=min_0(0+desired_roll_error);
     int bb=min_0(0+desired_pitch_error);
     int ll=min_0(0-desired_roll_error) ;

    imuLog += String(ff) + ",";
    imuLog += String(rr) + ",";
    imuLog += String(bb) + ",";
    imuLog += String(ll) + ",";


      // imuLog += String(imu.pitch) + ",";
      // imuLog += String(imu.roll) + ",";
     master_loop_count=0;

    pitch_error_last=pitch_error;
    roll_error_last=roll_error;
  // }


////if(imu.time-last_time > 12){
////	 LOG_PORT.print("MISSSs0-------------------->"); LOG_PORT.println(imu.time-last_time);
////}	

	last_time=imu.time;

	write_to_ardunio(imuLog);
    return;

    //BELOW IS RATE LOOP//
    //we have desired roll velocity as input and we try to get as close to that as possible
 //  float P=.08;
 //  float PP=.08;
  float P=0;
  float PP=0;

 /// float D_ROLL_INNER = .003;
 /// float D_PITCH_INNTER= .003;
 float D_ROLL_INNER = 0;//D seems to just mess up
 float D_PITCH_INNTER= 0;



     //float I=0;
     //float II=0;

     //imu.gx(-2000 through 2000) //angular velocity
     //X does seem to be pitch roll access

     //LOG_PORT.print("desired roll velocity"); LOG_PORT.println(desired_roll_error);

     float roll_error_velocity=imu.gx-desired_roll_error;

     //Y does seem to be pitch access
     //there is lots of noise?
     float pitch_error_velocity=imu.gy-desired_pitch_error;

    // ROLL_ERROR_TOTAL_I+=I*(roll_error_velocity)*(1/SAMPLE_RATE);
    // PITCH_ERROR_TOTAL_I+=II*(pitch_error_velocity)*(1/SAMPLE_RATE);

     float d_roll_error_inner = D_ROLL_INNER * ((roll_error_velocity - roll_error_inner_last) / ((float) 1/SAMPLE_RATE));
     float d_pitch_error_inner = D_PITCH_INNTER * ((pitch_error_velocity - pitch_error_inner_last) / ((float) 1/SAMPLE_RATE));

     pitch_error_inner_last=pitch_error_velocity;
     roll_error_inner_last=roll_error_velocity;

     int f=min_0(pitch_error_velocity * PP + d_pitch_error_inner);
     int r=-min_0(roll_error_velocity * P + d_roll_error_inner) ;
     int b=-min_0(pitch_error_velocity * PP + d_pitch_error_inner);
     int l=min_0(roll_error_velocity * P + d_roll_error_inner) ;

    imuLog += String(f) + ",";
    imuLog += String(r) + ",";
    imuLog += String(b) + ",";
    imuLog += String(l) + ",";

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
//i'm pretty sure this can hang and cause issues lets only run in evvery two second'
//get_ardunio_info_count++;
//100 itmes per second
if(get_ardunio_info_count==3){
    Wire.requestFrom(4, 12);
    int bytes = Wire.available();
    char message[12];
    for(int i = 0; i< bytes; i++){
      message[i]=Wire.read();
    }
    get_ardunio_info_count=0;

 if(bytes){
   char p_gain_char[5]; 
   p_gain_char[0]=message[0];
   p_gain_char[1]=message[1];
   p_gain_char[2]=message[2];
   p_gain_char[3]=message[3];
   p_gain_char[4]='\0';
   p_gain=atoi(p_gain_char);

   char d_gain_char[5]; 
   d_gain_char[0]=message[5];
   d_gain_char[1]=message[6];
   d_gain_char[2]=message[7];
   d_gain_char[3]=message[8];
   d_gain_char[4]='\0';
   d_gain=atoi(d_gain_char);
   //LOG_PORT.print("D GAIN IS AT");
   //LOG_PORT.println(d_gain);
 }

}
last_time=imu.time;
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

float clamp_d(float t){
return t;
	if(abs(t) < 5){
		return 0;
	}

	if(t < -50){
		return -50;
	}
	if(t > 50 ){
		return 50;
	}
	return t;
}

float min_0(float t){
return t;    
}



