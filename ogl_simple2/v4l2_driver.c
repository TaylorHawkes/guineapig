#include "v4l2_driver.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#define CALIBRATING 0
#define CALIBRATING2 0
#define EDGE_DETECT 1

int IMAGE_WIDTH = 640;
int IMAGE_HEIGHT = 480;
struct v4l2_ubuffer *v4l2_ubuffers;
struct v4l2_ubuffer *v4l2_ubuffers2;
struct  edge_array **second_image_edges;


int image_1_edge_count=0;
int image_1_edge_match_count_total=0;
int c=0;

unsigned char   *data_buffer1;
unsigned char   *data_buffer2;
unsigned char   *final_image;
float  *final_image_pos;



size_t stride;  


int v4l2_open(const char *device) {
  struct stat st;
  memset(&st, 0, sizeof(st));
  if (stat(device, &st) == -1) {
    perror("stat");
    return -1;
  }
  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no character device\n", device);
    return -1;
  } else
    printf("%s is a character device\n", device);
  return open(device, O_RDWR | O_NONBLOCK, 0);
}

int v4l2_close(int fd) { return close(fd); }

int v4l2_querycap(int fd, const char *device) {
  struct v4l2_capability cap;
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
    printf("Error opening device %s: unable to query device.\n", device);
    return -1;
  } else {
    printf("driver:\t\t%s\n", cap.driver);
    printf("card:\t\t%s\n", cap.card);
    printf("bus_info:\t%s\n", cap.bus_info);
    printf("version:\t%d\n", cap.version);
    printf("capabilities:\t%x\n", cap.capabilities);

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) {
      printf("Device %s: supports capture.\n", device);
    }

    if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) {
      printf("Device %s: supports streaming.\n", device);
    }
  }

  // emu all support fmt
  struct v4l2_fmtdesc fmtdesc;
  fmtdesc.index = 0;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  printf("\033[31mSupport format:\n\033[0m");
  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
    printf("\033[31m\t%d.%s\n\033[0m", fmtdesc.index + 1, fmtdesc.description);
    fmtdesc.index++;
  }
  return 0;
}

// set format
int v4l2_sfmt(int fd, uint32_t pfmt) {
  // set fmt
  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = pfmt;
  fmt.fmt.pix.height = IMAGE_HEIGHT;
  fmt.fmt.pix.width = IMAGE_WIDTH;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    fprintf(stderr, "Unable to set format\n");
    return -1;
  }
  return 0;
}

// get format
int v4l2_gfmt(int fd) {
  // set fmt
  struct v4l2_format fmt;
  if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
    fprintf(stderr, "Unable to get format\n");
    return -1;
  }

  //set the stride
 stride = fmt.fmt.pix.bytesperline;

  printf("\033[33mpix.pixelformat:\t%c%c%c%c\n\033[0m",
         fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
         (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
         (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
  printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);
  printf("pix.width:\t\t%d\n", fmt.fmt.pix.width);
  printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);
  return 0;
}

int v4l2_sfps(int fd, int fps) {
  struct v4l2_streamparm setfps;
  memset(&setfps, 0, sizeof(setfps));
  setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  setfps.parm.capture.timeperframe.numerator = 1;
  setfps.parm.capture.timeperframe.denominator = fps;
  if (ioctl(fd, VIDIOC_S_PARM, &setfps) == -1) {
    // no fatal error ,just put err msg
    fprintf(stderr, "Unable to set framerate\n");
    return -1;
  }
  return 0;
}

