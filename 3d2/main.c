#include "v4l2_driver.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define SAVE_EVERY_FRAME 0
#define DEPTH_MAP 1
#define USE_MJPEG 1
#define CALIBRATING 1

pthread_t thread_stream;
SDL_Window *sdlScreen;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Rect sdlRect;

/* miscellanous */
int thread_exit_sig = 0;

struct streamHandler {
  int fd;
  int fd2;
  void (*framehandler)(void *pframe, int length,void *pframe2,int length2);
};

void print_help() {
  printf("Usage: simple_cam <width> <height> <device>\n");
  printf("Example: simple_cam 640 480 /dev/video0\n");
}

static void frame_handler(void *pframe, int length,void *pframe2, int length2) {

#if !CALIBRATING
v4lconvert_yuyv_to_rgb24((unsigned char *) pframe, data_buffer1);
v4lconvert_yuyv_to_rgb24((unsigned char *) pframe2, data_buffer2);
compare_images();
#endif;


#if !USE_MJPEG

  //SDL_UpdateTexture(sdlTexture, &sdlRect,pframe , IMAGE_WIDTH * 2);
  SDL_UpdateTexture(sdlTexture, &sdlRect, final_image, IMAGE_WIDTH * 3);
  //  SDL_UpdateYUVTexture
  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
  SDL_RenderPresent(sdlRenderer);
#endif

#if USE_MJPEG
	SDL_RWops *buf_stream = SDL_RWFromMem(pframe, (int)length);
	SDL_Surface *frame = IMG_Load_RW(buf_stream, 0);
	SDL_Texture *tx = SDL_CreateTextureFromSurface(sdlRenderer, frame);

	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, tx, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);

	SDL_DestroyTexture(tx);
	SDL_FreeSurface(frame);
	SDL_RWclose(buf_stream);
#endif

#if DEPTH_MAP 

#endif

#if SAVE_EVERY_FRAME

  static yuv_index = 0;
//if(yuv_index < 1){
  char yuvifle[100];
  sprintf(yuvifle, "yuv-%d.yuv", yuv_index);
  FILE *fp = fopen(yuvifle, "wb");
  fwrite(pframe, length, 1, fp);
  fclose(fp);
  yuv_index++;
//}

#endif
}

static void *v4l2_streaming(void *arg) {


  // SDL2 begins
  memset(&sdlRect, 0, sizeof(sdlRect));
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    return NULL;
  }

  sdlScreen = SDL_CreateWindow("Simple YUV Window", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, IMAGE_WIDTH,
                               IMAGE_HEIGHT, SDL_WINDOW_SHOWN);

  if (!sdlScreen) {
    fprintf(stderr, "SDL: could not create window - exiting:%s\n",
            SDL_GetError());
    return NULL;
  }

  sdlRenderer = SDL_CreateRenderer(
      sdlScreen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (sdlRenderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer Error\n");
    return NULL;
  }
    
    //SDL_PIXELFORMAT_YUY2
    //SDL_PIXELFORMAT_RGB24

#if !USE_MJPEG
  sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
  sdlRect.w = IMAGE_WIDTH;
  sdlRect.h = IMAGE_HEIGHT;
#endif

#if USE_MJPEG
if (!IMG_Init(IMG_INIT_JPG)) {
    fprintf(stderr, "Unable  ti init image Error\n");
    return -1;
}
#endif

  int fd = ((struct streamHandler *)(arg))->fd;
  int fd2 = ((struct streamHandler *)(arg))->fd2;
  void (*handler)(void *pframe, int length,void *prame2, int length2) =
      ((struct streamHandler *)(arg))->framehandler;

  fd_set fds;
  fd_set fds2;
  struct v4l2_buffer buf;
  struct v4l2_buffer buf2;
  while (!thread_exit_sig) {
    int ret;
    int ret2;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    FD_ZERO(&fds2);
    FD_SET(fd2, &fds2);

    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    ret2 = select(fd2 + 1, &fds2, NULL, NULL, &tv);

    if (-1 == ret) {
      fprintf(stderr, "select error\n");
      return NULL;
    } else if (0 == ret) {
      fprintf(stderr, "timeout waiting for frame\n");
      continue;
    }
    if (-1 == ret2) {
      fprintf(stderr, "select error\n");
      return NULL;
    } else if (0 == ret2){
      fprintf(stderr, "timeout waiting for 2nd frame\n");
      continue;
    }

    //camera 1
    if (FD_ISSET(fd, &fds)) {
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
        fprintf(stderr, "VIDIOC_DQBUF failure\n");
        return NULL;
      }

///  if (handler)
///    (*handler)(v4l2_ubuffers[buf.index].start,
///               v4l2_ubuffers[buf.index].length);
#if CALIBRATING

if (handler)
  (*handler)(v4l2_ubuffers[buf.index].start,
             v4l2_ubuffers[buf.index].length,
             v4l2_ubuffers[buf.index].start,
             v4l2_ubuffers[buf.index].length);
#endif


      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
        fprintf(stderr, "VIDIOC_QBUF failure\n");
        return NULL;
      }
    }

    //camera 2
    if (FD_ISSET(fd2, &fds2)) {
      memset(&buf2, 0, sizeof(buf2));
      buf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf2.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd2, VIDIOC_DQBUF, &buf2)) {
        fprintf(stderr, "VIDIOC_DQBUF 2 failure\n");
        return NULL;
      }

    //change this back

