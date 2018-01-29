#include <stdlib.h>
# include <GL/glew.h>
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
#include "sim.h"
#include "ball/util.h"
#define MS_TO_NS(x) (1000000*(x))
#define NS_TO_MS(x) ((x)/1000000)


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





//Constants
double const GRAVITY_CONSTANT = 6.67408E-11;
double const EARTH_MASS = 5.972E+24;//kg
double const EARTH_RADIUS = 6371000;//in meters


Quadcopter new_quadcopter(){
     Quadcopter q;
     Force fr;
     fr.point.x=1;
     fr.point.y=-1;
     fr.point.z=-1;//-z is into screen
    
     fr.force.x=0;
     fr.force.y=0;
     fr.force.z=0;
     fr.start=0;

     Force fl;
     fl.point.x=-1;
     fl.point.y=-1;
     fl.point.z=-1;
     fl.force.x=0;
     fl.force.y=0;
     fl.force.z=0;
     fl.start=0;

     Force bl;
     bl.point.x=-1;
     bl.point.y=-1;
     bl.point.z=1;
     bl.force.x=0;
     bl.force.y=0;
     bl.force.z=0;
     bl.start=0;

    Force br;
     br.point.x=1;
     br.point.y=-1;
     br.point.z=1;
     br.force.x=0;
     br.force.y=0;
     br.force.z=0;
     br.start=0;


     Force wind;
     wind.point.x=0;
     wind.point.y=1;
     wind.point.z=0;

     wind.force.x=0;
     wind.force.y=1;
     wind.force.z=0;

     wind.start=2000;
     wind.stop=10000;

   Force wind2;
   wind2.point.x=0;
   wind2.point.y=1;
   wind2.point.z=0;

 //  wind2.force.x=0;
 //  wind2.force.y=1;
 //  wind2.force.z=0;
 //  wind2.start=4700;
 //  wind2.stop=10000;




     Force gravity;
     gravity.point.x=0;
     gravity.point.y=0;
     gravity.point.z=0;
     gravity.force.x=0;
     gravity.force.y=-9.8;
     gravity.force.z=0;
     gravity.start=0;
    

     Force upwardLiftOneSec;
     upwardLiftOneSec.point.x=0;
     upwardLiftOneSec.point.y=0;
     upwardLiftOneSec.point.z=0;
     upwardLiftOneSec.force.x=0;
     upwardLiftOneSec.force.y=50;
     upwardLiftOneSec.force.z=0;
     upwardLiftOneSec.start=21000;
     upwardLiftOneSec.stop=24000;

    Cube cube;
    new_cube(&cube);
    cube.s.mass=1;
    cube.s.forces[0]=fr;
    cube.s.forces[1]=fl;
    cube.s.forces[2]=br;
    cube.s.forces[3]=bl;
    cube.s.forces[4]=wind;
    //cube.s.forces[5]=wind2;
   // cube.s.forces[5]=gravity;
////cube.s.forces[5]=wind;

    cube.s.forces_count=5;
    cube.s.orientation.x=0;
    cube.s.orientation.y=0;
    cube.s.orientation.z=0;
    cube.s.orientation.w=1;//identity quaternion

    cube.s.position.x=0;
    cube.s.position.y=0;
    cube.s.position.z=0;

    q.cube=cube;

   return q; 
}

Quadcopter_hover(Quadcopter * q){
  q->cube.s.forces[0].force.y=(float)9.8/4;
  q->cube.s.forces[1].force.y=(float)9.8/4;
  q->cube.s.forces[2].force.y=(float)9.8/4;
  q->cube.s.forces[3].force.y=(float)9.8/4;
}



//global variables
Solid solids[2];
int solid_count=0;

struct timespec last, now;
double elapsed = 0;
double elapsed2 = 0;
double total_elapsed = 0;
double hz=100;//times per second
double hz2=10;//this if for stabilizing 
Quadcopter quadcopter;

Control controls[3];
int controls_count=0;

Quat new_quat(float angle,Vec axis){
    Quat q;
    q.w=cos(angle/2);
    q.x=axis.x*sin(angle/2);
    q.y=axis.y*sin(angle/2);
    q.z=axis.z*sin(angle/2);
    return q;
}


