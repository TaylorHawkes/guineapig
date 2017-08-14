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

void getViewMatrix(float *r_matrix){
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
	r_matrix[14]=-10;//z
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

void getProjectionMatrixThree(float *r_matrix,float fov,float aspect, float near,float far)
{
    float D2R = M_PI / 180.0;
    float yScale = 1.0 / tan(D2R * fov / 2);
    float xScale = yScale / aspect;
    float nearmfar = near - far;
    
	float m[] = {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, (far + near) / nearmfar, -1,
        0, 0, 2*far*near / nearmfar, 0 
    };    
    memcpy(r_matrix, m, sizeof(float)*16);
}


void getProjectionMatrixTwo( float *r_matrix,float angleOfView,float imageAspectRatio, float zMin,float zMax)
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

void getProjectionMatrix( float *r_matrix,float angleOfView,float imageAspectRatio, float n,float f){

    float scale = tan(angleOfView * 0.5 * M_PI / 180) * n; 
    float r = imageAspectRatio * scale;
    float l = -r; 
    float t = scale;
    float b = -t; 

    r_matrix[0] = 2 * n / (r - l); 
    r_matrix[1] = 0; 
    r_matrix[2] = 0; 
    r_matrix[3] = 0; 

    r_matrix[4] = 0; 
    r_matrix[5] = 2 * n / (t - b); 
    r_matrix[6] = 0; 
    r_matrix[7] = 0; 

    r_matrix[8] = (r + l) / (r - l); 
    r_matrix[9] = (t + b) / (t - b); 
    r_matrix[10] = -(f + n) / (f - n); 
    r_matrix[11] = -1; 

    r_matrix[12] = 0; 
    r_matrix[13] = 0; 
    r_matrix[14] = -2 * f * n / (f - n); 
    r_matrix[15] = 0; 
}


void getNormals(float *normals,float verticies[],int verticies_size){

////-1.0,-1.0,-1.0,1, x
////-1.0,-1.0, 1.0,1, y
////-1.0, 1.0, 1.0, 1, z

	int verticies_length=(verticies_size/sizeof(float)/4);//rows of m1;

//	float normals[verticies_length*(3/4)];
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

float g_normals_buffer_data[] = { 
	-1.0,-1.0,-1.0,
	-1.0,-1.0, 1.0,
	-1.0, 1.0, 1.0,
	1.0, 1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0,-1.0,
	1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	1.0,-1.0,-1.0,
	1.0, 1.0,-1.0,
	1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0, 1.0,
	-1.0, 1.0,-1.0,
	1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0, 1.0,
	-1.0,-1.0, 1.0,
	1.0,-1.0, 1.0,
	1.0, 1.0, 1.0,
	1.0,-1.0,-1.0,
	1.0, 1.0,-1.0,
	1.0,-1.0,-1.0,
	1.0, 1.0, 1.0,
	1.0,-1.0, 1.0,
	1.0, 1.0, 1.0,
	1.0, 1.0,-1.0,
	-1.0, 1.0,-1.0,
	1.0, 1.0, 1.0,
	-1.0, 1.0,-1.0,
	-1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	1.0,-1.0, 1.0
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

	//just initialte with verticies
	getNormals(&g_normals_buffer_data,g_vertex_buffer_data,sizeof(g_vertex_buffer_data));


	
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[0]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[1]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[2]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[3]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[5]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[6]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[7]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[8]);
	fprintf(stderr," x old  is at:%f\n",g_normals_buffer_data[9]);

	g_resources.normals_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        g_normals_buffer_data,
        sizeof(g_normals_buffer_data)
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

    g_resources.uniforms.fade_factor = glGetUniformLocation(g_resources.program, "fade_factor");
    g_resources.uniforms.Vmatrix = glGetUniformLocation(g_resources.program, "Vmatrix");
	g_resources.uniforms.Pmatrix = glGetUniformLocation(g_resources.program, "Pmatrix");
	g_resources.uniforms.Mmatrix = glGetUniformLocation(g_resources.program, "Mmatrix");
    g_resources.uniforms.textures[0] = glGetUniformLocation(g_resources.program, "textures[0]");
    g_resources.uniforms.textures[1] = glGetUniformLocation(g_resources.program, "textures[1]");
    g_resources.attributes.position = glGetAttribLocation(g_resources.program, "position");
    g_resources.attributes.normal = glGetAttribLocation(g_resources.program, "normal");


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

    getViewMatrix(&g_resources.Vmatrix); //start at -10z



   getModelMatrix(&g_resources.Mmatrix); 

   //y, axes rotation -- we rotate the model not the view
   float r_y_m[16];
   getRotateYMatrix(rotate_y,&r_y_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_y_m,sizeof(g_resources.Mmatrix),sizeof(r_y_m));

   float r_x_m[16];
   getRotateXMatrix(rotate_x,&r_x_m); 
   multiplyMatrices(&g_resources.Mmatrix,r_x_m,sizeof(g_resources.Mmatrix),sizeof(r_x_m));



////getRotateXMatrix(45,&r_matrix_x);

	//projection matrix

	 glUniformMatrix4fv(
			  g_resources.uniforms.Mmatrix,
			  1,
			  GL_FALSE,
			  &g_resources.Mmatrix
	  );

    //set Vmatrix
    glUniformMatrix4fv(
            g_resources.uniforms.Vmatrix,
            1,
            GL_FALSE,
            &g_resources.Vmatrix
    );

   getProjectionMatrixTwo(&g_resources.Pmatrix, 40+(zoom*2), 1, .1, 3000); 

   glUniformMatrix4fv(
            g_resources.uniforms.Pmatrix,
            1,
            GL_FALSE,
            &g_resources.Pmatrix
    );

  


    
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


	//refresh the normals the light is stagnent elsewhere

	//bind the normals buffer
    glBindBuffer(GL_ARRAY_BUFFER, g_resources.normals_buffer);
    glVertexAttribPointer(
        g_resources.attributes.normal,  /* attribute */
        3,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                        /* normalized? */
        sizeof(float)*3,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    glEnableVertexAttribArray(g_resources.attributes.normal);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);

    glDrawElements(
        GL_TRIANGLES,  /* mode */
        //GL_LINES,  /* mode */
        sizeof(g_vertex_buffer_data)/sizeof(float), /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

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
	glutKeyboardFunc(keyPressed);



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

