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
#include <stdlib.h>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

GLFWwindow* window;

#include "common/shader.hpp"
#include "common/controls.hpp"
//to add controls just add header common/controls.hpp and computeMatricesFromInputs(); 

using namespace glm;

int video_fildes,video_fildes2;
float a_pitch_final,a_roll_final;
int16_t ax, ay, az;
int16_t gx, gy, gz;
vec3 velocity;
int dt; //in miloscends
MPU6050 accelgyro;
float DEGREES_TO_RADIANS=0.01745329251; //multiplier PI/180
float GYRO_DPS =65.5;//https://cdn.sparkfun.com/assets/learn_tutorials/5/5/0/MPU9250REV1.0.pdf



void get_camera_data(){
  fd_set fds;
  fd_set fds2;
  struct v4l2_buffer buf;
  struct v4l2_buffer buf2;

    int ret;
    int ret2;

    FD_ZERO(&fds);
    FD_SET(video_fildes, &fds);

    FD_ZERO(&fds2);
    FD_SET(video_fildes2, &fds2);

    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    ret = select(video_fildes + 1, &fds, NULL, NULL, &tv);
    ret2 = select(video_fildes2 + 1, &fds2, NULL, NULL, &tv);


    //camera 1
    if (FD_ISSET(video_fildes, &fds)) {
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      ioctl(video_fildes, VIDIOC_DQBUF, &buf);

      v4lconvert_yuyv_to_rgb24((unsigned char *) v4l2_ubuffers[buf.index].start, data_buffer1);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      ioctl(video_fildes, VIDIOC_QBUF, &buf);
    }

//camera 2
    if (FD_ISSET(video_fildes2, &fds2)) {
      memset(&buf2, 0, sizeof(buf2));
      buf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf2.memory = V4L2_MEMORY_MMAP;
      ioctl(video_fildes2, VIDIOC_DQBUF, &buf2);

      v4lconvert_yuyv_to_rgb24((unsigned char *) v4l2_ubuffers2[buf2.index].start, data_buffer2);

      buf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf2.memory = V4L2_MEMORY_MMAP;
      ioctl(video_fildes2, VIDIOC_QBUF, &buf2);
    }

    compare_images();

}

void start_cameras(){

  int format= V4L2_PIX_FMT_YUYV;

  //video1
  const char *device = "/dev/video0";
  video_fildes = v4l2_open(device);
  v4l2_querycap(video_fildes, device);
  v4l2_sfmt(video_fildes, format);
  v4l2_gfmt(video_fildes);
  v4l2_sfps(video_fildes, 20);
  v4l2_mmap(video_fildes);
  v4l2_streamon(video_fildes);


  //video2
  const char *device2 = "/dev/video1";
   video_fildes2 = v4l2_open(device2);
  v4l2_querycap(video_fildes2, device2);
  v4l2_sfmt(video_fildes2, format);
  v4l2_gfmt(video_fildes2);
  v4l2_sfps(video_fildes2, 20);
  v4l2_mmap2(video_fildes2);
  v4l2_streamon(video_fildes2);

}