#if CALIBRATING
//sleep(1);
    usleep(100000);//1/10 of a second
    if (handler)
      (*handler)(v4l2_ubuffers2[buf.index].start,
                 v4l2_ubuffers2[buf.index].length,
                 v4l2_ubuffers2[buf.index].start,
                 v4l2_ubuffers2[buf.index].length);
#endif

#if !CALIBRATING
    if (handler)
      (*handler)(v4l2_ubuffers[buf.index].start,
                 v4l2_ubuffers[buf.index].length,
                 v4l2_ubuffers2[buf.index].start,
                 v4l2_ubuffers2[buf.index].length);
#endif

      buf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf2.memory = V4L2_MEMORY_MMAP;
      if (-1 == ioctl(fd2, VIDIOC_QBUF, &buf2)) {
        fprintf(stderr, "VIDIOC_QBUF 2 failure\n");
        return NULL;
      }
    }

  }
  return NULL;
}

int main(int argc, char const *argv[]) {

  //Thisi for the first device
  const char *device = "/dev/video0";
  if (argc == 2 && (strchr(argv[1], 'h') != NULL)) {
    print_help();
    exit(0);
  }

  if (argc > 2) {
    IMAGE_WIDTH = atoi(argv[1]);
    IMAGE_HEIGHT = atoi(argv[2]);
  }

  if (argc > 3) {
    device = argv[3];
  }

  int video_fildes = v4l2_open(device);
  if (video_fildes == -1) {
    fprintf(stderr, "can't open %s\n", device);
    exit(-1);
  }

  if (v4l2_querycap(video_fildes, device) == -1) {
    perror("v4l2_querycap");
    goto exit_;
  }

  // most of devices support YUYV422 packed.
  // V4L2_PIX_FMT_MJPEG
  // V4L2_PIX_FMT_YUYV

#if USE_MJPEG
    int format= V4L2_PIX_FMT_MJPEG;
#endif

#if !USE_MJPEG
    int format= V4L2_PIX_FMT_YUYV;
#endif

  if (v4l2_sfmt(video_fildes, format) == -1) {
    perror("v4l2_sfmt");
    goto exit_;
  }

  if (v4l2_gfmt(video_fildes) == -1) {
    perror("v4l2_gfmt");
    goto exit_;
  }

  if (v4l2_sfps(video_fildes, 20) == -1) { // no fatal error
    perror("v4l2_sfps");
  }

  if (v4l2_mmap(video_fildes) == -1) {
    perror("v4l2_mmap");
    goto exit_;
  }

  if (v4l2_streamon(video_fildes) == -1) {
    perror("v4l2_streamon");
    goto exit_;
  }

  //end first device setup
  //Thisi for the second device
  const char *device2 = "/dev/video1";
  int video_fildes2 = v4l2_open(device2);
  if (video_fildes2 == -1) {
    fprintf(stderr, "can't open %s\n", device2);
    exit(-1);
  }

  if (v4l2_querycap(video_fildes2, device2) == -1) {
    perror("v4l2_querycap");
    goto exit_;
  }

  //// most of devices support YUYV422 packed.
  if (v4l2_sfmt(video_fildes2, format) == -1) {
    perror("v4l2_sfmt");
    goto exit_;
  }

  if (v4l2_gfmt(video_fildes2) == -1) {
    perror("v4l2_gfmt");
    goto exit_;
  }

  if (v4l2_sfps(video_fildes2, 20) == -1) { // no fatal error
    perror("v4l2_sfps");
  }

  if (v4l2_mmap2(video_fildes2) == -1) {
      perror("v4l2_mmap");
      goto exit_;
  }


  if (v4l2_streamon(video_fildes2) == -1) {
    perror("v4l2_streamon");
    goto exit_;
  }
  //end second device setup


  // create a thread that will update frame int the buffer
  struct streamHandler sH = {video_fildes, video_fildes2,frame_handler};
  if (pthread_create(&thread_stream, NULL, v4l2_streaming, (void *)(&sH))) {
    fprintf(stderr, "create thread failed\n");
    goto exit_;
  }

  int quit = 0;
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) { // click close icon then quit
        quit = 1;
      }
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) // press ESC the quit
          quit = 1;
      }
    }
    usleep(25);
  }

  thread_exit_sig = 1;               // exit thread_stream
  pthread_join(thread_stream, NULL); // wait for thread_stream exiting

  if (v4l2_streamoff(video_fildes) == -1) {
    perror("v4l2_streamoff");
    goto exit_;
  }

if (v4l2_streamoff(video_fildes2) == -1) {
  perror("v4l2_streamoff 2");
  goto exit_;
}

  if (v4l2_munmap() == -1) {
    perror("v4l2_munmap");
    goto exit_;
  }
if (v4l2_munmap2() == -1) {
  perror("v4l2_munmap");
  goto exit_;
}

exit_:
  if (v4l2_close(video_fildes) == -1) {
    perror("v4l2_close");
  };
  SDL_Quit();
  return 0;
}
