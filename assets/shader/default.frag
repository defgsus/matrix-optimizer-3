#version 130

#define MO_NUM_LIGHTS 3

/* defines
 * MO_ENABLE_TEXTURE
 * MO_ENABLE_NORMALMAP
 */

// --- input from vertex shader ---

in vec3 v_pos;
in vec3 v_pos_eye;
in vec3 v_cam_dir;
in vec3 v_normal;
in vec4 v_color;
in vec2 v_texCoord;
in vec4 v_light_dir[MO_NUM_LIGHTS];


// --- uniforms ---

uniform vec4 u_color;
uniform vec4 u_light_color[MO_NUM_LIGHTS];
uniform vec3 u_light_pos[MO_NUM_LIGHTS];

#ifdef MO_ENABLE_TEXTURE
uniform sampler2D tex_0;
#endif

#ifdef MO_ENABLE_NORMALMAP
uniform sampler2D tex_norm_0;
vec3 normalmap = texture2D(tex_norm_0, v_texCoord).xyz * 2.0 - 1.0;
#endif



vec4 mo_ambient_color()
{
    return u_color * v_color
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


vec4 getLightColor(in vec3 light_normal, in vec4 color, in float shinyness, in int i)
{
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(mo_normal(), light_normal) );
    // shaping the linear light influence
    float diffuse = pow(d, 1.0 + shinyness);

    vec3 halfvec = reflect( v_cam_dir, mo_normal() );

    float spec = pow( max(0.0, dot(light_normal, halfvec)), 10.0);

    return vec4(diffuse * color.xyz /*+ spec * (0.5+0.5*color.xyz)*/, 0.);
}

vec4 mo_light_color()
{
    vec4 c = vec4(0., 0., 0., 0.);

    for (int i=0; i<MO_NUM_LIGHTS; ++i)
        c += getLightColor(v_light_dir[i].xyz, v_light_dir[i].w * u_light_color[i], 4.0, i);

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
    //vec4 col = vec4(v_cam_dir, 1.0);

    // final color
    color = vec4(clamp(col, 0.0, 1.0));

    //gl_DepthRangeParameters.
}
