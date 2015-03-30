#version 330

uniform vec4 u_resolution;      // width, height, 1./width, 1./height
uniform float u_time;           // scene time in seconds
uniform mat4 u_transformation;  // object's own transformation

in vec2 v_pos;
in vec4 v_screen;

out vec4 fragColor;

const float PI = 3.14159265358979;
const float TWO_PI = 3.14159265358979 * 2.;
const float HALF_PI = 1.5707963268;


void main(void)
{
    fragColor = vec4(.5+.5*sin(v_pos.x*TWO_PI),
                     .5+.5*sin(v_screen.x*PI),
                     .5+.5*sin(v_pos.y*TWO_PI), 1.);
}