void new_cube(Cube *cube){
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
    cube->s.verticies_size=sizeof(v);
    memcpy(cube->s.verticies, v, sizeof(v));

    //populate the indicies
    int indicies_size=sizeof(short) * (sizeof(v)/sizeof(float)) / 4;
    cube->s.indicies=malloc(indicies_size);
    cube->s.indicies_size=indicies_size;
    short t_indicies[indicies_size];
    get_indices(&t_indicies,indicies_size);
    memcpy(cube->s.indicies, t_indicies,indicies_size);
    
    //populate the normalsk

//   int size_of_normals=sizeof(v) * 3/4 ;

   //fprintf(stderr, "Normals sizei s %i", size_of_normals);
// cube->s.normals=malloc(size_of_normals);
// cube->s.normals_size_of=size_of_normals;

// float t_normals[size_of_normals];
// getNormals(&t_normals,v,sizeof(v));
// memcpy(cube->s.normals, t_normals, size_of_normals);
    
    //setup default model matrix

    // float m_matrix[16];
// getNormals(&t_normals,v,sizeof(v));
   // getModelMatrix(m_matrix);

	cube->s.inertia_tensor[0]=1;
	cube->s.inertia_tensor[1]=0;
    cube->s.inertia_tensor[2]=0;
    cube->s.inertia_tensor[3]=0;

	cube->s.inertia_tensor[4]=0;
	cube->s.inertia_tensor[5]=1;
	cube->s.inertia_tensor[6]=0;
	cube->s.inertia_tensor[7]=0; 

	cube->s.inertia_tensor[8]=0;
    cube->s.inertia_tensor[9]=0;
    cube->s.inertia_tensor[10]=1;
    cube->s.inertia_tensor[11]=0;

	cube->s.inertia_tensor[12]=0;//x, or w/out transpose
	cube->s.inertia_tensor[13]=0;//y,
	cube->s.inertia_tensor[14]=0;//z
	cube->s.inertia_tensor[15]=1;


     float m_matrix[16];
// getNormals(&t_normals,v,sizeof(v));
   // getModelMatrix(m_matrix);

	cube->s.model_matrix[0]=1;
	cube->s.model_matrix[1]=0;
    cube->s.model_matrix[2]=0;
    cube->s.model_matrix[3]=0;//x
	cube->s.model_matrix[4]=0;
	cube->s.model_matrix[5]=1;
	cube->s.model_matrix[6]=0;
	cube->s.model_matrix[7]=0; //y
	cube->s.model_matrix[8]=0;
    cube->s.model_matrix[9]=0;
    cube->s.model_matrix[10]=1;
    cube->s.model_matrix[11]=0;//z???
	cube->s.model_matrix[12]=0;//x, or w/out transpose
	cube->s.model_matrix[13]=0;//y,
	cube->s.model_matrix[14]=0;//z
	cube->s.model_matrix[15]=1;

}

void get_position_matrix(float * m,Vec p){
	m[0]=1;
	m[1]=0;
    m[2]=0;
    m[3]=0;//x
	m[4]=0;
	m[5]=1;
	m[6]=0;
	m[7]=0; //y
	m[8]=0;
    m[9]=0;
    m[10]=1;
    m[11]=0;//z???
	m[12]=p.x;//x, or w/out transpose
	m[13]=p.y;//y,
	m[14]=p.z;//z
	m[15]=1;
}

//todo:make htis 
void get_indices(short *indicies ,int indicies_size){
	int i=0;
    for (i = 0; i < indicies_size/sizeof(short); i++){ 
		indicies[i]=(short) i;
    }
}

//note that the vector should be angular velocity vector
Quat updateQuatByRotation(Quat update_quat, Vec v, float dt)
{   
		Quat w;
		w.w=0;
		w.x=v.x * dt;
		w.y=v.y * dt;
		w.z=v.z * dt;

		Quat new_quat=quat_mul(w,update_quat);

		update_quat.w += new_quat.w * 0.5;
		update_quat.x += new_quat.x * 0.5;
		update_quat.y += new_quat.y * 0.5;
		update_quat.z += new_quat.z * 0.5;

		quat_norm(&update_quat);

		return update_quat;
}


