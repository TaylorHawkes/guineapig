# Note to Taylor from Josh:
#   Make sure you compile your performance-sensitive code (like image stuff)
#   with optimization! It will be *way* faster; by default GCC is doing
#   no heavy optimization.
#   Flag is -O3 (highest optimization level)

g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/controls.o common/controls.cpp 
#g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/shader.o common/shader.cpp 
#g++ -lGL -lglut -lGLEW -lglfw  -Wall -g -c -o common/texture.o common/texture.cpp 
g++ -lSDL2 -lSDL2_image -lm  -Wall -g -c -o v4l2_driver.o v4l2_driver.c 

g++ -Wall -std=c++11 main.cpp ../MPU6050/I2Cdev.o ../MPU6050/MPU6050.o common/shader.o common/controls.o common/texture.o v4l2_driver.o -lGL -lglut -lGLEW -lglfw  -lSDL2 -lSDL2_image -lpthread -lm 
