#only so we build the two object files
#gcc  -Wall -g -c -o ball/hello-gl.o ball/hello-gl.c -I/usr/local/Cellar/glew/2.1.0/include/
gcc  -Wall -g -c -o ball/util.o ball/util.c -I/usr/local/Cellar/glew/2.1.0/include/
gcc  -Wall -g -c -o sim.o sim.c -I/usr/local/Cellar/glew/2.1.0/include/

#then we link them together with the output program
#gcc -Wall -g -o simulate main.o ball/hello-gl.o ball/util.o -framework GLUT -framework OpenGL -L/usr/local/Cellar/glew/2.1.0/lib/ -lGLEW 
gcc -Wall -g -o simulate sim.o ball/util.o -framework GLUT -framework OpenGL -L/usr/local/Cellar/glew/2.1.0/lib/ -lGLEW 
