#version 130

// vertex attributes
in vec4 a_position;
in vec4 a_color;
in vec3 a_normal;

// shader uniforms
uniform mat4 u_projection;
uniform mat4 u_view;

// output of vertex shader
out vec3 v_pos;
out vec4 v_color;
out vec3 v_normal;

void main()
{
    // pass attributes to fragment shader
    v_pos = a_position.xyz;
    v_color = a_color;
    v_normal = a_normal;

    // set final vertex position
    gl_Position = u_projection * u_view * a_position;
}
