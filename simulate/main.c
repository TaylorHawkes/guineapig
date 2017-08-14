//τ=Kt(I−I0) //torche= Kt is the torque contatn and I is current Io is when current is 0
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>
#define MS_TO_NS(x) (1000000*(x))

//Constants
double const GRAVITY_CONSTANT = 6.67408E-11;
double const EARTH_MASS = 5.972E+24;//kg
double const EARTH_RADIUS = 6371000;//in meters
//structure definiti
struct Vec{
    double x,y,z;
};

struct Solid
{
 struct Vec center_of_mass;
 struct Vec ineritia;
 struct Vec velocity;
 double mass;
};

struct Earth
{  
  struct Solid s;
};

struct Baseball
{  
    struct Solid s; 
};

//function definitions
double g_force(double mass1,double mass2,double dist);
struct Vec v_g_force(struct Solid s1, struct Solid s2);
double v_dot(struct Vec v1,struct Vec v2);
struct Vec v_prop(struct Vec v1,struct Vec v2);
double v_distance(struct Vec v1,struct Vec v2);
double v_distance_one(struct Vec v);
void sim(struct Baseball *my_baseball, double hz);
struct Vec v_norm(struct Vec v);
struct Vec v_sub_abs(struct Vec a,struct Vec b);
struct Vec v_mult_s(struct Vec v,double m);
struct Vec v_mult(struct Vec a,struct Vec b);
struct Vec v_divide_s(struct Vec v,double m);



struct Earth EARTH(){
    struct Earth e;
    e.s.center_of_mass.x=1000;
    e.s.center_of_mass.y=0;
    e.s.center_of_mass.z=0;
    e.s.mass=5.972E+24;
    return e;
};


struct Vec v_mult(struct Vec a,struct Vec b){
        struct Vec v;
        v.x=a.x*b.x;
        v.y=a.y*b.y;
        v.z=a.z*b.z;
        return v;
}

struct Vec v_sub(struct Vec a,struct Vec b){
        struct Vec v;
        v.x=a.x-b.x;
        v.y=a.y-b.y;
        v.z=a.z-b.z;
        return v;
}

struct Vec v_sub_abs(struct Vec a,struct Vec b){
        struct Vec v;
        v.x=fabs(a.x-b.x);
        v.y=fabs(a.y-b.y);
        v.z=fabs(a.z-b.z);

        return v;
}

struct Vec v_add(struct Vec a,struct Vec b){
        struct Vec v;
        v.x=a.x+b.x;
        v.y=a.y+b.y;
        v.z=a.z+b.z;
        return v;
}

//force in neutons 
double g_force(double mass1,double mass2,double dist){
     double gForce;
     gForce = (GRAVITY_CONSTANT * mass1 * mass2) / pow(dist,2);
     return gForce;
}

//get the g_force vector for structure one, realitve to structure 2 
struct Vec v_g_force(struct Solid s1, struct Solid s2){

     double d=v_distance(s1.center_of_mass,s2.center_of_mass);
     double p=v_dot(s1.center_of_mass,s2.center_of_mass);
     double f=g_force(s1.mass,s2.mass,d);
     
     //struct Vec vdiff=v_sub(s1.center_of_mass,s2.center_of_mass);
     //return vdiff;
        
     struct Vec prop=v_prop(s1.center_of_mass,s2.center_of_mass);

     struct Vec vfn; //forces in neutons acting on x,y,z
     vfn.x=f*prop.x;
     vfn.y=f*prop.y;
     vfn.z=f*prop.z;

     return vfn;
      
}

//force in neutons  (9.819650)
double EARTH_GRAVITY_FORCE_ON_SURFACE(){
    return g_force((double) 1,EARTH_MASS,EARTH_RADIUS);
}



/*Basic Vector Function */
//vectors should be normalized for this to be helpful
//returns a value between -1 and 1 (-1 means 180 0 means 90 and 1 means same direction (or no angle))
//go ahead an normalize just in ase
double v_dot(struct Vec v1,struct Vec v2){                                                                                  
  struct Vec v1n=v_norm(v1); 
  struct Vec v2n=v_norm(v2); 
  return v1n.x * v2n.x + v1n.y * v2n.y + v1n.z * v2n.z; 
}

