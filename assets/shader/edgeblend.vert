#version 330

in vec4 a_position;
in vec4 a_texCoord;

uniform mat4 u_viewTransform;

out vec4 v_texCoord;

void main(void)
{
    v_texCoord = a_texCoord;

    gl_Position = u_viewTransform * a_position;
}
