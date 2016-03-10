#version 330

in vec4 v_texCoord;
out vec4 fragColor;

#ifdef MO_USE_COLOR
    uniform vec4 u_color;
#endif

#if defined(MO_FULLDOME_CUBE) || defined(MO_CUBE)
    uniform samplerCube tex_framebuffer;
#else
    uniform sampler2D tex_framebuffer;
#endif

#ifdef MO_ANTIALIAS
    // <xres,yres,1/xres,1/yres>
    uniform vec4 u_resolution;
#endif

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;
const float angle_of_view = 180.0;

// -- cube to unwrapped cube 2d --
vec4 mo_cube_unwrap(in samplerCube tex, in vec2 uv)
{
    uv *= vec2(3,4);
    vec3 f = vec3(fract(uv)*2.-1., 0.);
    ivec2 sect = ivec2(uv);
    if (sect == ivec2(1,3))
            return texture(tex, normalize(vec3(f.x, 1., f.y)));
    if (sect == ivec2(1,2))
            return texture(tex, normalize(vec3(f.x, f.y, -1.)));
    if (sect == ivec2(2,2))
            return texture(tex, normalize(vec3(1., f.y, f.x)));
    if (sect == ivec2(0,2))
            return texture(tex, normalize(vec3(-1., f.y, -f.x)));
    if (sect == ivec2(1,1))
            return texture(tex, normalize(vec3(f.x, -1., -f.y)));
    if (sect == ivec2(1,0))
            return texture(tex, normalize(vec3(f.x, -f.y, 1.)));

    return vec4(0,0,0,0);
}

vec4 mo_pixel(in vec2 texCoord)
{
#ifdef MO_FULLDOME_CUBE
    // -- cube to fulldome master --

    // normalized screen coords
    vec2 scr = texCoord * 2.0 - 1.0;

    float
            // distance from center
            dist = length(scr),
            // cartesian screen-space to spherical
            theta = dist * HALF_PI * angle_of_view / 180.0,
            phi = atan(scr.y, scr.x);

    // spherical-to-cartesian
    vec3 lookup = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                -cos(theta));

    return dist > 1.0 ?
                vec4(0., 0., 0., 1.)
              : texture(tex_framebuffer, lookup);

#elif defined(MO_CUBE)
    // -- cube to unwrapped cube 2d --
    return mo_cube_unwrap(tex_framebuffer, texCoord);

#else
    // -- 2d to 2d --
    return texture(tex_framebuffer, texCoord);
#endif
}



void main(void)
{
#ifndef MO_ANTIALIAS
    fragColor = mo_pixel(v_texCoord.xy);
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

    fragColor = col / float(MO_ANTIALIAS * MO_ANTIALIAS);
#endif

#ifdef MO_USE_COLOR
    fragColor *= u_color;
#endif
}