int v4l2_mmap2(int fd){

  //map for rgb data
  data_buffer2 = (unsigned char *) malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 3 * sizeof(char));


  // request for 4 buffers
  struct v4l2_requestbuffers req;
  req.count = BUF_NUM;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    fprintf(stderr, "request for buffers error\n");
    return -1;
  }

   // mmap for buffers
  v4l2_ubuffers2 = (struct v4l2_ubuffer*) malloc(req.count * sizeof(struct v4l2_ubuffer));
  if (!v4l2_ubuffers2) {
    fprintf(stderr, "Out of memory\n");
    return -1;
  }


  struct v4l2_buffer buf;
  unsigned int n_buffers;
  for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;
    // query buffers
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
      fprintf(stderr, "query buffer error\n");
      return -1;
    }

    v4l2_ubuffers2[n_buffers].length = buf.length;
    // map 4 buffers in driver space to usersapce
    v4l2_ubuffers2[n_buffers].start = mmap(
        NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
#ifdef DEBUG
    printf("buffer offset:%d\tlength:%d\n", buf.m.offset, buf.length);
#endif
    /**
      *  output:
      *  buffer offset:0	length:614400
      *  buffer offset:614400	length:614400
      *  buffer offset:1228800	length:614400
      *  buffer offset:1843200	length:614400
      *
      *  explanation：saved in YUV422 format，a pixel needs 2 byte storage in
      *  average，as our image size is 640*480. 640*480*2=614400
    */
    if (v4l2_ubuffers2[n_buffers].start == MAP_FAILED) {
      fprintf(stderr, "buffer map error %u\n", n_buffers);
      return -1;
    }
  }
  return 0;
}

int v4l2_mmap(int fd){

  data_buffer1 = (unsigned char *) malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 3 * sizeof(char));
  final_image = (unsigned char *) malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 3 * sizeof(char));
  final_image_pos = (float *) malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 3 * sizeof(float));

 // second_image_edges = (unsigned char *) malloc(IMAGE_HEIGHT * IMAGE_WIDTH * 6 * sizeof(char));

  // request for 4 buffers
  struct v4l2_requestbuffers req;
  req.count = BUF_NUM;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
    fprintf(stderr, "request for buffers error\n");
    return -1;
  }

  // mmap for buffers
  v4l2_ubuffers = (struct v4l2_ubuffer*) malloc(req.count * sizeof(struct v4l2_ubuffer));
  if (!v4l2_ubuffers) {
    fprintf(stderr, "Out of memory\n");
    return -1;
  }

  struct v4l2_buffer buf;
  unsigned int n_buffers;
  for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;
    // query buffers
    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
      fprintf(stderr, "query buffer error\n");
      return -1;
    }

    v4l2_ubuffers[n_buffers].length = buf.length;
    // map 4 buffers in driver space to usersapce
    v4l2_ubuffers[n_buffers].start = mmap(
        NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
#ifdef DEBUG
    printf("buffer offset:%d\tlength:%d\n", buf.m.offset, buf.length);
#endif
    /**
      *  output:
      *  buffer offset:0	length:614400
      *  buffer offset:614400	length:614400
      *  buffer offset:1228800	length:614400
      *  buffer offset:1843200	length:614400
      *
      *  explanation：saved in YUV422 format，a pixel needs 2 byte storage in
      *  average，as our image size is 640*480. 640*480*2=614400
    */
    if (v4l2_ubuffers[n_buffers].start == MAP_FAILED) {
      fprintf(stderr, "buffer map error %u\n", n_buffers);
      return -1;
    }
  }
  return 0;
}

int v4l2_munmap2() {
  int i;
  for (i = 0; i < BUF_NUM; i++) {
    if (munmap(v4l2_ubuffers2[i].start, v4l2_ubuffers2[i].length) == -1) {
      fprintf(stderr, "munmap 2 failure %d\n", i);
      return -1;
    }
  }
  return 0;
}

int v4l2_munmap() {
  int i;
  for (i = 0; i < BUF_NUM; i++) {
    if (munmap(v4l2_ubuffers[i].start, v4l2_ubuffers[i].length) == -1) {
      fprintf(stderr, "munmap failure %d\n", i);
      return -1;
    }
  }
  return 0;
}

int v4l2_streamon(int fd) {
  // queue in the four buffers allocated by VIDIOC_REQBUFS, pretty like water
  // filling a bottle in turn
  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  unsigned int n_buffers;
  for (n_buffers = 0; n_buffers < BUF_NUM; n_buffers++) {
    buf.index = n_buffers;
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      fprintf(stderr, "queue buffer failed\n");
      return -1;
    }
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    fprintf(stderr, "stream on failed\n");
    return -1;
  }
  return 0;
}

