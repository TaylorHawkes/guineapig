//τ=Kt(I−I0) //torche= Kt is the torque contatn and I is current Io is when current is 0
#include <stdlib.h>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <time.h>
#include <unistd.h>
#include "main.h"
#include "ball/util.h"
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

struct Quadcopter
{  
    struct Solid s; 
};

void init_glut(int argc, char** argv);
void update_sphere(float radius,float start_x, float start_y, float start_z);

//function definitions
double g_force(double mass1,double mass2,double dist);
struct Vec v_g_force(struct Solid s1, struct Solid s2);
double v_dot(struct Vec v1,struct Vec v2);
struct Vec v_prop(struct Vec v1,struct Vec v2);
double v_distance(struct Vec v1,struct Vec v2);
double v_distance_one(struct Vec v);
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

struct Baseball my_baseball;

void setup_sim(){
	my_baseball.s.center_of_mass.x=0;
	my_baseball.s.center_of_mass.y=EARTH_RADIUS+100;
	my_baseball.s.center_of_mass.z=0;
	my_baseball.s.mass=1;//kg
	my_baseball.s.velocity.x=0;//kg
	my_baseball.s.velocity.y=0;//kg
	my_baseball.s.velocity.z=0;//kg

    
}

void sim(){
	double hz=50;
    double seconds=1/hz;
    struct Earth e=EARTH();

   // struct Vec d=v_sub(my_baseball->s->center_of_mass,e.s.center_of_mass);

    struct Vec f=v_g_force(e.s,my_baseball.s);

    my_baseball.s.velocity=v_add( 
           my_baseball.s.velocity, 
           v_divide_s(v_mult_s(f,seconds),my_baseball.s.mass)
    );

    my_baseball.s.center_of_mass=v_add( 
      my_baseball.s.center_of_mass, 
      v_mult_s(my_baseball.s.velocity,seconds)
    );

    //hack for now to detect collision
    if(my_baseball.s.center_of_mass.y < EARTH_RADIUS){
        //in perfectly ellastic we simply flip velocity...? i think
        my_baseball.s.velocity.y=my_baseball.s.velocity.y*(.90) * -1 ;//bound coeficient
    }


    //printf("%lf",(float) my_baseball.s.center_of_mass.y); printf("\n");

    update_sphere(3,
        (float)	my_baseball.s.center_of_mass.x,
        (float) my_baseball.s.center_of_mass.y,
        (float) my_baseball.s.center_of_mass.z
    );

 //   update_sphere(1,0,EARTH_RADIUS+20,0);
    

   // (f*seconds)/my_baseball.mass;
   // my_baseball->velocity+=(f*seconds)/my_baseball->mass;

    //printf("%lf",my_baseball.s.velocity.y); printf("\n");
    //printf("%lf",my_baseball.s.center_of_mass.y); printf("\n");
    //printf("%lf",my_baseball->s.center_of_mass.z); printf("\n");
//  //printf("%lf",my_baseball->velocity); printf("\n");

}

/*display stuff*/

typedef struct {
  float *verticies;
  float *normals;
  short *indicies;//todo-malloc this is not working
  int verts_size_of; //store size_of verts/we use malloc and loose this
  int indicies_size_of;
  int normals_size_of;
 // float (*build_verticies)(const Shape *shape);
} Shape;

typedef struct {
  Shape s;
} Cube;

typedef struct {
  Shape s;
} Sphere;

Shape shapes[20];
int shape_count=0;


static float cube_build_verticies(const Shape *s)
{
  Cube *cube = (Cube *) s;//I think this is the magic where we caset
 //rect->veritices=[1,23,34]
}

