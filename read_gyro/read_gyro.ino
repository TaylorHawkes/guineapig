#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#define SERIAL_BAUD_RATE 115200 // Serial port baud

#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h


MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class
unsigned short imu_update_rate=1000; //in hz
int gyro_address= 0x68; 
int temperature;
int acc_axis[4], gyro_axis[4];

double gyro_pitch, gyro_roll, gyro_yaw;

float gyro_roll_calibrate_offset, angle_roll, gyro_roll_input, pid_output_roll, pid_last_roll_d_error;

void setup() {

	Serial.begin(SERIAL_BAUD_RATE);
	Wire.begin();
	//imu_begin();
	if (imu.begin() != INV_SUCCESS){
	   Serial.println("could not start");
		return;
	}
//imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors
	imu.setGyroFSR(500); // Set gyro to 500 dps
	imu.setAccelFSR(8); // Set accel to +/-2g
	imu.setLPF(42); // Set LPF corner frequency to 5Hz
	
	/*CALIBRATION*/	
	//take 5 seconds to calibrate
	int total_loops=(5 * 250);
	float gyro_total=0;
    for (int i=0; i<total_loops; i++){
		delay(4);
		get_gyro_data();
		gyro_total+=gyro_roll;
	}
	gyro_roll_calibrate_offset=gyro_total/total_loops;

}


void loop(){
//gyro_roll_input = (gyro_roll_input * 0.7) + ((gyro_roll / 65.5) * 0.3);
//0.0000611 = 1 / (250Hz / 65.5)
angle_roll += gyro_roll * 0.0000611; //Calculate the traveled roll angle and add this to the angle_roll variable.
Serial.println(angle_roll);	
delay(2);//this loop should be 4000us
get_gyro_data();

return;
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
    data[0] = 0x80;
    arduino_i2c_write(gyro_address,0x6B, 1, data);
    delay(100);   

    /* Wake up chip. */
    data[0] = 0x00;
    arduino_i2c_write(gyro_address,0x6B, 1, data);
    delay(100);   

   //set data rate we may have to disable low power mode...
	unsigned char data_rate = 1000 / imu_update_rate - 1;
	arduino_i2c_write(gyro_address,0x19, 1, &data_rate);

	//set low pass filter to 42//I THINK
	arduino_i2c_write(gyro_address,0x1A, 1,0x03);
}

void get_gyro_data(){

    Wire.beginTransmission(gyro_address);                                   //Start communication with the gyro.
    Wire.write(0x3B);                                                       //Start reading @ register 43h and auto increment with every read.
    Wire.endTransmission();                                                 //End the transmission.
    Wire.requestFrom(gyro_address,14);                                      //Request 14 bytes from the gyro.
    
    while(Wire.available() < 6);                                           //Wait until the 14 bytes are received.
	acc_axis[1] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_x variable.
	acc_axis[2] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_y variable.
	acc_axis[3] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_z variable.
	temperature = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the temperature variable.

	gyro_axis[1] = Wire.read()<<8|Wire.read();//x rol                             //Read high and low part of the angular data.
	gyro_axis[2] = Wire.read()<<8|Wire.read();//y                              //Read high and low part of the angular data.
	gyro_axis[3] = Wire.read()<<8|Wire.read();//z  

	gyro_roll=gyro_axis[1] - gyro_roll_calibrate_offset;

}
  

