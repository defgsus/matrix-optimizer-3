#include <to/header>

void main(void)
{
    v_texCoord = a_texCoord.xy;

    gl_Position = u_viewTransform * a_position;
}