void getNormals(float *normals,float verticies[],int verticies_size){
	int verticies_length=(verticies_size/sizeof(float)/4);//rows of m1;

	int i=0;
    for (i = 0; i < verticies_length/4; i++) {

		float U0 = verticies[i*4+1+0]-verticies[i*4+0+0];
		float U1 = verticies[i*4+1+1]-verticies[i*4+0+1];
		float U2 = verticies[i*4+1+2]-verticies[i*4+0+2];

		float V0 = verticies[i*4+2+0]-verticies[i*4+0+0];
		float V1 = verticies[i*4+2+1]-verticies[i*4+0+1];
		float V2 = verticies[i*4+2+2]-verticies[i*4+0+2];


        float x=(U1*V2)-(U2*V1);
        float y=(U2*V0)-(U0*V2);
        float z=(U0*V1)-(U1*V0);
		normals[i*3]=x;
		normals[i*3+1]=y;
		normals[i*3+2]=z;
    }

} 



//todo:make htis work
void get_indices(short *indicies ,int indicies_size){
	int i=0;
    for (i = 0; i < indicies_size/sizeof(short); i++){ 
		indicies[i]=(short) i;
    }
}


void new_sphere(Sphere *sphere,float radius,float start_x, float start_y, float start_z){
	int latitudeBands = 20;
	int longitudeBands = 20;
	float *verticies;

	int verts_size_of=(latitudeBands+1) * (longitudeBands+1) * 4 * sizeof(float);

	verticies=(float *) malloc(verts_size_of);
	//fprintf(stderr,"v index: %i\n",verts_size_of);
	int latNumber=0;
    int v_index=0;
     for (latNumber = 0; latNumber <= latitudeBands; latNumber++) {                                 
       float theta = latNumber * M_PI / latitudeBands;                                                 
       float sinTheta = sin(theta);                                                                  
       float cosTheta = cos(theta);
		int longNumber=0;
       for (longNumber = 0; longNumber <= longitudeBands; longNumber++) {                           
         float phi = longNumber * 2 * M_PI / longitudeBands;                                           
         float sinPhi = sin(phi);
         float cosPhi = cos(phi);                                                                    
         float x = cosPhi * sinTheta;                                                                     
         float y = cosTheta;
         float z = sinPhi * sinTheta;
		  verticies[v_index]=start_x+x*radius; v_index++;
		  verticies[v_index]=start_y+y*radius; v_index++;
		  verticies[v_index]=start_z+z*radius; v_index++;
		  verticies[v_index]=1; v_index++;
       } 
     }

//	fprintf(stderr,"v index: %i\n",v_index);

    short *indexData;
	int index_size_of=(latitudeBands+1) * (longitudeBands+1) * 6 * sizeof(short);
	indexData= (short *) malloc(index_size_of);

	 latNumber=0;
    int i_index=0;
    for (latNumber = 0; latNumber < latitudeBands; latNumber++) {
		int longNumber=0;
       for (longNumber = 0; longNumber < longitudeBands; longNumber++) {                            
         float first = (latNumber * (longitudeBands + 1)) + longNumber;                                   
         float second = first + longitudeBands + 1;                                                       
          
		//first triangel
         indexData[i_index]=(short) second;i_index++;
         indexData[i_index]=(short) first;i_index++;
         indexData[i_index]=(short) first+1;i_index++;

        //second triangle
 		 indexData[i_index]=(short) second+1;i_index++;
         indexData[i_index]=(short) second;i_index++;
         indexData[i_index]=(short) first+1;i_index++;
       }                                                                                                
     }                                                  

	 sphere->s.verticies=malloc(verts_size_of);
	 sphere->s.verts_size_of=verts_size_of;
	 memcpy(sphere->s.verticies, verticies, verts_size_of);

	 sphere->s.indicies=malloc(index_size_of);
	 sphere->s.indicies_size_of=index_size_of;
	 memcpy(sphere->s.indicies, indexData, index_size_of);

     
///  var verts_final=[]; 
///  for (var i = 0; i < indexData.length; i++) {
///      verts_final.push(verticies[indexData[i]]);                                                     
///  }

}

