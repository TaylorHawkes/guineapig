
void setup(){
   DDRB |= B00010000;//set pin 12 to output  
   //pinMode(12,OUTPUT);
}

//we are going to go at 250hz which means
//4000uz frames
//we should have
void loop(){
    //total 4000us
      
    // timer=micros(); //this takes like 3.5 microseconds 
     PORTB |= B00010000;   //set 12 to high //digitalWrite(12,HIGH);    
     delayMicroseconds(2000);
     PORTB &= B11101111;   //set 12 to low digitalWrite(12,LOW);    
     delayMicroseconds(2000);
}
