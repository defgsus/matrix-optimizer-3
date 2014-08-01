#version 130

attribute vec4 a_position;
attribute vec4 a_texCoord;

uniform mat4 u_view;

out vec4 v_texCoord;

void main(void)
{
    v_texCoord = a_texCoord;

    gl_Position = u_view * a_position;
}
