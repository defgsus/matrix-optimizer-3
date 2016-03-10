#include <to/header>

uniform vec4        u_tex_scale;

void main(void)
{
    // [0,1]
    vec2 uv = v_texCoord.xy;

    uv = u_tex_scale.xy * uv + u_tex_scale.zw;

    vec4 col = texture(u_tex, uv);

    fragColor = mo_color_range(col);
}
