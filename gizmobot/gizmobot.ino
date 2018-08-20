//#include "giz_gps.h"
#include "giz_compass.h"
//#include "giz_wheel_encoder.h"

float position;//x,y from lat,lng
float current_heading;//0-360
float desired_heading;//0-360
double dt,last_micros;

//  Point WAYPOINTS[] = {
//    Point(-104.98004913, 39.88695907),
//    Point(-104.97996520, 39.88705062),
//    Point(-104.98004913, 39.88694763)
//  };

GizCompass giz_compass;

void setup() {

//setup gps 
//    GizGps.init();
//setup compass
 giz_compass.init();

Serial.begin(57600);

//setup wheel encoder 

//setup distance sensors

//setup stearing servo

//setup drive motor

    
}

void loop() {
    //delay(1000);
    dt=micros()-last_micros;
    last_micros=micros();

    //if(1hz_loop)

    //get our position && heading
    update_position(); 
    Serial.println(giz_compass.heading);
    update_current_heading();
    update_desired_heading();
    turn_to_desired_heading();

// if(1hz_loop){
//    if(did_hit_waypoint()){
//        update_to_next_waypoint();
//    }
//    if(did_finish()){
//        stop();
//    }
// }

}

static void increment_desired_waypoint(){

}

bool did_hit_waypoint(){

}

//updated desired heading based on current pos/waypoint
void update_desired_heading(){

}

void turn_to_desired_heading(){
    //PID control to desired heading
}


void update_position(){
    //update gps and put in x,y
 //   GizGps.update();
   // GizGps.x();
   // GizGps.y();
  //  GizWheelEncoder.update();
    //GizWheelEncoder.left();
    //GizWheelEncoder.right();

    //kalman complimentary filter/comp to update position 
    
}
void update_current_heading(){
    giz_compass.update();
    //combine compass + wheel encoder + gps heading
   // GizGps.x();
   // GizGps.y();
   // GizWheelEncoder.update();
    //GizWheelEncoder.left();
    //GizWheelEncoder.right();

    //kalman complimentary filter/comp to update position 
    
}


