#include <Wire.h>
#define MPL3115A2_ADDRESS 0x60
#define OUT_P_MSB  0x01    
#define CTRL_REG1  0x26 


void setup() {
	Wire.begin();  
	Serial.begin(9600);
}

void toggleOneShot(void)                                               
{                                                                                 

 //read
  Wire.beginTransmission(MPL3115A2_ADDRESS);                                      
  Wire.write(CTRL_REG1);  // Address of CTRL_REG1                                   
  Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
  Wire.requestFrom(MPL3115A2_ADDRESS, 1); // Request the data...
  byte tempSetting=Wire.read();

  tempSetting &= ~(1<<1); //Clear OST bit                                         

  Wire.beginTransmission(MPL3115A2_ADDRESS);
  Wire.write(CTRL_REG1);
  Wire.write(tempSetting);
  Wire.endTransmission(true);

  //do it again ? maybe
                                                                                  
//  tempSetting = IIC_Read(CTRL_REG1); //Read current settings to be safe           
//  tempSetting |= (1<<1); //Set OST bit                                            
//  IIC_Write(CTRL_REG1, tempSetting);                                              
}                                                                                


void loop() {
  
	toggleOneShot();
    Wire.beginTransmission(MPL3115A2_ADDRESS);                                    
    Wire.write(OUT_P_MSB);
    Wire.endTransmission(false);
    Wire.requestFrom(MPL3115A2_ADDRESS,3);

     byte msb, csb, lsb;
    msb = Wire.read();
    csb = Wire.read();
    lsb = Wire.read();

    // The least significant bytes l_altitude and l_temp are 4-bit,
    // fractional values, so you must cast the calulation in (float),
    // shift the value over 4 spots to the right and divide by 16 (since 
    // there are 16 values in 4-bits). 
    float tempcsb = (lsb>>4)/16.0;

    float altitude = (float)( (msb << 8) | csb) + tempcsb;

	Serial.println(altitude);
    
}
 