void new_cube(Cube *cube)
{
    float v[]={
        -1.0,-1.0,-1.0,1,
        -1.0,-1.0, 1.0,1,
        -1.0, 1.0, 1.0, 1,
        1.0, 1.0,-1.0, 1,
        -1.0,-1.0,-1.0,1,
        -1.0, 1.0,-1.0, 1,
        1.0,-1.0, 1.0,1,
        -1.0,-1.0,-1.0,1,
        1.0,-1.0,-1.0,1,
        1.0, 1.0,-1.0,1,
        1.0,-1.0,-1.0,1,
        -1.0,-1.0,-1.0,1,
        -1.0,-1.0,-1.0,1,
        -1.0, 1.0, 1.0,1,
        -1.0, 1.0,-1.0,1,
        1.0,-1.0, 1.0,1,
        -1.0,-1.0, 1.0,1,
        -1.0,-1.0,-1.0,1,
        -1.0, 1.0, 1.0,1,
        -1.0,-1.0, 1.0,1,
        1.0,-1.0, 1.0,1,
        1.0, 1.0, 1.0,1,
        1.0,-1.0,-1.0,1,
        1.0, 1.0,-1.0,1,
        1.0,-1.0,-1.0,1,
        1.0, 1.0, 1.0,1,
        1.0,-1.0, 1.0,1,
        1.0, 1.0, 1.0,1,
        1.0, 1.0,-1.0,1,
        -1.0, 1.0,-1.0,1,
        1.0, 1.0, 1.0,1,
        -1.0, 1.0,-1.0,1,
        -1.0, 1.0, 1.0,1,
        1.0, 1.0, 1.0,1,
        -1.0, 1.0, 1.0,1,
        1.0,-1.0, 1.0,1
    };

    //populate the verts
    cube->s.verticies=malloc(sizeof(v));
    cube->s.verts_size_of=sizeof(v);
    memcpy(cube->s.verticies, v, sizeof(v));

    //populate the indicies
    int size_of_indicies=sizeof(short) * (sizeof(v)/sizeof(float)) / 4;
    cube->s.indicies=malloc(size_of_indicies);
    cube->s.indicies_size_of=size_of_indicies;
    short t_indicies[size_of_indicies];
    get_indices(&t_indicies,size_of_indicies);
    memcpy(cube->s.indicies, t_indicies,size_of_indicies);
    
    //populate the normalsk

   int size_of_normals=sizeof(v) * 3/4 ;

   //fprintf(stderr, "Normals sizei s %i", size_of_normals);
   cube->s.normals=malloc(size_of_normals);
   cube->s.normals_size_of=size_of_normals;

   float t_normals[size_of_normals];
   getNormals(&t_normals,v,sizeof(v));
   memcpy(cube->s.normals, t_normals, size_of_normals);
}

/*
 * Global data used by our render callback:
 */

int mouse_state,mouse_button;
float move_x=0;
float move_y=0;
float rotate_x=0;
float rotate_y=0;
int old_x=0;
int old_y=0;
int zoom=0;

static struct {
    GLuint vertex_buffer, element_buffer, normals_buffer;
    GLuint textures[2];
    GLuint vertex_shader, fragment_shader, program;
    
    struct {
        GLint fade_factor;
        GLint textures[2];
        GLint Vmatrix;
        GLint Pmatrix;
        GLint Mmatrix;
    } uniforms;

    struct {
        GLint position;
        GLint normal;
    } attributes;

    float fade_factor;
    GLfloat Vmatrix[16];
    GLfloat Pmatrix[16];
    GLfloat Mmatrix[16];

} g_resources;

