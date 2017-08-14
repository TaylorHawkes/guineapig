#version 110

attribute vec4 position;
varying vec2 texcoord;
varying vec4 oposition; 
attribute vec3 normal;     

uniform mat4 Vmatrix;
uniform mat4 Pmatrix;
uniform mat4 Mmatrix;
varying vec3 N;  

void main()
{
    vec4 p=Pmatrix*Vmatrix*Mmatrix*vec4(position);
    gl_Position = p;
    texcoord =vec2(0.5); 
    oposition = position;
	N=normal;
}