int v4l2_streamoff(int fd) {
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)) {
    fprintf(stderr, "stream off failed\n");
    return -1;
  }
  printf("stream is off\n");
  return 0;
}

void compare_images(){


    int x,y,yy;

#if CALIBRATING2
    int hits=0;
  for (x = 200; x < (400); x += 1) {
      for (y = 0; y < (IMAGE_WIDTH * 3); y += 3) {
              int color=data_buffer1[3*(x*IMAGE_WIDTH)+y+1];
               final_image[3*(x*IMAGE_WIDTH)+y]=color;
               final_image[3*(x*IMAGE_WIDTH)+y+1]=color;
               final_image[3*(x*IMAGE_WIDTH)+y+2]=color;
      }
  }

return;
#endif
#if CALIBRATING
    int hits=0;
  for (x = 200; x < (201); x += 1) {
      for (y = 0; y < (IMAGE_WIDTH * 3); y += 3) {
	  double d_smallest=1000;
	  int best_match_pixel=0;
          for (yy = y; yy > 3; yy -= 3) {
                double d = sqrt(
                      //pixel 1
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y]-data_buffer2[3*(x*IMAGE_WIDTH)+yy],2)+
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+1]-data_buffer2[3*(x*IMAGE_WIDTH)+yy+1],2)+
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+2]-data_buffer2[3*(x*IMAGE_WIDTH)+yy+2],2)+
                      //pixel2
                  pow(data_buffer1[x*IMAGE_WIDTH+y+1]-data_buffer2[x*IMAGE_WIDTH+yy+1],2)+
                  pow(data_buffer1[x*IMAGE_WIDTH+y+1+1]-data_buffer2[x*IMAGE_WIDTH+yy+1+1],2)+
                  pow(data_buffer1[x*IMAGE_WIDTH+y+2+1]-data_buffer2[x*IMAGE_WIDTH+yy+2+1],2)

                 );

                 if(d < d_smallest){
                    d_smallest=d;
                 }
                 if(d_smallest <=5){
                     best_match_pixel=yy;
                     hits+=1;
                     break;
                 }

          }
      }

  }

fprintf(stderr, "hits %i\n", hits);
return;
#endif

#if EDGE_DETECT
    int hits=0;
    int edge=0;
    int color=0;

//first we populate the second image with edges

second_image_edges = (struct edge_array **) malloc(IMAGE_HEIGHT *  sizeof(struct edge_array*));

for (x = 0; x < (IMAGE_HEIGHT); x += 1) {
    second_image_edges[x] = (struct edge_array*) malloc(IMAGE_WIDTH * sizeof(struct edge_array));
}


