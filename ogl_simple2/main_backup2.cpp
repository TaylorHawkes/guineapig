#include "v4l2_driver.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <chrono>
#include <unistd.h>
#include "../MPU6050/I2Cdev.h"
#include "../MPU6050/MPU6050.h"
#include <thread>
// Include standard headers
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string.h>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>



typedef struct {
    float x,y,z;
} Vec3;


typedef struct {
    float m[16];
} Mat4;

typedef struct {                                                                              
    float x=0;
	float y=0;
	float z=0;
	float w=1;
} Quat;

void updateQuatByRotation(Quat *q, Vec3 *v);
void quat_mul(Quat *q1,Quat *q2,Quat *r);
void quatToMatrix(Quat q_bad,Mat4 *m);
void toEulerAngle(Quat& q, double &roll, double &pitch, double &yaw);
void toQuaternion(Quat& q, double roll, double pitch,double yaw);
void invert(Mat4 *m);

float v_dot( Vec3 v1, Vec3 v2);               
Vec3 v_cross( Vec3 a, Vec3 b);
Vec3 v_mult_s( Vec3 v,float m);
Vec3 rotate_vector_by_quaternion(Vec3 v, Quat q);


void updateQuatByRotation(Quat *q, Vec3 *v){

        Quat w,r;
        w.w=0;                                                                                
        w.x=v->x;                                                                         
        w.y=v->y;                                                                         
        w.z=v->z;                                                                        
		
        quat_mul(q,&w,&r);
        q->w += r.w * 0.5;                                                    
        q->x += r.x * 0.5;                                                    
        q->y += r.y * 0.5;                                                    
        q->z += r.z * 0.5;                                                    
}


void quat_mul(Quat *q1,Quat *q2,Quat *r){
    r->w = (q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z);
    r->x = (q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y);
    r->y = (q1->w*q2->y - q1->x*q2->z + q1->y*q2->w + q1->z*q2->x);
    r->z = (q1->w*q2->z + q1->x*q2->y - q1->y*q2->x + q1->z*q2->w);
}


//I still dont understand this...
void getProjectionMatrix( Mat4 *m,float angleOfView,float imageAspectRatio, float zMin, float zMax)
{
	float ang = tan((angleOfView*.5)*M_PI/180);//angle*.5                                                    
    m->m[0] = 0.5/ang; 
    m->m[1] = 0; 
    m->m[2] = 0; 
    m->m[3] = 0; 

    m->m[4] = 0; 
    m->m[5] = 0.5*imageAspectRatio/ang;
    m->m[6] = 0; 
    m->m[7] = 0; 

    m->m[8] = 0; 
    m->m[9] = 0;
    m->m[10] = -(zMax+zMin)/(zMax-zMin); 
    m->m[11] = -1; 

    m->m[12] = 0; 
    m->m[13] = 0; 
    m->m[14] = (-2*zMax*zMin)/(zMax-zMin);
    m->m[15] = 0;  
}

//This is where the camera is... Its like opposite of Model I think... ie we remove the world around the camera

//if we invert this then it is where we are
void getViewMatrix(Mat4 *m){
	m->m[0]=1;
	m->m[1]=0;
    m->m[2]=0;
    m->m[3]=0;

	m->m[4]=0;
	m->m[5]=1;
	m->m[6]=0;
	m->m[7]=0; 

	m->m[8]=0;
    m->m[9]=0;
    m->m[10]=1;
    m->m[11]=0; 
	
	//This is like camera position mmmk
	m->m[12]=0; //x
    m->m[13]=5;//y
    m->m[14]=20;//z
    m->m[15]=1;
	
   //and we flip
   m->m[12]*=-1;
   m->m[13]*=-1;
   m->m[14]*=-1;

}




void getIdentityMatrix(Mat4 *r){
    r->m[0] =1;
    r->m[1] =0;
    r->m[2] =0;
    r->m[3] =0;

    r->m[4] =0;
    r->m[5] =1;
    r->m[6] =0;
    r->m[7] =0;

    r->m[8] =0;
    r->m[9] =0;
    r->m[10] =1;
    r->m[11] =0;

    r->m[12] =0;
    r->m[13] =0;
    r->m[14] =0;
    r->m[15] =1;
}

