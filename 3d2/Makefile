Debug = 0

LDFLAGS = -lSDL2 -lSDL2_image -lpthread -lm
CFLAGS = -Wall
INC_DIR = $(shell pkg-config --cflags sdl2)
EXEC = simple_cam

ifeq ($(Debug), 1)
CFLAGS += -g -DDEBUG
endif

CFLAGS += -ggdb

.PHONY = all clean

all:$(EXEC)

$(EXEC): v4l2_driver.o main.c
	cc $^ $(CFLAGS) $(INC_DIR) $(LDFLAGS) -o $@ 

v4l2_driver.o:v4l2_driver.c
	cc $(CFLAGS) -o $@ -c $^ 

clean:
	rm *.o
	find -type f -executable -exec rm {} \;
