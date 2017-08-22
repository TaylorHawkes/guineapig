#only so we build the two object files
gcc  -Wall -g -c -o hello-gl.o hello-gl.c -I/usr/local/Cellar/glew/2.1.0/include/
gcc  -Wall -g -c -o util.o util.c -I/usr/local/Cellar/glew/2.1.0/include/

#then we link them together with the output program
gcc -Wall -g -o hello-gl hello-gl.o util.o -framework GLUT -framework OpenGL -L/usr/local/Cellar/glew/2.1.0/lib/ -lGLEW 
