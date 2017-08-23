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
#include "sim.h"
#include "ball/util.h"
#define MS_TO_NS(x) (1000000*(x))


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


typedef struct {
    Vec point;//point on object force is applied
    Vec force;//direction and magnitude of force neutons
} Force;


typedef struct {
  Vec center_of_mass;
  Vec ineritia;
  Vec velocity;
 float mass;

 Force forces[10];
 int forces_count;

 float *verticies;
 float *normals;
 float *indicies;
 int verticies_size; 
 int indicies_size;
 int normals_size;

 GLfloat model_matrix[16];

} Solid ;

typedef struct {
  Solid s;
} Cube;

//global variables
Solid solids[2];
int solid_count=0;
struct timespec last, now;
double elapsed = 0;
double hz=100;//times per second

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

//todo:make htis work
void get_indices(short *indicies ,int indicies_size){
	int i=0;
    for (i = 0; i < indicies_size/sizeof(short); i++){ 
		indicies[i]=(short) i;
    }
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

       fprintf(stderr,"x is :%f\n",solids[s].model_matrix[12]);


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


//This will onlly work with (x,4) * (4*4) matrix
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
	r_matrix[13]=0;//y,
	r_matrix[14]=-5;//z
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

void sim(void){

  float seconds=1/hz;
 //    fprintf(stderr, "Sim\n");
  int i;
  for(i=0;i<solid_count;i++){
      int t;
      for(t=0;t<solids[i].forces_count;t++){

        //velocity = force/mass * time 
    Vec accel= v_divide_s(solids[i].forces[t].force, solids[i].mass );
     Vec velocity_new=v_mult_s(accel, seconds);
     solids[i].velocity=v_add(velocity_new,solids[i].velocity);
      
      //then update the model matrix

  solids[i].model_matrix[12]+= (GLfloat) seconds * solids[i].velocity.x;
  solids[i].model_matrix[13]+= (GLfloat) seconds * solids[i].velocity.y;
  solids[i].model_matrix[14]+= (GLfloat) seconds * solids[i].velocity.z;
      
      }

      //update velocity
      //update position
  }
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


void simulate_timer(void){
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

     Cube drone;
     new_cube(&drone);
     drone.s.mass=1;

     Force fr_force;//apply force to front right
     fr_force.point.x=-1;
     fr_force.point.y=-1;
     fr_force.point.z=1;

     fr_force.force.x=0;
     fr_force.force.y=1;//neuton
     fr_force.force.z=0;

     drone.s.forces[0]=fr_force;
     drone.s.forces_count=1;


     solids[0]=drone.s;
     solid_count++;
  

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


