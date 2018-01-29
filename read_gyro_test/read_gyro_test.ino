#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library
#define SERIAL_BAUD_RATE 9600 // Serial port baud
#include <Wire.h> // Depending on your Arduino version, you may need to include Wire.h



int acc_axis[4], gyro_axis[4], euler_axis[4];
int temperature;
int gyro_address= 0x68; 
MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class

void setup() {

	Serial.begin(SERIAL_BAUD_RATE);
Wire.begin();
Wire.begin();
Serial.println("ok");
Wire.setClock(1000000);//this appears to be as fast as we can go
   // Wire.setClock(1000000);//this appears to be as fast as we can go

//	imu_begin();
    if (imu.begin() != INV_SUCCESS){
       Serial.println("could not start");
        return;
    }

    imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL ); // Enable all sensors
    imu.setGyroFSR(500); // Set gyro to 500 dps
    imu.setAccelFSR(8); // Set accel to +/-2g
    imu.setLPF(42); // Set LPF corner frequency to 5Hz // id don't think we can do this with sample rate at 1000
    imu.setSampleRate(1000); // Set LPF corner frequency to 5Hz
    Serial.println("updating");
    imu.update(UPDATE_ACCEL | UPDATE_GYRO);
    Serial.println(imu.ax);
   imu.update(UPDATE_ACCEL | UPDATE_GYRO);
    Serial.println(imu.ax);



//get_gyro_data(0);
////get_gyro_data(0);
////get_gyro_data(0);

}
void loop(){
     delay(500);   
     imu.update(UPDATE_ACCEL | UPDATE_GYRO );
    Serial.println(imu.ax);

    }

void get_gyro_data(double dt_p){

    Wire.beginTransmission(gyro_address);                                   //Start communication with the gyro.
    Wire.write(0x3B);                                                       //Start reading @ register 43h and auto increment with every read.
    Wire.endTransmission(false);                                            //End the transmission.
    Wire.requestFrom(gyro_address,1);                                      //Request 14 bytes from the gyro.
    
    Serial.println("getting");
    while(Wire.available() < 1);                                           //Wait until the 14 bytes are received.
	acc_axis[1] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_x variable.
	acc_axis[2] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_y variable.
	acc_axis[3] = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the acc_z variable.
	temperature = Wire.read()<<8|Wire.read();                               //Add the low and high byte to the temperature variable.
	gyro_axis[1] = Wire.read()<<8|Wire.read();//x rol                             //Read high and low part of the angular data.
	gyro_axis[2] = Wire.read()<<8|Wire.read();//y                              //Read high and low part of the angular data.
	gyro_axis[3] = Wire.read()<<8|Wire.read();//z  

    Serial.println("done getting");
}


