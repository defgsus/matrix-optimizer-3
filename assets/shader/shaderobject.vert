#version 330

in vec4 a_position;
in vec4 a_texCoord;

uniform vec4 u_resolution; // w, h, 1/w, 1/h
uniform mat4 u_viewTransform;

out vec2 v_texCoord;
out vec2 v_pos;
out vec2 v_screen;

void main(void)
{
    vec4 apos = u_viewTransform * a_position;

    v_texCoord = a_texCoord.xy;
    v_pos = apos.xy;
    v_screen = (apos.xy * .5 + .5) * u_resolution.xy;

    gl_Position = apos;
}
