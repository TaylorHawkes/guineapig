GLEW_INCLUDE = /usr/local/Cellar/glew/2.1.0/include/
GLEW_LIB = /usr/local/Cellar/glew/2.1.0/lib/

hello-gl: hello-gl.o util.o
	gcc -Wall -g -o hello-gl $^ -framework GLUT -framework OpenGL -L$(GLEW_LIB) -lGLEW 

.c.o:
	gcc  -Wall -g -c -o $@ $< -I$(GLEW_INCLUDE)

clean:
	rm -f hello-gl hello-gl-dummy hello-gl.o util.o hello-gl-dummy.o

