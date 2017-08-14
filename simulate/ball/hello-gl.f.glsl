#version 110

uniform float fade_factor;
uniform sampler2D textures[2];
varying vec4 oposition;
varying vec3 N;


void main()
{
    vec4 c=vec4(.5,.5,.5,1);
    vec4 lightPos = vec4(10000, 10000, 10000,1); 
    vec4 toLight = lightPos - oposition;
    float dist = length(toLight);
    vec4 R = vec4(normalize(N),1); 
    vec4 visual_s=R + c;
    float L = 1.0 - clamp(dot(normalize(toLight), R),0.0,1.0);                                           
    c *= pow(L, 0.4);                                                                                    
    gl_FragColor = vec4(normalize(N),1); 

}