int second_image_edges_counts[480]={0};
int second_image_edges_count=0;

  for (x = 0; x < (IMAGE_HEIGHT); x += 1) {
      for (y = 0; y < (IMAGE_WIDTH * 3); y += 3) {
            double d = sqrt(
              pow(data_buffer2[3*(x*IMAGE_WIDTH)+y]-data_buffer2[3*(x*IMAGE_WIDTH)+y+3],2)+
              pow(data_buffer2[3*(x*IMAGE_WIDTH)+y+1]-data_buffer2[3*(x*IMAGE_WIDTH)+y+4],2)+
              pow(data_buffer2[3*(x*IMAGE_WIDTH)+y+2]-data_buffer2[3*(x*IMAGE_WIDTH)+y+5],2)
            );
            if(d > 15){
                 edge=1;
                 second_image_edges_counts[x]+=1;
                //second_image_edges_counts
            }else{
                 edge=0;
            }
            if(edge){
               struct edge_array e; 
               e.p1_r= data_buffer2[3*(x*IMAGE_WIDTH)+y +0];
               e.p1_g= data_buffer2[3*(x*IMAGE_WIDTH)+y +1];
               e.p1_b= data_buffer2[3*(x*IMAGE_WIDTH)+y +2];
               e.p2_r= data_buffer2[3*(x*IMAGE_WIDTH)+y +3];
               e.p2_g= data_buffer2[3*(x*IMAGE_WIDTH)+y +4];
               e.p2_b= data_buffer2[3*(x*IMAGE_WIDTH)+y +5];
               e.x=x;
               e.y=y;
               //this is a cube we are searching
               second_image_edges[x][second_image_edges_counts[x]]=e;
            }
    }
  }

 int fifty_count=0;
 int xx=0;
 image_1_edge_count=0;
  for (x = 0; x < (IMAGE_HEIGHT); x += 1) {
      for (y = 0; y < (IMAGE_WIDTH * 3); y += 3) {
            double d = sqrt(
              pow(data_buffer1[3*(x*IMAGE_WIDTH)+y]-data_buffer1[3*(x*IMAGE_WIDTH)+y+3],2)+
              pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+1]-data_buffer1[3*(x*IMAGE_WIDTH)+y+4],2)+
              pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+2]-data_buffer1[3*(x*IMAGE_WIDTH)+y+5],2)
            );
            
            if(d > 20){
                 edge=1;
                 image_1_edge_count +=1;
            }else{
                 edge=0;
            }
            //what if we assume simliar amount of edges then only check back and foward kinda close to this amount???

            fifty_count+=1;
            if(edge && fifty_count >= 5){
                fifty_count=0;
                 double d_smallest=1000;
                 int best_match_pixel=0;
                 
                 int xx_min=x-3;
                 int xx_max=x+5;
                 if(xx_min < 0) {
                     xx_min=0;
                 }
                 if(xx_max > IMAGE_HEIGHT){
                     xx_max=IMAGE_HEIGHT;
                 }
                 
                 //only test every 50th pixel
                 for (yy = 0; yy < second_image_edges_counts[x]; yy += 1) {
                     for (xx = xx_min; xx < xx_max; xx += 1) {
                        double d = sqrt(
                              //pixel 1
                         pow((int) data_buffer1[3*(x*IMAGE_WIDTH)+y+0]- (int) second_image_edges[xx][yy].p1_r,2)+
                         pow((int) data_buffer1[3*(x*IMAGE_WIDTH)+y+1]- (int) second_image_edges[xx][yy].p1_g,2)+
                         pow((int) data_buffer1[3*(x*IMAGE_WIDTH)+y+2]- (int) second_image_edges[xx][yy].p1_b,2)+
                       //     //pixel2
                         pow((int) data_buffer1[x*IMAGE_WIDTH+y+1+0]- (int) second_image_edges[xx][yy].p2_r,2)+
                         pow((int) data_buffer1[x*IMAGE_WIDTH+y+1+1]- (int) second_image_edges[xx][yy].p2_g,2)+
                         pow((int) data_buffer1[x*IMAGE_WIDTH+y+1+2]- (int) second_image_edges[xx][yy].p2_b,2)
                         );

                         if(d <=8){
                             best_match_pixel=second_image_edges[xx][yy].y;
                             break;
                         }
                    }
                    if(best_match_pixel){
                        break;
                    }
              }

              //if we found a match
              if(best_match_pixel){
                 //fprintf(stderr," :good we hit here --------------- \n");
                  //y = 640 * 3
                  //best patch pixel is 
                int distance=680-((best_match_pixel-y)/3);
                //int color= 255- round(distance * .6375);
                   final_image[3*(x*IMAGE_WIDTH)+y]=color;
                   final_image[3*(x*IMAGE_WIDTH)+y+1]=color;
                   final_image[3*(x*IMAGE_WIDTH)+y+2]=color;

                   final_image_pos[c+0]=x;
                   final_image_pos[c+1]=(y/3);
                   final_image_pos[c+2]=distance;
                   c+=3;
                   image_1_edge_match_count_total+=1;
              }else{
                 //fprintf(stderr," bad miss\n");
                   final_image[3*(x*IMAGE_WIDTH)+y]=0;
                   final_image[3*(x*IMAGE_WIDTH)+y+1]=0;
                   final_image[3*(x*IMAGE_WIDTH)+y+2]=0;
              }
        }else{
               final_image[3*(x*IMAGE_WIDTH)+y]=0;
               final_image[3*(x*IMAGE_WIDTH)+y+1]=0;
               final_image[3*(x*IMAGE_WIDTH)+y+2]=0;

        }


      }

  }


    fprintf(stderr,"Edge count image 1 %d\n",image_1_edge_count);
    fprintf(stderr,"Edge count total matches: %d\n",image_1_edge_match_count_total);
