#include <to/header>

uniform vec4  u_color;
uniform sampler2D u_tex_feedback;

uniform vec3  iResolution;              // resolution of output texture in pixels
uniform float iGlobalTime;              // scene time in seconds
uniform float iChannelTime[4];          // playback of channel in seconds (not defined yet)
        vec3  iChannelResolution[4];    // resolution per channel in pixels
uniform vec4  iMouse;                   // x, y in pixels, z, w = buttons
uniform vec4  iDate;                    // year, month, day, time in seconds
uniform float iSampleRate;              // sound sampling rate in Hertz

//%mo_user_code%

void main(void)
{
    vec2 ts_ = vec2(textureSize(iChannel0, 0));
    iChannelResolution[0] = vec3(ts_, ts_.y > 0. ? ts_.x / ts_.y : 1.);
    ts_ = vec2(textureSize(iChannel1, 0));
    iChannelResolution[1] = vec3(ts_, ts_.y > 0. ? ts_.x / ts_.y : 1.);
    ts_ = vec2(textureSize(iChannel2, 0));
    iChannelResolution[2] = vec3(ts_, ts_.y > 0. ? ts_.x / ts_.y : 1.);
    ts_ = vec2(textureSize(iChannel3, 0));
    iChannelResolution[3] = vec3(ts_, ts_.y > 0. ? ts_.x / ts_.y : 1.);

    vec4 col;
    mainImage(col, v_texCoord * iResolution.xy);

    fragColor = u_color * col;
}
