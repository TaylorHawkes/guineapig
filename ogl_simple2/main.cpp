#include "v4l2_driver.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>



// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/shader.hpp"
#include "common/controls.hpp"
//to add controls just add header common/controls.hpp and computeMatricesFromInputs(); 

int video_fildes,video_fildes2;

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
  v4l2_mmap(video_fildes2);
  v4l2_streamon(video_fildes2);

}



int main( void )
{
    start_cameras();

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
	
    GLfloat g_vertex_buffer_data[9000] = {0.0};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	do{
        //first we get camera info
        get_camera_data();
        int x,y,c=0;

          for (x = 0; x < (40); x += 1) {
              for (y = 0; y < (40) ; y += 1) {
                  g_vertex_buffer_data[c]   = (GLfloat)  x;
                  g_vertex_buffer_data[c+1] = (GLfloat)  y;
                  g_vertex_buffer_data[c+2] = (GLfloat)  0;//this is depth
                  c+=3;
              }
        }

		glClear( GL_COLOR_BUFFER_BIT );
		glUseProgram(programID);

        computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

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
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
		//glDrawArrays(GL_POINTS, 0, 3); // 3 indices starting at 0 -> 1 triangle

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

	return 0;
}