void matrix_product(Mat4 *a,Mat4 *b, Mat4 *r){
    r->m[0]  = (a->m[0]*b->m[0]) + (a->m[1]*b->m[4]) +(a->m[2]*b->m[8]) + (+a->m[3]*b->m[12]);
    r->m[1]  = (a->m[0]*b->m[1]) + (a->m[1]*b->m[5]) +(a->m[2]*b->m[9]) +(+a->m[3]*b->m[13]);
    r->m[2]  = (a->m[0]*b->m[2]) + (a->m[1]*b->m[6]) +(a->m[2]*b->m[10])+ (+a->m[3]*b->m[14]);
    r->m[3]  = (a->m[0]*b->m[3]) + (a->m[1]*b->m[7]) +(a->m[2]*b->m[11])+ (+a->m[3]*b->m[15]);

    r->m[4]  = (a->m[4]*b->m[0]) + (a->m[5]*b->m[4]) +(a->m[6]*b->m[8])+ (+a->m[7]*b->m[12]);
    r->m[5]  = (a->m[4]*b->m[1]) + (a->m[5]*b->m[5]) +(a->m[6]*b->m[9])+ (+a->m[7]*b->m[13]);
    r->m[6]  = (a->m[4]*b->m[2]) + (a->m[5]*b->m[6]) +(a->m[6]*b->m[10])+ (+a->m[7]*b->m[14]);
    r->m[7]  = (a->m[4]*b->m[3]) + (a->m[5]*b->m[7]) +(a->m[6]*b->m[11])+(+a->m[7]*b->m[15]);

    r->m[8]  = (a->m[8]*b->m[0]) + (a->m[9]*b->m[4]) +(a->m[10]*b->m[8])+ (+a->m[11]*b->m[12]);
    r->m[9]  = (a->m[8]*b->m[1]) + (a->m[9]*b->m[5]) +(a->m[10]*b->m[9])+ (+a->m[11]*b->m[13]);
    r->m[10] = (a->m[8]*b->m[2]) + (a->m[9]*b->m[6]) +(a->m[10]*b->m[10])+ (+a->m[11]*b->m[14]);
    r->m[11] = (a->m[8]*b->m[3]) + (a->m[9]*b->m[7]) +(a->m[10]*b->m[11])+(+a->m[11]*b->m[15]);

    r->m[12] = (a->m[12]*b->m[0]) + (a->m[13]*b->m[4]) +(a->m[14]*b->m[8]) + (+a->m[15]*b->m[12]);
    r->m[13] = (a->m[12]*b->m[1]) + (a->m[13]*b->m[5]) +(a->m[14]*b->m[9]) +(+a->m[15]*b->m[13]);
    r->m[14] = (a->m[12]*b->m[2]) + (a->m[13]*b->m[6]) +(a->m[14]*b->m[10])+ (+a->m[15]*b->m[14]);
    r->m[15] = (a->m[12]*b->m[3]) + (a->m[13]*b->m[7]) +(a->m[14]*b->m[11])+ (+a->m[15]*b->m[15]);
}

void print_matrix(Mat4 *a){
		fprintf(stderr, "%f %f %f %f \n",a->m[0],a->m[1],a->m[2],a->m[3]);
		fprintf(stderr, "%f %f %f %f \n",a->m[4],a->m[5],a->m[6],a->m[7]);
		fprintf(stderr, "%f %f %f %f \n",a->m[8],a->m[9],a->m[10],a->m[11]);
		fprintf(stderr, "%f %f %f %f \n",a->m[12],a->m[13],a->m[14],a->m[15]);
}

void print_quat(Quat *q){
		fprintf(stderr, "%f %f %f %f \n",q->w,q->x,q->y,q->z);
}


//HHEADER declrations
void computeMatricesFromInputs();
void updateGyroPositionThead();
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);


GLFWwindow* window;

////glm::quat currentOrientation;
Mat4 View;
Mat4 Model;
Mat4 Rotation;
Mat4 Projection;
Mat4 MVP;
Vec3 gyro;
Quat ooo;
Quat a_ooo;