/*
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer(
    GLenum target,
    const void *buffer_data,
    GLsizei buffer_size
) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

static GLuint make_texture(const char *filename)
{
    int width, height;
    void *pixels = read_tga(filename, &width, &height);
    GLuint texture;

    if (!pixels)
        return 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );
    free(pixels);
    return texture;
}

static void show_info_log( GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length;
    char *log;
    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
   // fprintf(stderr, "%s", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}




//angle is degrees 
void getRotateXMatrix(float angle,float *r_matrix){
    float a=angle * (M_PI /180);
	r_matrix[0]=1;
	r_matrix[1]=0;
    r_matrix[2]=0;
    r_matrix[3]=0;

	r_matrix[4]=0;
	r_matrix[5]=cos(a); 
	r_matrix[6]= -1*sin(a); 
	r_matrix[7]=0; 

	r_matrix[8]=0;
    r_matrix[9]=sin(a);
    r_matrix[10]=cos(a);
    r_matrix[11]=0;

	r_matrix[12]=0;
	r_matrix[13]=0; 
	r_matrix[14]=0; 
	r_matrix[15]=1;
}

void getRotateYMatrix(float angle,float *r_matrix){
    //a is radiansl
    float a=angle * (M_PI /180);

	r_matrix[0]=cos(a); 
	r_matrix[1]=0;
    r_matrix[2]=sin(a); 
    r_matrix[3]=0;

	r_matrix[4]=0;
	r_matrix[5]=1;
	r_matrix[6]=0;
	r_matrix[7]=0; 

	r_matrix[8]=-sin(a);
    r_matrix[9]=0;
    r_matrix[10]=cos(a);
    r_matrix[11]=0;

	r_matrix[12]=0;
	r_matrix[13]=0; 
	r_matrix[14]=0; 
	r_matrix[15]=1;

}

void getViewMatrix(float *r_matrix){
	r_matrix[0]=1;
	r_matrix[1]=0;
    r_matrix[2]=0;
    r_matrix[3]=0;

	r_matrix[4]=0;
	r_matrix[5]=1;
	r_matrix[6]=0;
	r_matrix[7]=0; 

	r_matrix[8]=0;
    r_matrix[9]=0;
    r_matrix[10]=1;
    r_matrix[11]=0;

	r_matrix[12]=0;//x, 
	r_matrix[13]=-6371000;//y,
	r_matrix[14]=-500;//z
	r_matrix[15]=1;
}

void getModelMatrix( float *r_matrix){
	r_matrix[0]=1;
	r_matrix[1]=0;
    r_matrix[2]=0;
    r_matrix[3]=0;//x

	r_matrix[4]=0;
	r_matrix[5]=1;
	r_matrix[6]=0;
	r_matrix[7]=0; //y

	r_matrix[8]=0;
    r_matrix[9]=0;
    r_matrix[10]=1;
    r_matrix[11]=0;//z???

	r_matrix[12]=0;//x, or w/out transpose
	r_matrix[13]=0;//y,
	r_matrix[14]=0;//z
	r_matrix[15]=1;
}


void getProjectionMatrix( float *r_matrix,float angleOfView,float imageAspectRatio, float zMin,float zMax)
{

	float ang = tan((angleOfView*.5)*M_PI/180);//angle*.5                                                    

    r_matrix[0] = 0.5/ang; 
    r_matrix[1] = 0; 
    r_matrix[2] = 0; 
    r_matrix[3] = 0; 

    r_matrix[4] = 0; 
    r_matrix[5] = 0.5*imageAspectRatio/ang;
    r_matrix[6] = 0; 
    r_matrix[7] = 0; 

    r_matrix[8] = 0; 
    r_matrix[9] = 0;
    r_matrix[10] = -(zMax+zMin)/(zMax-zMin); 
    r_matrix[11] = -1; 

    r_matrix[12] = 0; 
    r_matrix[13] = 0; 
    r_matrix[14] = (-2*zMax*zMin)/(zMax-zMin);
    r_matrix[15] = 0;  

}



//This will onlly work with (x,4) * (4*4) matrix
void multiplyMatrices(float *verticies,float translation[],int verticies_size,int translation_size){

    float result[verticies_size];
	int verticies_length=(verticies_size/sizeof(float)/4);//rows of m1;
    int i=0;

    for (i = 0; i < verticies_length; i++) { 
        float x=verticies[(i*4)+0];
        float y=verticies[(i*4)+1];
        float z=verticies[(i*4)+2];
        float w=verticies[(i*4)+3];

        result[(i*4)+0]=(x*translation[0])+(y*translation[1])+(z*translation[2]) + (w*translation[3]);
        result[(i*4)+1]=(x*translation[4])+(y*translation[5])+(z*translation[6]) + (w*translation[7]) ;
        result[(i*4)+2]=(x*translation[8])+(y*translation[9])+(z*translation[10]) + (w*translation[11]);
        result[(i*4)+3]=(x*translation[12])+(y*translation[13])+(z*translation[14]) + (w*translation[15]);
    }
   
    int t=0;
	//there must be a better way to do this
    for (t = 0; t <= (sizeof(result)/sizeof(float)); t++) {
		verticies[t]=result[t];
	}
}

//tay I think this will only work if m2 is a cube because we are looping over its rows not columns
//This will onlly work with (x,3) * (3*3) matrix
/*
void multiplyMatrices(float *m1,float m2[],int m1size,int m2size){

    float result[m1size];
	int i,j,k;
	int m1_rows=(m1size/sizeof(float)/3);//rows of m1;
	int m2_rows=(m2size/sizeof(float)/3);//rows of m1;
	int m1_cols=3;
	int m2_cols=3;

	//i is column
	//loop through  the length of matrix 1 rows
    for (i = 0; i < m1_rows; i++) {                                                                
        //result[i] = []; 
        for (j = 0; j < m2_cols; j++){

            float sum = 0; 
			//then loop throu lenght of m1 columns
            for (k = 0; k < m1_cols; k++) {                                                     
				//so is the m1 row and k m1 and m2 column
				//j is the m2 column
                sum += m1[i*3+k] * m2[k*3+j];                                                              

			 //fprintf(stderr,"Adding :%f\n", m2[k*3+j]);
			 //fprintf(stderr,"Adding :%f\n", m1[i*3+k]);
            }
            result[i*3+j] = sum;                                                                          
        }
    }

	//there must be a better way to do this
    for (i = 0; i <= (sizeof(result)/sizeof(float)); i++) {
		m1[i]=result[i];
	}

   // return result;                                                                                       
}                                                                                                        
*/



