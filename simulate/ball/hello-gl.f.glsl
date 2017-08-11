#version 110

uniform float fade_factor;
uniform sampler2D textures[2];
varying vec4 oposition;


void main()
{
     gl_FragColor = (oposition+.3);
}
