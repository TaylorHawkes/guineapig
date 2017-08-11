#version 110

attribute vec4 position;
varying vec2 texcoord;
varying vec4 oposition; 

void main()
{
    gl_Position = position;
    texcoord =vec2(0.5); 
    oposition = position;
}