void move(float verts[],int size){
	int x=0;	

	 fprintf(stderr, "OpenGL 2.0 not available\n");
	 //fprintf(stderr,"Size of verts:%i\n",sizeof(float));
	 fprintf(stderr,"Size of GL Float:%i\n",size);
	 //fprintf(stdout,"COOL\n");
	for (x=0; x< size/sizeof(float); ++x) {
        fprintf(stderr,"%f\n", verts[x]);
	}
} 


void replace_shape(Shape *shape){
	shapes[1]=*shape;
	//shape_count++;
}

void add_shape(Shape *shape){
	shapes[shape_count]=*shape;
	shape_count++;

//   g_resources.vertex_buffer = make_buffer(
//          GL_ARRAY_BUFFER,
//          shape->verticies,
//          shape->verts_size_of
//   );

    //normals need to update as the objecgt uptates:todo
////g_resources.normals_buffer = make_buffer(
////	 GL_ARRAY_BUFFER,
////	 shape->normals,
////	 shape->normals_size_of
//// );

///  g_resources.element_buffer = make_buffer(
///       GL_ELEMENT_ARRAY_BUFFER,
///       shape->indicies,
///       shape->indicies_size_of
/// );
}


/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{

    //g_resources.textures[0] = make_texture("hello1.tga");
    //g_resources.textures[1] = make_texture("hello2.tga");

//  if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0) {
//      return 0;
//  }

    g_resources.vertex_shader = make_shader(
        GL_VERTEX_SHADER,
        "/Users/taylorhawkes/Desktop/guinea_pig.h/simulate/ball/hello-gl.v.glsl"
    );
    if (g_resources.vertex_shader == 0)
        return 0;

    g_resources.fragment_shader = make_shader(
        GL_FRAGMENT_SHADER,
        "/Users/taylorhawkes/Desktop/guinea_pig.h/simulate/ball/hello-gl.f.glsl"
    );
    if (g_resources.fragment_shader == 0)
        return 0;

    g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);
    if (g_resources.program == 0)
        return 0;

    g_resources.uniforms.fade_factor = glGetUniformLocation(g_resources.program, "fade_factor");
    g_resources.uniforms.Vmatrix = glGetUniformLocation(g_resources.program, "Vmatrix");
	g_resources.uniforms.Pmatrix = glGetUniformLocation(g_resources.program, "Pmatrix");
	g_resources.uniforms.Mmatrix = glGetUniformLocation(g_resources.program, "Mmatrix");
    //g_resources.uniforms.textures[0] = glGetUniformLocation(g_resources.program, "textures[0]");
    //g_resources.uniforms.textures[1] = glGetUniformLocation(g_resources.program, "textures[1]");
    g_resources.attributes.position = glGetAttribLocation(g_resources.program, "position");
    g_resources.attributes.normal = glGetAttribLocation(g_resources.program, "normal");

    return 1;
}


