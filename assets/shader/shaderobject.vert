#version 330

in vec4 a_position;
//in vec4 a_texCoord;

uniform vec4 u_resolution; // w, h, 1/w, 1/h
//uniform mat4 u_viewTransform;

out vec2 v_pos;
out vec2 v_screen;

void main(void)
{
    //v_texCoord = a_texCoord;
    v_pos = a_position.xy;
    v_screen = a_position.xy * u_resolution.xy;

    gl_Position = a_position;
}
