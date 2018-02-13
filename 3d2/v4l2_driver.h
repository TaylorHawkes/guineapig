#ifndef _V4l2_DRIVER__
#define _V4l2_DRIVER__

#include <inttypes.h>
#include <pthread.h>

//Taylor chaning this 
//#define BUF_NUM 4
//seems to work ok with one
#define BUF_NUM 1

extern int IMAGE_WIDTH;
extern int IMAGE_HEIGHT;
extern size_t stride;  

struct v4l2_ubuffer {
  void *start;
  unsigned int length;
};


struct edge_array {
  unsigned char  p1_r;
  unsigned char  p1_g;
  unsigned char  p1_b;
  unsigned char  p2_r;
  unsigned char  p2_g;
  unsigned char  p2_b;
  int x; 
  int y; 

};

extern struct  edge_array **second_image_edges;

extern struct v4l2_ubuffer *v4l2_ubuffers;
extern struct v4l2_ubuffer *v4l2_ubuffers2;


extern unsigned char   *data_buffer1;
extern unsigned char   *data_buffer2;
extern unsigned char   *final_image;


/* functions */

extern int v4l2_open(const char *device);
extern int v4l2_close(int fd);
extern int v4l2_querycap(int fd, const char *device);
extern int v4l2_sfmt(int fd, uint32_t pfmt);
extern int v4l2_gfmt(int fd);
extern int v4l2_mmap(int fd);
extern int v4l2_mmap2(int fd);
extern int v4l2_munmap();
extern int v4l2_munmap2();
extern int v4l2_sfps(int fd, int fps);
extern int v4l2_streamon(int fd);
extern int v4l2_streamoff(int fd);
extern void v4lconvert_yuyv_to_rgb24(const unsigned char *src, unsigned char *dest);

extern void compare_images();

#endif
