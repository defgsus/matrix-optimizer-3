#version 130

in vec4 v_texCoord;

#ifndef MO_FULLDOME_CUBE
uniform sampler2D tex_framebuffer;
#else
uniform samplerCube tex_framebuffer;
#endif

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;


void main(void)
{
#ifndef MO_FULLDOME_CUBE
    gl_FragColor = texture2D(tex_framebuffer, v_texCoord.xy);
#else

    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

    float
            // distance from center
            dist = length(scr),
            // cartesian screen-space to spherical
            theta = dist * HALF_PI,// * angle_of_view / 180.0,
            phi = atan(scr.y, scr.x);

    // spherical-to-cartesian
    vec3 lookup = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                -cos(theta));

    gl_FragColor = dist > 1.0 ?
                vec4(1.0, 0., 0., 1.)
              : textureCube(tex_framebuffer, lookup);

#endif
}
