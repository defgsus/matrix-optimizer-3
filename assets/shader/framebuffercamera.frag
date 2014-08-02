#version 130

in vec4 v_texCoord;

#ifndef MO_FULLDOME_CUBE
uniform sampler2D tex_framebuffer;
#else
uniform samplerCube tex_framebuffer;
#endif

uniform vec4 u_color;
uniform float u_angle;

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;

void main(void)
{
#ifndef MO_FULLDOME_CUBE
    gl_FragColor = u_color * texture2D(tex_framebuffer, v_texCoord.xy);

#else
    // cube to fulldome

    // normalized screen coords
    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

    float
            // distance from center
            dist = length(scr),
            // cartesian screen-space to spherical
            theta = dist * HALF_PI * u_angle / 180.0,
            phi = atan(scr.y, scr.x);

    // spherical-to-cartesian
    vec3 lookup = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                -cos(theta));

    gl_FragColor =
            dist > 1.0 ?
                vec4(0., 0., 0., 1.)
              : u_color * texture(tex_framebuffer, lookup);

#endif
}
