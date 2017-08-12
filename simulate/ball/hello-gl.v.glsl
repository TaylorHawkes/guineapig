#version 110

attribute vec4 position;
varying vec2 texcoord;
varying vec4 oposition; 

uniform mat4 Vmatrix;

void main()
{
   vec4 p=Vmatrix*vec4(position);
    gl_Position = p;
    texcoord =vec2(0.5); 
    oposition = position;
}