printf(stderr, " pos: %f \n",final_image_pos[x+0]);
		//          g_vertex_buffer_data[x]  = (GLfloat)  final_image_pos[x+1]/100;
		//          g_vertex_buffer_data[x+1] = 4 - (GLfloat)  final_image_pos[x+0]/100;
		//          g_vertex_buffer_data[x+2] = - (GLfloat) final_image_pos[x+2] /100;//this is depth
		//  }


    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);
	
    //computeMatricesFromInputs();
	//Update Model by Rotation
	quatToMatrix(ooo,&Rotation);

////Mat4 t;
////getIdentityMatrix(&t);      
//matrix_product(&Rotation,&t,&Model);

	//print_matrix(&Rotation);

    matrix_product(&Rotation,&View,&MVP);
    matrix_product(&MVP,&Projection,&MVP);

 // Send our transformation to the currently bound shader, 
 // in the "MVP" uniform

 glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP.m);
 // 1rst attribute buffer : vertices
 glEnableVertexAttribArray(0);
 glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
 glVertexAttribPointer(
     0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
     3,                  // size
     GL_FLOAT,           // type
     GL_FALSE,           // normalized?
     0,                  // stride
     (void*)0            // array buffer offset
 );

 // Draw the triangle !
 glDrawArrays(GL_TRIANGLES, 0, sizeof(g_vertex_buffer_data)/4); // 3 indices starting at 0 -> 1 triangle
 //glDrawArrays(GL_POINTS, 0, sizeof(g_vertex_buffer_data)/4); // 3 indices starting at 0 -> 1 triangle

 glDisableVertexAttribArray(0);

 // Swap buffers
 glfwSwapBuffers(window);
 glfwPollEvents();

} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

// Cleanup VBO and shader
glDeleteBuffers(1, &vertexbuffer);
glDeleteProgram(programID);
glDeleteVertexArrays(1, &VertexArrayID);

// Close OpenGL window and terminate GLFW
glfwTerminate();

//std::terminate();//I think this cill threads

return 0;
}

