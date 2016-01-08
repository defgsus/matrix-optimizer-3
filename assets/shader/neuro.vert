#version 130

uniform vec2 u_resolution;

in vec4 a_position;

out vec2 v_pixelCoord;

void main()
{
    v_pixelCoord = (a_position.xy * .5 + .5) * u_resolution.xy;
    gl_Position = a_position;
}