int main( void )
{

    //std::thread t1(updateGyroPositionThead);//crank of the new thread
    //start_cameras();

    glfwInit() ;//no eror
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	window = glfwCreateWindow( 1024, 768, "Tutorial 03 - Matrices", NULL, NULL); //no error
	glfwMakeContextCurrent(window);
	glewExperimental = true; // Needed for core profile
    glewInit();//forget error

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "SimpleTransform.vertexshader", "SingleColor.fragmentshader" );
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
    //GLfloat g_vertex_buffer_data[1000000] = {0.0};
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    GLfloat g_vertex_buffer_data[] = { 
        //rect front
		0.0f,1.0f,0.0f, 
		8.0f,0.0f,0.0f, 
		0.0f, 0.0f,0.0f,
        0.0f,1.0f,0.0f, 
		8.0f,1.0f,0.0f,
		8.0f, 0.0f,0.0f,
        //rect right side
        8.0f,1.0f,0.0f, 
		8.0f,0.0f,-4.0f,
		8.0f, 0.0f,0.0f,
        8.0f,1.0f,0.0f, 
		8.0f,1.0f,-4.0f,
		8.0f, 0.0f,-4.0f,
        //rect left side
        0.0f,1.0f,0.0f, 
		0.0f,0.0f,-4.0f,
		0.0f, 0.0f,0.0f,
        0.0f,1.0f,0.0f, 
		0.0f,1.0f,-4.0f,
		0.0f, 0.0f,-4.0f,
        //rect back
		0.0f,1.0f,-4.0f, 
		8.0f,0.0f,-4.0f,
		0.0f, 0.0f,-4.0f,
        0.0f,1.0f,-4.0f, 
		8.0f,1.0f,-4.0f,
		8.0f, 0.0f,-4.0f,
        //rect bottom
    	0.0f,0.0f,0.0f, 
		8.0f,0.0f,0.0f,
		8.0f, 0.0f,-4.0f,
        0.0f,0.0f,0.0f, 
		8.0f,0.0f,-4.0f,
		0.0f, 0.0f,-4.0f,
        //rect top
    	0.0f,1.0f,0.0f, 
		8.0f,1.0f,0.0f,
		8.0f, 1.0f,-4.0f,
        0.0f,1.0f,0.0f, 
		8.0f,1.0f,-4.0f,
		0.0f, 1.0f,-4.0f,
        //forward triange
    	3.0f,1.0f,-4.0f, 
		5.0f,1.0f,-4.0f,
		4.0f,1.0f,-7.0f,
	};

    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	do{
//        get_camera_data();


		//its closer to 45
		//400 / 10 = 40
		//  for (x = 0; x < (image_1_edge_match_count_total * 3); x += 3) {
		//          //x is actually y (and its backwords)
		//          //y is actually x
		//          //fprintf(stderr, " pos: %f \n",final_image_pos[x+0]);
		//          g_vertex_buffer_data[x]  = (GLfloat)  final_image_pos[x+1]/100;
		//          g_vertex_buffer_data[x+1] = 4 - (GLfloat)  final_image_pos[x+0]/100;
		//          g_vertex_buffer_data[x+2] = - (GLfloat) final_image_pos[x+2] /100;//this is depth
		//  }



    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);

    computeMatricesFromInputs();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();

  glm::mat4 Model      = glm::mat4(1.0f);
  //ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
  //ViewMatrix= glm::lookAt( glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0) );



  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * Model;




    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
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

void hey() {

   vec3 EulerAngles(0, 0, 0);
   quat currentOrientation = quat(EulerAngles); 

  //accelgyro.initialize();//init accel/gyro
//auto this_time = std::chrono::high_resolution_clock::now();
//auto last_time = std::chrono::high_resolution_clock::now();
//  int i=0;
//  do{
//  i++;
//  this_time = std::chrono::high_resolution_clock::now();
//  auto dt = std::chrono::duration_cast<std::chrono::microseconds>(this_time - last_time).count();
//  last_time = std::chrono::high_resolution_clock::now();

//  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

//  int x,y,c=0;


//     float delta_seconds=dt/1000000;

//   //this is distance moved vector
//   //x,y,z ar different than acell
//    vec3 gyro(
//      (-gy/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds, //this is pitch
//       (gz/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds, //this moves  yaw (z apparently)
//      (-gx/GYRO_DPS) * DEGREES_TO_RADIANS * delta_seconds //roll
//    );

//  quat gyroQuat = quat(gyro);

//  currentOrientation = currentOrientation * gyroQuat;//here we update by gyro rotation

//  float acc_total_vector = sqrt((ax*ax)+(ay*ay)+(az*az));
//  if(abs(ax) < acc_total_vector){
//       a_pitch_final = -asin((float)ax/acc_total_vector);
//  }
//  if(abs(ay) < acc_total_vector){
//       a_roll_final = asin((float)ay/acc_total_vector);
//  }

// glm::vec3 gyroEuler = glm::eulerAngles(currentOrientation); 	
// //gyroEuler.x=pitch //gyroEuler.y=yaw //gyroEuler.z=roll

// //this is true position vector 
//  vec3 accell(
//      -a_pitch_final, //this is pitch
//      -gyroEuler.y, //yaw
//      -a_roll_final//roll
//  );

// vec3 acell_gyro_delta(
//   accell.x-gyroEuler.x,//pitch
//   0,//yaw
//   accell.z-gyroEuler.z //roll maybe
// );

//  acell_gyro_delta*= 0.02f;//scale diff down

//  quat accelcorrectionQuat = quat(acell_gyro_delta);
//  currentOrientation*=accelcorrectionQuat;
//      ////    //add in the moving of accell 
//      ////    vec3 acceleration=vec3(0,az,0); //az is up/down

//      ////    //current accelleration is in m/second //mm, second
//      ////    acceleration*= 0.023809523f ;//scale this junk

//      ////    velocity=(velocity)+(acceleration * delta_seconds);

//      ////    //velocity +=vec3(.01,0,0);
//      ////    //fprintf(stderr, " x: %6hd, y:%6hd ,z:%6hd \n",ax,ay,az);
//      ////    

//      //// // glm::mat4 translateMatrix = glm::translate(velocity.x,velocity.y,velocity.z);
//      ////    

//  ////    Model = translate(Model, velocity); 





//      //Model=  glm::toMat4(currentOrientation);
//   //Model = glm::translate(0,0,0);


//      }while(1);
}



