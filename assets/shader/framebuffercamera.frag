#version 330

in vec4 v_texCoord;
out vec4 fragColor;

#ifndef MO_FULLDOME_CUBE
uniform sampler2D tex_framebuffer;
#else
uniform samplerCube tex_framebuffer;
#endif

#ifdef MO_ANTIALIAS
    // <xres,yres,1/xres,1/yres>
    uniform vec4 u_resolution;
#endif
uniform vec4 u_color;
uniform float u_angle;

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;

vec4 mo_pixel(in vec2 texCoord)
{
#ifndef MO_FULLDOME_CUBE
    return texture2D(tex_framebuffer, texCoord);

#else
    // cube to fulldome

    // normalized screen coords
    vec2 scr = texCoord * 2.0 - 1.0;

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

    return dist > 1.0 ?
                vec4(0., 0., 0., 1.)
              : texture(tex_framebuffer, lookup);
#endif
}



void main(void)
{
#ifndef MO_ANTIALIAS
    fragColor = u_color * mo_pixel(v_texCoord.xy);
#else

    vec4 col = vec4(0., 0., 0., 0.);

    for (int y=0; y<MO_ANTIALIAS; y++)
    {
        vec2 ofs = vec2(0., float(y) / MO_ANTIALIAS * u_resolution.w);

        for (int x=0; x<MO_ANTIALIAS; x++)
        {
            ofs.x = float(x) / MO_ANTIALIAS * u_resolution.z;

            col += mo_pixel(v_texCoord.xy + ofs);
        }
    }

    fragColor = u_color * (col / float(MO_ANTIALIAS * MO_ANTIALIAS));
#endif
}
