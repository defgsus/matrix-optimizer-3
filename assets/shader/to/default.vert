#include <to/header>

void main(void)
{
    v_texCoord = a_texCoord.xy;
    v_pixelCoord = a_texCoord.xy * u_resolution.xy;

    gl_Position = u_viewTransform * a_position;
}
