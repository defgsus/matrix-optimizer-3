#version 330

in vec4 a_position;
in vec4 a_texCoord;

uniform mat4 u_viewTransform;
uniform mat4 u_cvt; // XXX Drawable eats the u_cubeViewTransform variable

out vec4 v_texCoord;
out mat3 v_dir_matrix;

void main(void)
{
    v_texCoord = a_texCoord;
    v_dir_matrix = mat3(u_cvt);

    gl_Position = u_viewTransform * a_position;
}
