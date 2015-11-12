#version 330

in vec4 a_position;

uniform vec4 u_resolution;      // w, h, 1/w, 1/h
uniform mat4 u_viewTransform;

out vec2 v_pos;
out vec2 v_screen;

void main(void)
{
    vec4 apos = u_viewTransform * a_position;

    v_pos = a_position.xy;
    v_screen = apos.xy * u_resolution.xy;

    gl_Position = apos;
}
