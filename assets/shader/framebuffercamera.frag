#version 130

uniform sampler2D tex_framebuffer;
uniform vec4 u_color;

in vec4 v_texCoord;

void main(void)
{
    gl_FragColor = u_color * texture2D(tex_framebuffer, v_texCoord.xy);
}
