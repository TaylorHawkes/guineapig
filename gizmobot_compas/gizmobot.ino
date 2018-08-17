#include <LSM303.h>
#include <Wire.h>
// compass connects to 20/21 (SDA/SCL)
// compass connects to A4/A5

///////////////////////
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M 0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

  LSM303 compass;
  int compassMin;
  int compassMax;

void init_compass() {
  Serial.println(F("Initializing LSM303 compass. Please wait... "));
  Wire.begin();
  
  compass.init();
  compass.enableDefault();
  
  // Calibration values...
  compass.m_min = (LSM303::vector<int16_t>){445, -1242, -1005};
  compass.m_max = (LSM303::vector<int16_t>){1468, -184, -124};

  compassMin = 360;
  compassMax = 0;

  Serial.println(F("Compass initialized!"));
}


void setup() {
    Serial.begin(115200);
    init_compass();
}

void loop() {
  compass.read();
  Serial.println(compass.heading());

//float current_bearing  = compass.heading() - 92; // Convert from -Y to +X, minus the chassis offset
//if (current_bearing < 0) {
//  current_bearing += 360;
//}


}