struct timespec last, now;
double elapsed = 0;
double hz=50;// run simulation 100 times per second


/*
 * GLUT callbacks:
 */
static void update_fade_factor(void)
{
	last = now;

	usleep(100); // Sleep for 1/1000 second

	clock_gettime(CLOCK_MONOTONIC, &now);

	if(!last.tv_nsec){
        return;
    }

	elapsed += (1000000000 * (double)(now.tv_sec  - last.tv_sec)) +
			   (double)(now.tv_nsec - last.tv_nsec);


	while (elapsed >= MS_TO_NS(1000/hz)) {
        elapsed -= MS_TO_NS(1000/hz);
        sim();
		glutPostRedisplay();
		//render new glut
		 //render();
	}

}




//this is the function getting called a bunch
void render()
{
    
   glFlush();//clear
   glClearColor(1.0,1.0,1.0,1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


   glUseProgram(g_resources.program);
   glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);

   getViewMatrix(&g_resources.Vmatrix); //start at -10z
   getModelMatrix(&g_resources.Mmatrix); 

   //y, axes rotation -- we rotate the model not the view
   float r_y_m[16];
   getRotateYMatrix(rotate_y,&r_y_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_y_m,sizeof(g_resources.Mmatrix),sizeof(r_y_m));

   float r_x_m[16];
   getRotateXMatrix(rotate_x,&r_x_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_x_m,sizeof(g_resources.Mmatrix),sizeof(r_x_m));


    glUniformMatrix4fv(g_resources.uniforms.Mmatrix, 1, GL_FALSE, &g_resources.Mmatrix);
    glUniformMatrix4fv(g_resources.uniforms.Vmatrix, 1, GL_FALSE, &g_resources.Vmatrix); 

    getProjectionMatrix(&g_resources.Pmatrix, 40+(zoom*2), 1, .1, 10000); 
    glUniformMatrix4fv( g_resources.uniforms.Pmatrix, 1, GL_FALSE, &g_resources.Pmatrix);
    
//  glActiveTexture(GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
//  glUniform1i(g_resources.uniforms.textures[0], 0);

/// glActiveTexture(GL_TEXTURE1);
/// glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
/// glUniform1i(g_resources.uniforms.textures[1], 1);

	//loop over and set buffer
	int s=0;
	for(s=0;s<shape_count;s++){
	   g_resources.vertex_buffer = make_buffer(
			  GL_ARRAY_BUFFER,
			  shapes[s].verticies,
			  shapes[s].verts_size_of
	   );
	  g_resources.element_buffer = make_buffer(
		   GL_ELEMENT_ARRAY_BUFFER,
		   shapes[s].indicies,
		   shapes[s].indicies_size_of
	  );

		//bind positions
		glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
		glVertexAttribPointer(g_resources.attributes.position, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
		glEnableVertexAttribArray(g_resources.attributes.position);

	//bind the normals buffer
	  //glBindBuffer(GL_ARRAY_BUFFER, g_resources.normals_buffer);
	  //glVertexAttribPointer(g_resources.attributes.normal, 3,GL_FLOAT,GL_FALSE, sizeof(float)*3, (void*)0);
	  //glEnableVertexAttribArray(g_resources.attributes.normal);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
		
		glDrawElements(
			GL_TRIANGLES,  /* mode */
			shapes[s].verts_size_of, /* count */
			GL_UNSIGNED_SHORT,  /* type */
			(void*)0            /* element array buffer offset */
		);

	}

    glDisableVertexAttribArray(g_resources.attributes.position);
    glDisableVertexAttribArray(g_resources.attributes.normal);
    glutSwapBuffers();
}



void mouse(int button, int state, int x, int y)
{

	mouse_state =state;
	mouse_button =button;
    old_x = x; 
  	old_y = y;
}

void motion(int x, int y) {
	if (mouse_state == GLUT_DOWN){

            rotate_x += (y-old_y)*50*M_PI/1000;
			rotate_y +=  (x-old_x)*50*M_PI/1000;
			//fprintf(stderr," x old  is at:%f\n",rotate_x);
			//fprintf(stderr," x is at:%i\n",x);
			//fprintf(stderr,"rotate x is at:%i\n",rotate_x);
			old_x=x;
			old_y=y;
			glutPostRedisplay();
	}
}

void keyPressed (unsigned char key, int x, int y)
{
    if (key == 'j') {
         zoom=zoom+1;
    } else if(key=='k'){
         zoom=zoom-1;
    }
	glutPostRedisplay();
}
void say_high(){
	printf("HELLO DOG");
}

void init_glut(int argc, char** argv){
     glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
     glutInitWindowSize(1000, 1000);
     glutCreateWindow("Hello World");
     glutIdleFunc(&update_fade_factor);
     glutDisplayFunc(&render);
     //used to detect stuff
     glutMouseFunc(mouse);
     glutMotionFunc(motion);
     glutKeyboardFunc(keyPressed);

     glewInit();
  
     //add shapes after glew init
  ///Cube cube1;
  ///new_cube(&cube1);
     
	//we will init w/ the world
	 Sphere spere1;
     new_sphere(&spere1,6371000,0,0,0);
     add_shape(&spere1.s);

 	 Sphere spere2;
  	 new_sphere(&spere2,1,1,6371030,0);
  	 add_shape(&spere2.s);

 /// Sphere spere3;
 /// new_sphere(&spere3,1,0,1002,0);
 /// add_shape(&spere3.s);

    // add_shape(&cube1.s);

     if (!GLEW_VERSION_2_0) {
         fprintf(stderr, "OpenGL 2.0 not available\n");
         return ;
     }

     if (!make_resources()) {
         fprintf(stderr, "Failed to load resources\n");
         return ;
     }

	 clock_gettime(CLOCK_MONOTONIC, &last);
	//glutMainLoopEvent();
     glutMainLoop();
}



void update_sphere(float radius,float start_x, float start_y, float start_z){

      printf("y:%lf",(float) start_y); printf("\n");
      
      Sphere sphere3;
      new_sphere(&sphere3,radius,start_x,start_y,0);
      replace_shape(&sphere3.s);
}
/*end dsiplay stuff*/



int main(int argc, char** argv){
setup_sim();
init_glut(argc, argv);
return 0;

}







