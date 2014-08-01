#version 130

uniform sampler2D tex_framebuffer;

in vec4 v_texCoord;

void main(void)
{
    gl_FragColor = texture2D(tex_framebuffer, v_texCoord.xy);
}
