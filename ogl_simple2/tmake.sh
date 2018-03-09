# Note to Taylor from Josh:
#   Make sure you compile your performance-sensitive code (like image stuff)
#   with optimization! It will be *way* faster; by default GCC is doing
#   no heavy optimization.
#   Flag is -O3 (highest optimization level)

g++ -Wall -std=c++11 main.cpp ../MPU6050/I2Cdev.o ../MPU6050/MPU6050.o -lGL -lglut -lGLEW -lglfw  -lSDL2 -lSDL2_image -lpthread -lm
