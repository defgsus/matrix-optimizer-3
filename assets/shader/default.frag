#version 130

#define MO_NUM_LIGHTS 3

/* defines
 * MO_ENABLE_TEXTURE
 * MO_ENABLE_NORMALMAP
 */

// --- input from vertex shader ---
in vec3 v_pos;
in vec4 v_color;
in vec2 v_texCoord;
in vec3 v_light_dir[MO_NUM_LIGHTS];
in mat3 v_normal_matrix;

// --- uniforms ---

uniform vec3 u_light_color[MO_NUM_LIGHTS];

#ifdef MO_ENABLE_TEXTURE
uniform sampler2D tex_0;
#endif

#ifdef MO_ENABLE_NORMALMAP
uniform sampler2D tex_norm_0;
vec3 normalmap = texture2D(tex_norm_0, v_texCoord).xyz * 2.0 - 1.0;
#endif



vec4 mo_ambient_color()
{
    return v_color
#ifdef MO_ENABLE_TEXTURE
            * texture2D(tex_0, v_texCoord)
#endif
            ;
}

vec3 mo_normal()
{
#ifndef MO_ENABLE_NORMALMAP
    return vec3(0., 0., 1.);
#else
    return normalmap;
#endif
}


vec4 getLightColor(in vec3 light_normal, in vec3 color, in float shinyness)
{
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(mo_normal(), light_normal) );
    // shaping the linear light influence
    float lighting = pow(d, 1.0 + shinyness);

    return vec4(lighting * color, 0.);
}

vec4 mo_light_color()
{
    vec4 c = vec4(0., 0., 0., 0.);

    for (int i=0; i<MO_NUM_LIGHTS; ++i)
        c += getLightColor(v_light_dir[i], u_light_color[i], 4.0);

    return c;
}

// output to rasterizer
out vec4 color;

void main()
{
    // 'ambient' or base color
    vec4 ambcol = mo_ambient_color();

    // adding the light to the base color
    vec4 col = ambcol + mo_light_color();
/*            + getLightColor(v_light_dir[0], vec3(1., 1., 1.), 1.5)
            + getLightColor(v_light_dir[1], vec3(0.2, 0.5, 1.), 2.0)
            + getLightColor(v_light_dir[2], vec3(1., .5, .2)*0.4, 3.0);
*/
    // final color
    color = vec4(clamp(col, 0.0, 1.0));

    //gl_DepthRangeParameters.
}
