#include <to/header>

#define PROC_LAYER(i__) col = combine_layer(i__, col, texture(u_tex_##i__, v_texCoord.xy));

uniform sampler2D   u_tex_0;
uniform sampler2D   u_tex_1;
uniform sampler2D   u_tex_2;
uniform sampler2D   u_tex_3;
uniform sampler2D   u_tex_4;
uniform sampler2D   u_tex_5;
uniform sampler2D   u_tex_6;
uniform sampler2D   u_tex_7;

vec4 combine_layer(in int layer, in vec4 a, in vec4 b)
{
    vec4 pa = a;
//%mod_func%
    vec4 c;
//%combine_func%
//%restore_func%
    return c;
}

void main(void)
{
    vec4 col = texture(u_tex_0, v_texCoord.xy);

    //%call_func%

    fragColor = col;
}
