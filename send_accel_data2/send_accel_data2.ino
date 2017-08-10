

#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#include <SD.h>

#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define SERIAL_BAUD_RATE 9600 // Serial port baud
#define INTERRUPT_PIN 4 // MPU-9250 INT pin tied to D4
#define SAMPLE_RATE 10  //10Hz.
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
void loop(){

//we are going digital usine fifo
if (!imu.fifoAvailable() ) { return; }
if (imu.dmpUpdateFifo() != INV_SUCCESS ){ return; }

// if ( digitalRead(INTERRUPT_PIN) == LOW ) // If MPU-9250 interrupt fires (active-low)
// imu.update();// Update all sensor's
	  String imuLog = ""; // Create a fresh line to log
	 imuLog += String(imu.time) + ","; // Add time to log string

     imu.computeEulerAngles2(false);
    // imu.computeEulerAngles2();

    imuLog += String(imu.yaw) + ",";
	imuLog += String(imu.pitch) + ",";
	imuLog += String(imu.roll) + ",";
	imuLog += String(imu.normal) + ",";

    imuLog += String(imu.ax) + ",";
	imuLog += String(imu.ay) + ",";
	imuLog += String(imu.az) + ",";

    imuLog += String(imu.gx) + ",";
	imuLog += String(imu.gy) + ",";
	imuLog += String(imu.gz) + ",";

    imuLog += String(imu.dqxr) + ",";
	imuLog += String(imu.dqyr) + ",";
	imuLog += String(imu.dqzr) + ",";
	imuLog += String(imu.dqwr) + ",";

    //normalized
    imuLog += String(imu.dqx) + ",";
	imuLog += String(imu.dqy) + ",";
	imuLog += String(imu.dqz) + ",";
	imuLog += String(imu.dqw) + ",";








	LOG_PORT.println(imuLog);
	//  imuLog += String(imu.roll, 2) + ", ";
	//  imuLog += String(imu.yaw, 2) + ", ";
//	write_to_ardunio(imuLog);
	
	//we are tring to get a 0 pitch angle

	float br_pitch_power_to_add=0;

	
	//float f=imu.pitch;	

	if(imu.pitch > 0.02){
		br_pitch_power_to_add= (imu.pitch) * pitch_ratio; 
	}else if(imu.pitch <  -0.02){
		br_pitch_power_to_add= (imu.pitch) * pitch_ratio; 
	}

	//we are dropping low and need more power

	if(imu.pitch > 2 && imu.pitch < 90){
		br_pitch_power_to_add= (imu.pitch) * pitch_ratio; 
	}else if(imu.pitch > 250 && imu.pitch < 358){
		br_pitch_power_to_add= -1*(360-imu.pitch) * pitch_ratio; 
	}

	//we are tring to get a 0 roational velocity
	//br - downward velocity is negative 
	//velocity at 0 has jitter of about 20 for some reason	
	float br_velocity_power_to_add=0;
	//we need upward velocity ..more power
////if(imu.gx < -40){
////	br_velocity_power_to_add= t_abs(imu.gx) * velocity_ratio; 
////}else if(imu.gx > 40){
////	//we need downward velocity ..less power
////	br_velocity_power_to_add=  -1 * imu.gx * velocity_ratio; 
////}

	new_br_power_add= (int) br_pitch_power_to_add + (int) br_velocity_power_to_add;

    //as front right goes up this go more negative,
    //so as front right increases when need to add more power to it
//   if(imu.gx >= (stablex+close_enough_to_being_stable))
//   {
//       //we need to add power 
//     fl_power_add=(t_abs(imu.gx)/power_add_ratio);
//   }else if(imu.gx <= (stablex-close_enough_to_being_stable)){
//       //we need to remove power 
//      fl_power_add=(t_abs(imu.gx)/power_add_ratio) * -1;//we are saying this is happneing to often
//   }

//if(new_br_power_add != br_power_add)	{
	 br_power_add=new_br_power_add;
	 int fl_power=fl_power_base-br_power_add;
	 int br_power=br_power_base+br_power_add;
    //  imuLog += String(fl_power) + ",";
    //  imuLog += String(br_power) + ",";
//	  write_to_ardunio(imuLog);
// } 	


     // ... do stuff with imu.ax, imu.ay, etc.
 //}
}

void write_to_ardunio(String tstring){

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



