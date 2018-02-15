g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/controls.o common/controls.cpp 
g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/shader.o common/shader.cpp 
g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/texture.o common/texture.cpp 
g++ -lSDL2 -lSDL2_image -lm  -Wall -g -c -o v4l2_driver.o v4l2_driver.c 

g++ main.cpp  common/shader.o common/controls.o common/texture.o v4l2_driver.o -lGL -lglut -lGLEW -lglfw  -lSDL2 -lSDL2_image -lm 
