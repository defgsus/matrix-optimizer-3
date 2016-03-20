#version 330

in vec4 a_position;
in vec4 a_texCoord;

out vec2 v_texCoord;

void main(void)
{
    v_texCoord = a_texCoord.xy;
    gl_Position = a_position;
}
