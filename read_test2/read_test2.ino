#include <SoftwareSerial.h> //I think this uses Timer1
SoftwareSerial gsm(2, 3);

void setup()
{
  Serial.begin(4800);           // start serial for output 
  Serial.println("starting");
  delay(20000);
  gsm.begin(4800);
  gsm.write("AT+CIPMODE=1\r"); res();
  gsm.write("AT+NETOPEN\r"); res();
  gsm.write("AT+CIPOPEN=0,\"UDP\",\"23.92.20.100\",9999,9000\r"); res();
  gsm.write("hey"); 
}

void res(){
  delay(5000);
  while (gsm.available() > 0){
     Serial.write(gsm.read());
  };
}

void loop()
{
  delay(100);
  gsm.write("hey"); 
}