void updateGyroPositionThead(){

 fprintf(stderr, " I am my own thread \n");
 accelgyro.initialize();//init accel/gyro
	auto this_time = std::chrono::high_resolution_clock::now();
	auto last_time = std::chrono::high_resolution_clock::now();
  int i=0;
  do{

  usleep(10000);
  i++;
  this_time = std::chrono::high_resolution_clock::now();
  auto dt = std::chrono::duration_cast<std::chrono::microseconds>(this_time - last_time).count();
  last_time = std::chrono::high_resolution_clock::now();

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  delta_seconds= ((float) dt) / 1000000;

//this is distance moved vector
//x,y,z ar different than acell
// gyro.x= (gy/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds; //this is pitch
// gyro.y=(-gz/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds; //this moves  yaw (z apparently)
// gyro.z= (-gx/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds;//roll

 gyro.x=(-gx/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds;//roll
 gyro.y=(-gy/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds; //pitch
 gyro.z=(gz/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds;//yaw

  updateQuatByRotation(&ooo,&gyro); 
  toEulerAngle(ooo,roll,pitch,yaw);


	float acc_total_vector = sqrt((ax*ax)+(ay*ay)+(az*az));
	if(abs(ax) < acc_total_vector){
		 a_pitch_final = asin((float)ax/acc_total_vector);
	}
	if(abs(ay) < acc_total_vector){
		 a_roll_final = -asin((float)ay/acc_total_vector);
	}

////double gyro_yaw,gyro_pitch,gyro_roll; 

	//fprintf(stderr, " pitch: %f \n,  ",gyro_roll/0.0174);
	//fprintf(stderr, " pitch: %f,  ",gyro_pitch/0.0174);
	//fprintf(stderr, " yaw: %f \n",gyro_yaw/0.0174);
		
	
	Vec3 accell_correction;
	accell_correction.x=(a_roll_final-roll) * 5 * delta_seconds;
	accell_correction.y=(a_pitch_final-pitch) * 5 * delta_seconds ;
	accell_correction.z=0;//(a_roll_final-gyro_roll) * .02;

    updateQuatByRotation(&ooo,&accell_correction); 

    //forward vector:
    Vec3 f_vec; 
    f_vec.x = 0;
    f_vec.y = 0;
    f_vec.z = 1;

	Vec3 ff_vec=rotate_vector_by_quaternion(f_vec,ooo);
	fprintf(stderr, " pitch accel x,y,z: %f, %f, %f \n",ff_vec.x,ff_vec.y,ff_vec.z);

    
    //up vector
//  x = 2 * (x*y - w*z)
//  y = 1 - 2 * (x*x + z*z)
//  z = 2 * (y*z + w*x)
//  
//  //left vector
//  x = 1 - 2 * (y*y + z*z)
//  y = 2 * (x*y + w*z)
//  z = 2 * (x*z - w*y)

	
////	  //add in the moving of accell 
//	  vec3 acceleration=vec3(0,az,0); //az is up/down

////	  //current accelleration is in m/second //mm, second
////	  acceleration*= 0.023809523f ;//scale this junk

////	  velocity=(velocity)+(acceleration * delta_seconds);

////	  //velocity +=vec3(.01,0,0);
////	  //fprintf(stderr, " x: %6hd, y:%6hd ,z:%6hd \n",ax,ay,az);
////		  

////	   // glm::mat4 translateMatrix = glm::translate(velocity.x,velocity.y,velocity.z);
////		  

////	 //Model=  glm::toMat4(currentOrientation);
////  ////    Model = translate(Model, velocity); 



////   //Model = glm::translate(0,0,0);


}while(1);
}

void computeMatricesFromInputs(){

////// glfwGetTime is called only once, the first time this function is called
////static double lastTime = glfwGetTime();

////// Compute time difference between current and last frame
////double currentTime = glfwGetTime();
////float deltaTime = float(currentTime - lastTime);

////// Get mouse position
////double xpos, ypos;
////glfwGetCursorPos(window, &xpos, &ypos);

////// Reset mouse position for next frame
////glfwSetCursorPos(window, 1024/2, 768/2);

////// Compute new orientation
////horizontalAngle += mouseSpeed * float(1024/2 - xpos );
////verticalAngle   += mouseSpeed * float( 768/2 - ypos );

////// Direction : Spherical coordinates to Cartesian coordinates conversion
////glm::vec3 direction(
////    cos(verticalAngle) * sin(horizontalAngle), 
////    sin(verticalAngle),
////    cos(verticalAngle) * cos(horizontalAngle)
////);

//////glm::vec3 direction(0,0,-1);
////
////// Right vector
////glm::vec3 right = glm::vec3(
////	sin(horizontalAngle - 3.14f/2.0f), 
////	0,
////	cos(horizontalAngle - 3.14f/2.0f)
////);
////
////// Up vector
////glm::vec3 up = glm::cross( right, direction );
//////glm::vec3 up = glm::vec3( 0,1,0  );


////// Move forward
////if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
////	position += direction * deltaTime * speed;
////}
////// Move backward
////if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
////	position -= direction * deltaTime * speed;
////}
////// Strafe right
////if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
////	position += right * deltaTime * speed;
////}
////// Strafe left
////if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
////	position -= right * deltaTime * speed;
////}

////float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

////// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
////ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 200.0f);


////// Camera matrix
////ViewMatrix       = glm::lookAt(
////						position,           // Camera is here
////						position+direction, // and looks here : at the same position, plus "direction"
////						up                  // Head is up (set to 0,-1,0 to look upside-down)
////				   );

////// For the next frame, the "last time" will be "now"
////lastTime = currentTime;
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}


	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void quatToMatrix(Quat q_bad,Mat4 *m){
	//I want x to be roll

	Quat q;
	q.x=q_bad.y;
	q.y=q_bad.z;//
	q.z=q_bad.x;

	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	x2 = q.x + q.x;
    y2 = q.y + q.y;
    z2 = q.z + q.z;

	xx = q.x * x2;
    xy = q.x * y2;
    xz = q.x * z2;

	yy = q.y * y2;
    yz = q.y * z2;
    zz = q.z * z2;
	wx = q.w * x2;
    wy = q.w * y2;
    wz = q.w * z2;

////_1_1 = 1.0f-(yy + zz); 
////_1_2 = xy + wz;
////_1_3 = xz - wy;
////_1_4 = 0;

////_2_1 = xy - wz;    
////_2_2 = 1.0f-(xx + zz);
////_2_3 = yz + wx;  
////_2_4 = 0;

////_3_1 = xz + wy;  
////_3_2 = yz - wx;  
////_3_3 = 1.0f-(xx + yy);
////_3_4 = 0;

////_4_1 = 0.0;
////_4_2 = 0.0;    
////_4_3 = 0.0;    
////_4_4 = 1;

 m->m[0] =1.0f-(yy + zz); 
 m->m[1] = xy + wz; 
 m->m[2] = xz - wy; 
 m->m[3]= 0;
		 
 m->m[4] = xy - wz;
 m->m[5] = 1.0f-(xx + zz); 
 m->m[6] = yz + wx;   
 m->m[7] = 0;
		 
 m->m[8] = xz + wy;  
 m->m[9] = yz - wx;  
 m->m[10]= 1.0f-(xx + yy);
 m->m[11] = 0;
		 
 m->m[12] =0;
 m->m[13] =0;
 m->m[14] =0;
 m->m[15] =1;

////m->m[0] =1.0f-(yy + zz); 
////m->m[4] = xy + wz; 
////m->m[8] = xz - wy; 
////m->m[12]= 0;
////	 
////m->m[1] = xy - wz;
////m->m[5] = 1.0f-(xx + zz); 
////m->m[9] =  yz + wx;   
////m->m[13] = 0;
////	 
////m->m[2] = xz + wy;  
////m->m[6] = yz - wx;  
////m->m[10]= 1.0f-(xx + yy);
////m->m[14] = 0;
////	 
////m->m[3] =0;
////m->m[7] =0;
////m->m[11] =0;
////m->m[15] =1;


}

void toEulerAngle(Quat &q, double &roll, double &pitch, double &yaw)
{
	// roll (x-axis rotation)
	double sinr = +2.0 * (q.w * q.x + q.y * q.z);
	double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
	roll = atan2(sinr, cosr);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (q.w* q.y - q.z * q.x);
        if (fabs(sinp) >= 1)
            pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
	    pitch = asin(sinp);
	// yaw (z-axis rotation)
	double siny = +2.0 * (q.w * q.z + q.x * q.y);
	double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);  
	yaw = atan2(siny, cosy);
}

void toQuaternion(Quat& q, double roll, double pitch,double yaw)
{
        // Abbreviations for the various angular functions
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);

	q.w = cy * cr * cp + sy * sr * sp;
	q.x = cy * sr * cp - sy * cr * sp;
	q.y = cy * cr * sp + sy * sr * cp;
	q.z = sy * cr * cp - cy * sr * sp;
}


Vec3 v_norm(Vec3 v){
    float l=sqrt(
        (v.x*v.x) +
        (v.y*v.y) +
        (v.z*v.z)
    );
	 Vec3 nv;
	nv.x=v.x/l;
	nv.y=v.y/l;
	nv.z=v.z/l;
	return nv;
}

float v_dot( Vec3 v1, Vec3 v2){  
   Vec3 v1n=v_norm(v1); 
   Vec3 v2n=v_norm(v2); 
  return v1n.x * v2n.x + v1n.y * v2n.y + v1n.z * v2n.z; 
}

Vec3 v_cross( Vec3 a, Vec3 b){
        Vec3 v;
        v.x=(a.y*b.z) - (a.z*b.y);
        v.y= (a.z*b.x) - (a.x*b.z);
        v.z= (a.x*b.y) -(a.y*b.x);
        return v;
}

Vec3 v_mult_s( Vec3 v,float m){
     Vec3 vf;
    vf.x=v.x*m;
    vf.y=v.y*m;
    vf.z=v.z*m;
    return vf;
}

Vec3 rotate_vector_by_quaternion(Vec3 v, Quat q){
    Vec3 u(q.x, q.y, q.z);
    float s = q.w;
    float vprime = 2.0f * v_dot(u, v) * u + (s*s - v_dot(u, u)) * v + 2.0f * s;
    Vec3 v_cross=v_cross(u, v);

  return v_mult_s(v_cross,prime);
}