//vector multtiply by scalar
struct Vec v_mult_s(struct Vec v,double m){
    struct Vec vf;
    vf.x=v.x*m;
    vf.y=v.y*m;
    vf.z=v.z*m;
    return vf;
}

//vector multtiply by scalar
struct Vec v_divide_s(struct Vec v,double m){
    struct Vec vf;
    vf.x=v.x/m;
    vf.y=v.y/m;
    vf.z=v.z/m;
    return vf;
}



//gets realive x,y,z proportion apply to x,y,z
struct Vec v_prop(struct Vec v1,struct Vec v2){
    struct Vec va =v_sub_abs(v1,v2);
    double d_total=va.x+va.y+va.z;
    
    struct Vec v;
    v.x=(v1.x-v2.x) / d_total;
    v.y=(v1.y-v2.y) / d_total;
    v.z=(v1.z-v2.z) / d_total;
    return v;
}


//get distance(magniture) between to vectos
double v_distance(struct Vec v1,struct Vec v2){
    return sqrt(
        ((v2.x-v1.x) * (v2.x-v1.x))+ 
        ((v2.y-v1.y) * (v2.y-v1.y))+
        ((v2.z-v1.z) * (v2.z-v1.z))
      );
}

double v_distance_one(struct Vec v){
   return sqrt(
        (v.x*v.x) +
        (v.y*v.y) +
        (v.z*v.z)
    );
}
//normailize to 1 from 0,0,0
struct Vec v_norm(struct Vec v){
    double l=sqrt(
        (v.x*v.x) +
        (v.y*v.y) +
        (v.z*v.z)
    );
	struct Vec nv;
	nv.x=v.x/l;
	nv.y=v.y/l;
	nv.z=v.z/l;
	return nv;
}


void sim(struct Baseball *my_baseball, double hz){
    double seconds=1/hz;
    struct Earth e=EARTH();
   // struct Vec d=v_sub(my_baseball->s->center_of_mass,e.s.center_of_mass);
    struct Vec f=v_g_force(e.s,my_baseball->s);

    my_baseball->s.velocity=v_add( 
         my_baseball->s.velocity, 
         v_divide_s(v_mult_s(f,seconds),my_baseball->s.mass)
    );

    my_baseball->s.center_of_mass=v_add( 
      my_baseball->s.center_of_mass, 
      v_mult_s(my_baseball->s.velocity,seconds)
    );
    

   // (f*seconds)/my_baseball.mass;
   // my_baseball->velocity+=(f*seconds)/my_baseball->mass;

    printf("%lf",my_baseball->s.velocity.y); printf("\n");
    //printf("%lf",my_baseball->s.center_of_mass.x); printf("\n");
    printf("%lf",my_baseball->s.center_of_mass.y); printf("\n");
    //printf("%lf",my_baseball->s.center_of_mass.z); printf("\n");
//  //printf("%lf",my_baseball->velocity); printf("\n");
}

int main(){

struct timespec last, now;
clock_gettime(CLOCK_MONOTONIC, &last);
double elapsed = 0;
double hz=100;// run simulation 100 times per second

struct Baseball my_baseball;
my_baseball.s.center_of_mass.x=0;
my_baseball.s.center_of_mass.y=EARTH_RADIUS+20;
my_baseball.s.center_of_mass.z=0;
my_baseball.s.mass=1;//kg
my_baseball.s.velocity.x=0;//kg
my_baseball.s.velocity.y=0;//kg
my_baseball.s.velocity.z=0;//kg

while (1) {
	last = now;

	usleep(100); // Sleep for 1/1000 second

	clock_gettime(CLOCK_MONOTONIC, &now);

    if(!last.tv_nsec){
        continue;
    }

	elapsed += (1000000000 * (double)(now.tv_sec  - last.tv_sec)) +
			   (double)(now.tv_nsec - last.tv_nsec);

	while (elapsed >= MS_TO_NS(1000/hz)) {
        elapsed -= MS_TO_NS(1000/hz);
        sim(&my_baseball,hz);
	}
}


}