return;
#endif


//we know the pixel will be to the right of the image

// fprintf(stderr,"%d",image_1[20]);
// fprintf(stderr,"\n");

//  int j;
//  for (j = 0; j + 1 < (IMAGE_WIDTH*IMAGE_HEIGHT); j += 1) {
//          fprintf(stderr,"%d",image_1[j]);
//  }

  for (x = 0; x < (400); x += 1) {
      for (y = 0; y < (IMAGE_WIDTH * 3); y += 3) {

	  double d_smallest=1000;
	  int best_match_pixel=0;
          for (yy = y; yy < IMAGE_WIDTH*3; yy += 3) {
                double d = sqrt(
                      //pixel 1
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y]-data_buffer2[3*(x*IMAGE_WIDTH)+yy],2)+
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+1]-data_buffer2[3*(x*IMAGE_WIDTH)+yy+1],2)+
                  pow(data_buffer1[3*(x*IMAGE_WIDTH)+y+2]-data_buffer2[3*(x*IMAGE_WIDTH)+yy+2],2)+
                      //pixel2
                  pow(data_buffer1[x*IMAGE_WIDTH+y+1]-data_buffer2[x*IMAGE_WIDTH+yy+1],2)+
                  pow(data_buffer1[x*IMAGE_WIDTH+y+1+1]-data_buffer2[x*IMAGE_WIDTH+yy+1+1],2)+
                  pow(data_buffer1[x*IMAGE_WIDTH+y+2+1]-data_buffer2[x*IMAGE_WIDTH+yy+2+1],2)
        /////////////         //pixel3
/////////////   pow(data_buffer1[x*IMAGE_WIDTH+y+2]-data_buffer2[x*IMAGE_WIDTH+yy+2],2)+
/////////////   pow(data_buffer1[x*IMAGE_WIDTH+y+1+2]-data_buffer2[x*IMAGE_WIDTH+yy+1+2],2)+
/////////////   pow(data_buffer1[x*IMAGE_WIDTH+y+2+2]-data_buffer2[x*IMAGE_WIDTH+yy+2+2],2)

                 );

             //  fprintf(stderr,"%d \n",(int) d);
                 if(d < d_smallest){
                    d_smallest=d;
                 }
                 if(d_smallest <=20){
                     best_match_pixel=yy;
                     break;
                 }

          }
          //if we found a match
          if(best_match_pixel){
              //y = 640 * 3
              //best patch pixel is 
              int distance=(best_match_pixel-y)/3;
              int color=255 - round(distance * .6375);

               final_image[3*(x*IMAGE_WIDTH)+y]=color;
               final_image[3*(x*IMAGE_WIDTH)+y+1]=color;
               final_image[3*(x*IMAGE_WIDTH)+y+2]=color;
          }else{
               final_image[3*(x*IMAGE_WIDTH)+y]=255;
               final_image[3*(x*IMAGE_WIDTH)+y+1]=255;
               final_image[3*(x*IMAGE_WIDTH)+y+2]=255;
          }
      }
  }

}

/*****
 * Taken from libv4l2 (in v4l-utils)
 *
 * (C) 2008 Hans de Goede <hdegoede@redhat.com>
 *
 * Released under LGPL
 */
#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dest ) {
    int j;
    int width=IMAGE_WIDTH;
    int height=IMAGE_HEIGHT;

    while (--height >= 0) {
        for (j = 0; j + 1 < width; j += 2) {
            int u = src[1];
            int v = src[3];
            int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
            int rg = (((u - 128) << 1) +  (u - 128) +
                    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
            int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

            *dest++ = CLIP(src[0] + v1);
            *dest++ = CLIP(src[0] - rg);
            *dest++ = CLIP(src[0] + u1);

            *dest++ = CLIP(src[2] + v1);
            *dest++ = CLIP(src[2] - rg);
            *dest++ = CLIP(src[2] + u1);
            src += 4;
        }
        src += stride - (width * 2);
    }
}

