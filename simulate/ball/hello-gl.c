#include <stdlib.h>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <math.h>
#include <stdio.h>
#include "util.h"

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


static struct {
    GLuint vertex_buffer, element_buffer;
    GLuint textures[2];
    GLuint vertex_shader, fragment_shader, program;
    
    struct {
        GLint fade_factor;
        GLint textures[2];
        GLint Vmatrix;
    } uniforms;

    struct {
        GLint position;
    } attributes;

    float fade_factor;
    GLfloat Vmatrix[16];

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

static void show_info_log(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
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


//todo:make htis work
void getIndices(float shape,float *indicies){
	int i=0;
    for (i = 0; i <= sizeof(shape)/sizeof(float); i++){                                               
		indicies[i]=i;
    }
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



//This will onlly work with (x,3) * (3*3) matrix
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


////float identity_m[] = { 
////	 1,0,0,//x = 1*x + 0*y + 0*z 
////	 0,1,0,//y = 0*x + 1*1 + 0*z 
////	 0,0,1 //z = 
////};


float identity_m[] = { 
	 1,0,0,0,//x = 1*x + 0*y + 0*z 
	 0,1,0,0,//y = 0*x + 1*1 + 0*z 
	 0,0,1,0,//z = 
	 0,0,0,1//z = 
};

/*
 * Data used to seed our vertex array and element array buffers:
 */


 float g_vertex_buffer_data[] = { 
-0.5,-0.5,-0.5,1,
-0.5,-0.5, 0.5,1,
-0.5, 0.5, 0.5, 1,
0.5, 0.5,-0.5, 1,
-0.5,-0.5,-0.5,1,
-0.5, 0.5,-0.5, 1,
0.5,-0.5, 0.5,1,
-0.5,-0.5,-0.5,1,
0.5,-0.5,-0.5,1,
0.5, 0.5,-0.5,1,
0.5,-0.5,-0.5,1,
-0.5,-0.5,-0.5,1,
-0.5,-0.5,-0.5,1,
-0.5, 0.5, 0.5,1,
-0.5, 0.5,-0.5,1,
0.5,-0.5, 0.5,1,
-0.5,-0.5, 0.5,1,
-0.5,-0.5,-0.5,1,
-0.5, 0.5, 0.5,1,
-0.5,-0.5, 0.5,1,
0.5,-0.5, 0.5,1,
0.5, 0.5, 0.5,1,
0.5,-0.5,-0.5,1,
0.5, 0.5,-0.5,1,
0.5,-0.5,-0.5,1,
0.5, 0.5, 0.5,1,
0.5,-0.5, 0.5,1,
0.5, 0.5, 0.5,1,
0.5, 0.5,-0.5,1,
-0.5, 0.5,-0.5,1,
0.5, 0.5, 0.5,1,
-0.5, 0.5,-0.5,1,
-0.5, 0.5, 0.5,1,
0.5, 0.5, 0.5,1,
-0.5, 0.5, 0.5,1,
0.5,-0.5, 0.5,1
};

/// static const GLushort g_element_buffer_data[] = {
///   0,  1,  2,      0,  2,  3,    // front
///   4,  5,  6,      4,  6,  7,    // back
///   8,  9,  10,     8,  10, 11,   // top
///   12, 13, 14,     12, 14, 15,   // bottom
///   16, 17, 18,     16, 18, 19,   // right
///   20, 21, 22,     20, 22, 23    // left
/// };
static const GLushort g_element_buffer_data[] = {
  0,  1,  2,      3,  4,  5,    // front
  6,  7,  8,      9,  10, 11,    // back
  12, 13, 14,     15, 16, 17,   // top
  18, 19, 20,     21, 22, 23,   // bottom
  24, 25, 26,     27, 28, 29,   // right
  30, 31, 32,     33, 34, 35    // left
};

/*
 * Load and create all of our resources:
 */
static int make_resources(void)
{
    g_resources.vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        g_vertex_buffer_data,
        sizeof(g_vertex_buffer_data)
    );
    g_resources.element_buffer = make_buffer(
        GL_ELEMENT_ARRAY_BUFFER,
        g_element_buffer_data,
        sizeof(g_element_buffer_data)
    );

    g_resources.textures[0] = make_texture("hello1.tga");
    g_resources.textures[1] = make_texture("hello2.tga");

    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
        return 0;

    g_resources.vertex_shader = make_shader(
        GL_VERTEX_SHADER,
        "hello-gl.v.glsl"
    );
    if (g_resources.vertex_shader == 0)
        return 0;

    g_resources.fragment_shader = make_shader(
        GL_FRAGMENT_SHADER,
        "hello-gl.f.glsl"
    );
    if (g_resources.fragment_shader == 0)
        return 0;

    g_resources.program = make_program(g_resources.vertex_shader, g_resources.fragment_shader);
    if (g_resources.program == 0)
        return 0;

    g_resources.uniforms.fade_factor
        = glGetUniformLocation(g_resources.program, "fade_factor");
    g_resources.uniforms.Vmatrix
        = glGetUniformLocation(g_resources.program, "Vmatrix");
    g_resources.uniforms.textures[0]
        = glGetUniformLocation(g_resources.program, "textures[0]");
    g_resources.uniforms.textures[1]
        = glGetUniformLocation(g_resources.program, "textures[1]");
    g_resources.attributes.position
        = glGetAttribLocation(g_resources.program, "position");

    return 1;
}

/*
 * GLUT callbacks:
 */
static void update_fade_factor(void)
{
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    g_resources.fade_factor = sinf((float)milliseconds * 0.001f) * 0.5f + 0.5f;
    glutPostRedisplay();
}

//this is the function getting called a bunch
static void render(void)
{
    
	glFlush();//clear
	glClearColor(1.0,1.0,1.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

    glUseProgram(g_resources.program);
    glUniform1f(g_resources.uniforms.fade_factor, g_resources.fade_factor);
    getRotateXMatrix(-rotate_x,&g_resources.Vmatrix); 

   float temp_rotate_matrix[16];
   getRotateYMatrix(rotate_y,&temp_rotate_matrix); 

   multiplyMatrices(&g_resources.Vmatrix,temp_rotate_matrix,sizeof(g_resources.Vmatrix),sizeof(temp_rotate_matrix));

////getRotateXMatrix(45,&r_matrix_x);



    //set Vmatrix
    glUniformMatrix4fv(
            g_resources.uniforms.Vmatrix,
            1,
            GL_FALSE,
            &g_resources.Vmatrix
    );
    //uniformMatrix4fv
    //uniformMatrix4fv
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[0]);
    glUniform1i(g_resources.uniforms.textures[0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_resources.textures[1]);
    glUniform1i(g_resources.uniforms.textures[1], 1);




    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        4,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(float)*4,                /* stride */
        (void*)0                          /* array buffer offset */
    );


    glEnableVertexAttribArray(g_resources.attributes.position);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);

    glDrawElements(
        GL_TRIANGLES,  /* mode */
        //GL_LINES,  /* mode */
        sizeof(g_vertex_buffer_data)/sizeof(float), /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(g_resources.attributes.position);
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


/*
 * Entry point
 */
int main(int argc, char** argv)
{

	//move(g_vertex_buffer_data,sizeof(g_vertex_buffer_data));
//        multiplyMatrices(&g_vertex_buffer_data,identity_m,sizeof(g_vertex_buffer_data),sizeof(identity_m));
    


//fprintf(stderr, "%f,",g_vertex_buffer_data[0]); fprintf(stderr, "%f,",g_vertex_buffer_data[1]); fprintf(stderr, "%f\n",g_vertex_buffer_data[2]); fprintf(stderr, "%f\n",g_vertex_buffer_data[4]);



//  float r_matrix_x[9];
//  getRotateYMatrix(40,&r_matrix_x);
//  multiplyMatrices(&g_vertex_buffer_data,r_matrix_x,sizeof(g_vertex_buffer_data),sizeof(r_matrix_x));


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Hello World");
    //glutIdleFunc(&update_fade_factor);
    glutDisplayFunc(&render);

    //used to detect stuff
    glutMouseFunc(mouse);
    glutMotionFunc(motion);


    glewInit();
    if (!GLEW_VERSION_2_0) {
        fprintf(stderr, "OpenGL 2.0 not available\n");
        return 1;
    }

    if (!make_resources()) {
        fprintf(stderr, "Failed to load resources\n");
        return 1;
    }

    glutMainLoop();
    return 0;
}