//this is the function getting called a bunch
void render()
{
   int zoom =1;
   glFlush();//clear
   glClearColor(1.0,1.0,1.0,1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(g_resources.program);
   glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);

   getViewMatrix(&g_resources.Vmatrix); //start at -10z
   getModelMatrix(&g_resources.Mmatrix); 

   //y, axes rotation -- we rotate the model not the view
   float r_y_m[16];
   getRotateYMatrix(0,&r_y_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_y_m,sizeof(g_resources.Mmatrix),sizeof(r_y_m));

   float r_x_m[16];
   getRotateXMatrix(0,&r_x_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_x_m,sizeof(g_resources.Mmatrix),sizeof(r_x_m));



    glUniformMatrix4fv(g_resources.uniforms.Vmatrix, 1, GL_FALSE, &g_resources.Vmatrix); 

    getProjectionMatrix(&g_resources.Pmatrix, 40+(zoom*2), 1, .1, 10000); 
    glUniformMatrix4fv( g_resources.uniforms.Pmatrix, 1, GL_FALSE, &g_resources.Pmatrix);

	int s=0;
    for(s=0;s<solid_count;s++){


       glUniformMatrix4fv(g_resources.uniforms.Mmatrix, 1, GL_FALSE, &solids[s].model_matrix);

       //fprintf(stderr,"x is :%f\n",solids[s].model_matrix[12]);


	   g_resources.vertex_buffer = make_buffer(
			  GL_ARRAY_BUFFER,
			  solids[s].verticies,
			  solids[s].verticies_size
	   );
	  g_resources.element_buffer = make_buffer(
		   GL_ELEMENT_ARRAY_BUFFER,
		   solids[s].indicies,
		   solids[s].indicies_size
	  );

		//bind positions
		glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
		glVertexAttribPointer(g_resources.attributes.position, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
		glEnableVertexAttribArray(g_resources.attributes.position);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
		
		glDrawElements(
			GL_TRIANGLES,  /* mode */
			solids[s].verticies_size, /* count */
			GL_UNSIGNED_SHORT,  /* type */
			(void*)0            /* element array buffer offset */
		);

	}

    glDisableVertexAttribArray(g_resources.attributes.position);
    glDisableVertexAttribArray(g_resources.attributes.normal);
    glutSwapBuffers();
}


//This will onlly  with (x,4) * (4*4) matrix
void multiplyMatrices(float *verticies, float translation[],int verticies_size,int translation_size){

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
	r_matrix[13]=2;//y,
	r_matrix[14]=-20;//z
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

void getProjectionMatrix( float *r_matrix,float angleOfView,float imageAspectRatio, float zMin, float zMax)
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

/*
 * Functions for creating OpenGL objects:
 */
static GLuint make_buffer( GLenum target, const void *buffer_data, GLsizei buffer_size) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{

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
        //show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
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
        //show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}


float v_distance( Vec v1, Vec v2){
    return sqrt(
        ((v2.x-v1.x) * (v2.x-v1.x))+ 
        ((v2.y-v1.y) * (v2.y-v1.y))+
        ((v2.z-v1.z) * (v2.z-v1.z))
      );
}

void copy_matrix(float *a,float *b){
    a[0]=b[0]; 
    a[1]=b[1]; 
    a[2]=b[2]; 
    a[3]=b[3]; 
    a[4]=b[4]; 
    a[5]=b[5]; 
    a[6]=b[6]; 
    a[7]=b[7]; 
    a[8]=b[8]; 
    a[9]=b[9]; 
    a[10]=b[10]; 
    a[11]=b[11]; 
    a[12]=b[12]; 
    a[13]=b[13]; 
    a[14]=b[14]; 
    a[15]=b[15]; 
    a[16]=b[16]; 
}

void copy_matrix_add(float *a,float *b){
    a[0]+=b[0]; 
    a[1]+=b[1]; 
    a[2]+=b[2]; 
    a[3]+=b[3]; 
    a[4]+=b[4]; 
    a[5]+=b[5]; 
    a[6]+=b[6]; 
    a[7]+=b[7]; 
    a[8]+=b[8]; 
    a[9]+=b[9]; 
    a[10]+=b[10]; 
    a[11]+=b[11]; 
    a[12]+=b[12]; 
    a[13]+=b[13]; 
    a[14]+=b[14]; 
    a[15]+=b[15]; 
    a[16]+=b[16]; 
}



void Quadcopter_apply_to_all_thrust(Solid *s, float thrust){
    s->forces[0].force.y=thrust;
    s->forces[1].force.y=thrust;
    s->forces[2].force.y=thrust;
    s->forces[3].force.y=thrust;
}

void Quadcopter_add_to_all_thrust(Solid *s, float thrust){
    s->forces[0].force.y+=thrust;
    s->forces[1].force.y+=thrust;
    s->forces[2].force.y+=thrust;
    s->forces[3].force.y+=thrust;
}

void Quadcopter_go_to_height(Solid * s, float height){
//   //we want to stabilize at a certain height   
//   //if we are below height add more force,  if above add less force
    float thrust=s->forces[0].force.y;
    if(s->position.y < 1){
        thrust=9.9/4;
    } else {
        thrust=9.7/4;
    }
    Quadcopter_apply_to_all_thrust(s,thrust);
}


float going_up_start_time=0;
int done=0;

void Quadcopter_up_one_meter(Solid * s, float height){
    if(done==1){
        return ;
    }
    if(going_up_start_time==0){
        going_up_start_time=total_elapsed;
        Quadcopter_apply_to_all_thrust(s,(9.8*2)/4);
    }
      //fprintf(stderr, "TIME IS : %f\n",NS_TO_MS(total_elapsed-going_up_start_time));
/// //stop after a second
  if(3710 < NS_TO_MS(total_elapsed-going_up_start_time)){
      Quadcopter_apply_to_all_thrust(s,0);
  }

  if(6000 < NS_TO_MS(total_elapsed-going_up_start_time)){
      Quadcopter_apply_to_all_thrust(s,9.8/4);
      done=1;
  }

}


float ROLL_ERROR_TOTAL_I;
float PITCH_ERROR_TOTAL_I;
//float damping=.99999999999999999; 
void Quadcopter_stabilize_orientation(Solid * s){
    
   Quat q=solids[0].orientation;

   float yaw = atan2(2.0*(q.y*q.z + q.w*q.x), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
   float pitch = asin(-2.0*(q.x*q.z - q.w*q.y));
   float roll = atan2(2.0*(q.x*q.y + q.w*q.z), q.w*q.w + q.x*q.x - q.y*q.y - q.z*q.z);

   
    float thrust=1;

    float desired_roll=0;
    float T=desired_roll;
    float desired_pitch =0;
    float TT=desired_pitch;

    float P=4;
    float I=.5;

    float PP=4;
    float II=.5;

    //float roll_error= T - roll;
    float roll_error= T - solids[0].angular_velocity.z;
    float pitch_error= TT - solids[0].angular_velocity.x;

    ROLL_ERROR_TOTAL_I+=I*(roll_error)*(1/hz2);
    PITCH_ERROR_TOTAL_I+=II*(pitch_error)*(1/hz2);


 //fprintf(stderr, "x IS : %f\n",solids[0].angular_velocity.x);
 //fprintf(stderr, "y IS : %f\n",solids[0].angular_velocity.y);
 //fprintf(stderr, "roll IS : %f\n",roll);
    //work
    
    float right_thrust;
    float left_thrust;
    float back_thrust;
    float front_thrust;

 ///cube.s.forces[0]=fr;
 ///cube.s.forces[1]=fl;
 ///cube.s.forces[3]=br;
 ///cube.s.forces[4]=bl;

//  right_thrust=-((roll_error * P) + ROLL_ERROR_TOTAL_I);
//  left_thrust=(roll_error * P) + ROLL_ERROR_TOTAL_I;

  right_thrust=-((roll_error * P) + ROLL_ERROR_TOTAL_I);
  left_thrust= (roll_error * P) + ROLL_ERROR_TOTAL_I ;

  front_thrust=-((pitch_error * PP) + ROLL_ERROR_TOTAL_I);
  back_thrust= (pitch_error * PP) + ROLL_ERROR_TOTAL_I ;
 //front_thrust=0;
 //back_thrust=0;


 fprintf(stderr, "front thrust  IS : %f\n",front_thrust);
 fprintf(stderr, "back thrust  IS : %f\n",back_thrust);

 // damping=damping*damping;
 // right_thrust*=damping;
 // left_thrust*=damping;
    //
    s->forces[0].force.y=min_0(right_thrust + front_thrust);
    s->forces[1].force.y=min_0(left_thrust + front_thrust);
    s->forces[2].force.y=min_0(right_thrust + back_thrust);
    s->forces[3].force.y=min_0(left_thrust + back_thrust);

}

float min_0(float t){
    return t;//todo:remove this 
    if(t<0){
       return 0; 
    }
    return t;
}


float TOTAL_I;
void Quadcopter_stabilize_velocity(Solid * s, desired_velocity ){

    //fprintf(stderr, "POS  Y IS : %f\n",s->position.y);
    //fprintf(stderr, "VElocity  Y IS : %f\n",s->velocity.y);
    float T=desired_velocity;
    float P=4.9;
    float I=.5;
    float thrust;
    float velocity_error= T - s->velocity.y;

    TOTAL_I+=I*(velocity_error)*(1/hz2);
    //Proportional
    thrust=(velocity_error * P) + TOTAL_I;
    //fprintf(stderr, "THRUST  Y IS : %f\n",thrust);
    Quadcopter_add_to_all_thrust(s,thrust);
}

void apply_controls(){
  int i=0;
  for(i=0;i<controls_count;i++){
        if( NS_TO_MS(total_elapsed) < controls[i].start || 
            NS_TO_MS(total_elapsed) > controls[i].stop  
        ){ continue; }

         if(controls[i].first_run==1){
            //reset the integration error
             controls[i].first_run=0;
             TOTAL_I=0;
         }

        // Quadcopter_stabilize_orientation(&solids[0]);
         //be sure ot stabilize orientation first
        // Quadcopter_stabilize_velocity(&solids[0],controls[i].a_value);
  }
}



void sim(void){

  //Quadcopter_up_one_meter(&solids[0],1);
  float seconds=1/hz;
  int i;
  for(i=0;i<solid_count;i++){

      int t;

      Vec all_forces;
        all_forces.x=0;
        all_forces.y=0;
        all_forces.z=0;

      Vec all_torques;
        all_torques.x=0;
        all_torques.y=0;
        all_torques.z=0;


      //I think we should just sum forces

      for(t=0;t<solids[i].forces_count;t++){
        //only test if start is set
        if(solids[i].forces[t].start){
                if( NS_TO_MS(total_elapsed) < solids[i].forces[t].start  || 
                    NS_TO_MS(total_elapsed) > solids[i].forces[t].stop  
            ){
                continue;
            }
        }

            

        all_forces=v_add(all_forces,solids[i].forces[t].force);
        Vec pos;
        pos.x=0;
        pos.y=0;
        pos.z=0;

		//point of force = point - position(where position is center of mass)
        Vec pf=v_sub(solids[i].forces[t].point,pos); //todo: get center point
		//(1,0,0) x (0,1,0) = (0,0,1)
		//ie: torque now represents axis of rotation and (i think) magnitude of rotation
		//we are assuming 90% force
        Vec new_torque=v_mult(solids[i].forces[t].force,pf);
        all_torques=v_add(all_torques,new_torque);
      }


        Vec accel= v_divide_s(all_forces, solids[i].mass);
        Vec velocity_new=v_mult_s(accel, seconds);
        solids[i].velocity=v_add(solids[i].velocity,velocity_new);

        Vec pos_dela=v_mult_s(solids[i].velocity,seconds);
        solids[i].position=v_add(solids[i].position,pos_dela);

        float pos_matrix[16];
        get_position_matrix(pos_matrix, solids[i].position);
        
        //rotational acceleration
        //float torque = f * pf;//(point at which force is applied, relative to center of mass)
		//A=F/m  Accel angular = Torque / intertia tensor
		Vec angular_acelleration = all_torques;//divided by inertia tensor 
		solids[i].angular_velocity = v_add(solids[i].angular_velocity,v_mult_s(angular_acelleration,seconds));
        solids[i].orientation=updateQuatByRotation(solids[i].orientation, solids[i].angular_velocity,seconds);

        float r_matrix[16];
        quat_to_matrix(solids[i].orientation,&r_matrix);
        mult_matrix(&pos_matrix,r_matrix);
        copy_matrix(&solids[i].model_matrix,&pos_matrix);

      //update velocity
      //update position
  }
}

void mult_matrix(float *a,float *b){
    float m[16];
    int i=0;
    for (i = 0; i < 4; i++) { 
        float x=a[(i*4)+0];
        float y=a[(i*4)+1];
        float z=a[(i*4)+2];
        float w=a[(i*4)+3];

        m[(i*4)+0]=(x*b[0])+(y*b[1])+(z*b[2]) + (w*b[3]);
        m[(i*4)+1]=(x*b[4])+(y*b[5])+(z*b[6]) + (w*b[7]) ;
        m[(i*4)+2]=(x*b[8])+(y*b[9])+(z*b[10]) + (w*b[11]);
        m[(i*4)+3]=(x*b[12])+(y*b[13])+(z*b[14]) + (w*b[15]);
    }

    a[0]=m[0]; 
    a[1]=m[1]; 
    a[2]=m[2]; 
    a[3]=m[3]; 
    a[4]=m[4]; 
    a[5]=m[5]; 
    a[6]=m[6]; 
    a[7]=m[7]; 
    a[8]=m[8]; 
    a[9]=m[9]; 
    a[10]=m[10]; 
    a[11]=m[11]; 
    a[12]=m[12]; 
    a[13]=m[13]; 
    a[14]=m[14]; 
    a[15]=m[15]; 
    a[16]=m[16]; 

    //copy_matrix(&a,&m);
}

void quat_to_matrix(Quat q,float *r_matrix){

    r_matrix[0]= 1 - ((2*(q.y*q.y))+ (2*(q.z*q.z)));
    r_matrix[1]= (2*q.x*q.y) + (2*q.z*q.w);
    r_matrix[2]= (2*q.x*q.z) - (2*q.y*q.w);
    r_matrix[3]= 0;

    r_matrix[4]= (2*q.x*q.y) - (2*q.z*q.w);
    r_matrix[5]= 1- ((2*(q.x*q.x)) + (2*(q.z*q.z)));
    r_matrix[6]= (2*q.y*q.z) + (2*q.x*q.w);
    r_matrix[7]= 0;

    r_matrix[8]= (2*q.x*q.z) + (2*q.y*q.w);
    r_matrix[9]= (2*q.y*q.z) - (2*q.x*q.w);
    r_matrix[10]= 1- ((2*(q.x*q.x)) + (2*(q.y*q.y)));
    r_matrix[11]= 0;

    r_matrix[12]= 0;
    r_matrix[13]= 0;
    r_matrix[14]= 0;
    r_matrix[15]= 1;
}

void quat_norm(Quat * q){

 float d= (q->w*q->w) + (q->x*q->x) + (q->y*q->y) + (q->z*q->z);

 if(d==0){
     q->w = 1;
     q->x = 0;
     q->y = 0;
     q->z = 0;
     return ;
 }

 float m  = sqrt(d);
 q->w = q->w / m;
 q->x = q->x / m;
 q->y = q->y / m;
 q->z = q->z / m ; 
}

Quat quat_mul(Quat q1,Quat q2) {

	Quat q;
//      q.x =  q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
//      q.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
//      q.z =  q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
//      q.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
    q.w = (q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
    q.x = (q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y);
    q.y = (q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x);
    q.z = (q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w);


    //quat_norm(&q);

	return q;
}

Quat quat_add(Quat q1,Quat q2){
	Quat c;
    c.x = q1.x + q2.x;
    c.y = q1.y + q2.y;
    c.z = q1.z + q2.z;
    c.w = q1.w + q2.w;
	return c;
}


Vec v_norm( Vec v){
    float l=sqrt(
        (v.x*v.x) +
        (v.y*v.y) +
        (v.z*v.z)
    );
	 Vec nv;
	nv.x=v.x/l;
	nv.y=v.y/l;
	nv.z=v.z/l;
	return nv;
}

//dot product
float v_dot( Vec v1, Vec v2){                                                                                  
   Vec v1n=v_norm(v1); 
   Vec v2n=v_norm(v2); 
  return v1n.x * v2n.x + v1n.y * v2n.y + v1n.z * v2n.z; 
}

float quat_dot(Quat a, Quat b){
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w*b.w; 
}


//cross product or vector product
Vec v_mult( Vec a, Vec b){
        Vec v;
        v.x=(a.y*b.z) - (a.z*b.y);
        v.y= (a.z*b.x) - (a.x*b.z);
        v.z= (a.x*b.y) -(a.y*b.x);
        return v;
}
 //tay you pry don't want to use this
 Vec v_mult_compenent_product(Vec a, Vec b ){
      Vec c;
      c.x=a.x*b.x;
      c.y=a.y*b.y;
      c.z=a.z*b.z;
      return c;
 } 

 Vec v_mult_s( Vec v,float m){
     Vec vf;
    vf.x=v.x*m;
    vf.y=v.y*m;
    vf.z=v.z*m;
    return vf;
}
//vector multtiply by scalar
 Vec v_divide_s( Vec v,float m){
    Vec vf;
    vf.x=v.x/m;
    vf.y=v.y/m;
    vf.z=v.z/m;
    return vf;
}

Vec v_add(Vec a, Vec b){
         Vec v;
        v.x=a.x+b.x;
        v.y=a.y+b.y;
        v.z=a.z+b.z;
        return v;
}

Vec v_sub( Vec a, Vec b){
         Vec v;
        v.x=a.x-b.x;
        v.y=a.y-b.y;
        v.z=a.z-b.z;
        return v;
}

float v_mag( Vec v){
   return sqrt(
        (v.x*v.x) +
        (v.y*v.y) +
        (v.z*v.z)
    );
}


void simulate_timer(void){
	last = now;
	usleep(100); // Sleep for 1/1000 second
	clock_gettime(CLOCK_MONOTONIC, &now);

	if(!last.tv_nsec){
        return;
    }

	elapsed += (1000000000 * (double)(now.tv_sec  - last.tv_sec)) +
			   (double)(now.tv_nsec - last.tv_nsec);

	elapsed2 += (1000000000 * (double)(now.tv_sec  - last.tv_sec)) +
			   (double)(now.tv_nsec - last.tv_nsec);



    //this loop is for real life
	while (elapsed2 >= MS_TO_NS(1000/hz2)) {
        elapsed2 -= MS_TO_NS(1000/hz2);
        apply_controls();
        //Quadcopter_stabilize_velocity(&solids[0],0);
	}

    //this loop is for real life
	while (elapsed >= MS_TO_NS(1000/hz)) {
        total_elapsed+=elapsed;
        elapsed -= MS_TO_NS(1000/hz);
        sim();//pretent this is real live
		glutPostRedisplay();
		//render new glut
		 //render();
	}
}



int main(int argc, char** argv){
	 clock_gettime(CLOCK_MONOTONIC, &now);

     glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
     glutInitWindowSize(1000, 1000);
     glutCreateWindow("Hello World");
     glutIdleFunc(&simulate_timer);

     
     glutDisplayFunc(&render);
     //used to detect stuff
     //glutMouseFunc(mouse);
     //glutMotionFunc(motion);
     //glutKeyboardFunc(keyPressed);
     glewInit();


    quadcopter=new_quadcopter();

    Control go_up_control;
    go_up_control.start=0;
    go_up_control.stop=20000;
    go_up_control.a_value=0;
    go_up_control.first_run=1;


    Control c2;//this is just hack, I have messed someting up w/ memory and it's not '
    c2.start=20000;
    c2.stop=35000;
    c2.a_value=2;
    c2.first_run=1;

    Control c3;//this is just hack, I have messed someting up w/ memory and it's not '
    c3.start=35000;
    c3.stop=1244000;
    c3.a_value=0;
    c3.first_run=1;

    Control c4;//this is just hack, I have messed someting up w/ memory and it's not n'


    controls[0]=go_up_control;
    controls[1]=c2;
    controls[2]=c3;
    controls_count=3;


    //Quadcopter_hover(&quadcopter);
    solids[0]=quadcopter.cube.s;
    solid_count++;
  

    //controls_count++;



     if (!GLEW_VERSION_2_0) {
         fprintf(stderr, "OpenGL 2.0 not available\n");
         return -1;
     }

     if (!make_resources()) {
         fprintf(stderr, "Failed to load resources\n");
         return -1;
     }

	 //clock_gettime(CLOCK_MONOTONIC, &last);
     glutMainLoop();

    return 0;
}


