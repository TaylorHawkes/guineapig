#version 110

uniform float fade_factor;
uniform sampler2D textures[2];

varying vec2 texcoord;

void main()
{
     gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
