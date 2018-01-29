
void setup(){
  // DDRB |= B00010000;//set pin 12 to output  
   //pinMode(12,OUTPUT);
   pinMode(4,OUTPUT);
   pinMode(5,OUTPUT);
   pinMode(6,OUTPUT);
   pinMode(7,OUTPUT);
}

//we are going to go at 250hz which means
//4000uz frames
//we should have
void loop(){
    //total 4000us
      
    // timer=micros(); //this takes like 3.5 microseconds 
  //   PORTB |= B00010000;   //set 12 to high //digitalWrite(12,HIGH);    

     PORTD |= B11110000; //Set digital outputs 4,5,6 and 7 high.
     delayMicroseconds(1000);
     PORTD &= B11101111;                //Set digital output 4 to low if the time is expired.
     PORTD &= B11011111;                //Set digital output 5 to low if the time is expired.
     PORTD &= B10111111;                //Set digital output 6 to low if the time is expired.
     PORTD &= B01111111;                //7 to lo   
     delayMicroseconds(4000);
}
