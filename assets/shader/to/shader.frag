#include <to/header>

uniform vec4  u_color;

uniform vec3  iResolution;              // resolution of output texture in pixels
uniform float iGlobalTime;              // scene time in seconds
uniform float iChannelTime[4];          // playback of channel in seconds (not defined yet)
uniform vec3  iChannelResolution[4];    // resolution per channel in pixels
uniform vec4  iMouse;                   // (not defined yet)
uniform vec4  iDate;                    // year, month, day, time in seconds
uniform float iSampleRate;              // sound sampling rate in Hertz

uniform sampler2D   iChannel0;
uniform sampler2D   iChannel1;
uniform sampler2D   iChannel2;
uniform sampler2D   iChannel3;

//%mo_user_code%

void main(void)
{
    vec4 col;
    mainImage(col, v_texCoord * iResolution.xy);

    fragColor = u_color * col;
}
