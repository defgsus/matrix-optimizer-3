#version 130


/* defines
 * MO_ENABLE_LIGHTING
 * MO_NUM_LIGHTS x
 * MO_ENABLE_TEXTURE
 * MO_ENABLE_TEXTURE_POST_PROCESS
 * MO_ENABLE_NORMALMAP
 * MO_FRAGMENT_LIGHTING
 * MO_TEXTURE_IS_FULLDOME_CUBE
 */

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;

// --- input from vertex shader ---

in vec3 v_pos;
in vec3 v_pos_world;
in vec3 v_pos_eye;
in vec3 v_cam_dir;
in vec3 v_normal;
in vec3 v_normal_eye;
in vec4 v_color;
in vec2 v_texCoord;
#ifdef MO_ENABLE_LIGHTING
    #ifndef MO_FRAGMENT_LIGHTING
        in vec4 v_light_dir[MO_NUM_LIGHTS];
    #else
        in mat3 v_normal_space;
    #endif
#endif

// --- output to rasterizer ---

out vec4 color;


// --- uniforms ---

uniform vec4 u_color;
#ifdef MO_ENABLE_LIGHTING
    uniform float u_diffuse_exp;
    uniform vec4 u_light_color[MO_NUM_LIGHTS];
    uniform vec3 u_light_pos[MO_NUM_LIGHTS];
    #ifdef MO_FRAGMENT_LIGHTING
        uniform vec4 u_light_direction[MO_NUM_LIGHTS];
        uniform float u_light_dirmix[MO_NUM_LIGHTS];
    #endif
#endif

#ifdef MO_ENABLE_TEXTURE
    #ifndef MO_TEXTURE_IS_FULLDOME_CUBE
        uniform sampler2D tex_0;
    #else
        uniform samplerCube tex_0;
    #endif
#endif

#ifdef MO_ENABLE_TEXTURE_POST_PROCESS
    uniform vec3 u_post_transform; // grayscale, invert, +/-shift
    uniform vec3 u_post_bright; // brightness, contrast, threshold
    uniform vec4 u_post_alpha; // rgb color and to-alpha amount
    uniform float u_post_alpha_edge;
#endif

#ifdef MO_ENABLE_NORMALMAP
uniform sampler2D tex_norm_0;
uniform float u_bump_scale;
vec3 normalmap = mix(
            vec3(0., 0., 1.),
            texture(tex_norm_0, v_texCoord).xyz * 2.0 - 1.0,
            u_bump_scale
        );
#endif



// ------------------- functions ------------------------

#ifdef MO_ENABLE_TEXTURE_POST_PROCESS
vec4 mo_color_post_process(in vec4 col)
{
    // color to alpha
    float cmatch = (1. - distance(u_post_alpha.rgb, col.rgb) / sqrt(3.));
    col.a -= col.a * u_post_alpha.a * smoothstep(u_post_alpha_edge, 1.0, cmatch);

    // brightness/contrast
    col.rgb = clamp(
                (col.rgb - u_post_bright.z) * u_post_bright.y
                    + u_post_bright.z + u_post_bright.x, 0.0, 1.0);

    // color shift
    if (u_post_transform.z > 0.0)
        col.rgb = mix(col.rgb, col.brg, u_post_transform.z);
    else if (u_post_transform.z < 0.0)
        col.rgb = mix(col.rgb, col.gbr, -u_post_transform.z);

    // negative
    col.rgb = mix(col.rgb, 1.0 - col.rgb, u_post_transform.y);

    // grayscale
    col.rgb = mix(col.rgb, vec3(col.r*0.3 + col.g*0.59 + col.b*0.11), u_post_transform.x);

    return col;
}
#endif

/* read from cubemap as if stiched to a fulldome master
   st = [0,1]
   angle [0,360] */
vec4 mo_texture_cube(in samplerCube tex, vec2 st, float angle)
{
    st = st * 2.0 - 1.0;

    float   // distance from center
            dist = length(st);

    if (dist > 1.0)
        return vec4(0., 0., 0., 1.);

    float   // cartesian screen-space to spherical
            theta = dist * HALF_PI * angle / 180.0,
            phi = atan(st.y, st.x);

    // spherical-to-cartesian
    vec3 lookup = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                -cos(theta));

    return texture(tex, lookup);
}


vec4 mo_ambient_color()
{
    return u_color * v_color
#ifdef MO_ENABLE_TEXTURE
    #ifndef MO_ENABLE_TEXTURE_POST_PROCESS
        #ifndef MO_TEXTURE_IS_FULLDOME_CUBE
                * texture(tex_0, v_texCoord)
        #else
                * mo_texture_cube(tex_0, v_texCoord, 180.0)
        #endif
    #else
        #ifndef MO_TEXTURE_IS_FULLDOME_CUBE
                * mo_color_post_process( texture(tex_0, v_texCoord) )
        #else
                * mo_color_post_process( mo_texture_cube(tex_0, v_texCoord, 180.0) )
        #endif
    #endif
#endif
            ;
}

/** Light directions are already in surface normal space,
    so this either returns the normal-map or <0,0,1> */
vec3 mo_normal()
{
#ifndef MO_ENABLE_NORMALMAP
    return vec3(0., 0., 1.);
#else
    return normalmap;
#endif
}


#ifdef MO_ENABLE_LIGHTING

vec4 mo_calc_light_color(in vec3 light_normal, in vec4 color, in float shinyness)
{
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(mo_normal(), light_normal) );
    // shaping the linear light influence
    float diffuse = pow(d, shinyness);

    //vec3 halfvec = reflect( v_cam_dir, v_normal_eye );
    //float spec = pow( max(0.0, dot(v_light_dir_eye[i], halfvec)), 10.0);

    return vec4(diffuse * color.xyz, 0.0);
    //return vec4(diffuse * color.xyz + spec * (0.5+0.5*color.xyz), 0.);
    //return vec4(spec * (0.5+0.5*color.xyz), 0.);
}

#ifndef MO_FRAGMENT_LIGHTING

    vec4 mo_light_color()
    {
        vec4 c = vec4(0., 0., 0., 0.);

        for (int i=0; i<MO_NUM_LIGHTS; ++i)
            c += mo_calc_light_color(
                        v_light_dir[i].xyz, v_light_dir[i].w * u_light_color[i], u_diffuse_exp);

        return c;
    }

#else

    vec4 mo_light_color()
    {
        vec4 c = vec4(0., 0., 0., 0.);

        for (int i=0; i<MO_NUM_LIGHTS; ++i)
        {
            // vector towards light in world coords
            vec3 lightvec = u_light_pos[i] - v_pos_world;
            // distance to lightsource
            float dist = length(lightvec);
            // normalized direction towards lightsource
            vec3 lightvecn = lightvec / dist;
            // normalized direction towards lightsource in surface-normal space
            vec3 ldir = v_normal_space * lightvecn;

            // calculate influence from distance attenuation
            float att = 1.0 / (1.0 + u_light_color[i].w * dist * dist);

            // attenuation from direction
            if (u_light_dirmix[i]>0)
            {
                float diratt = pow( max(0.0, dot(u_light_direction[i].xyz, lightvecn)),
                                    u_light_direction[i].w);
                att *= mix(1.0, diratt, u_light_dirmix[i]);
            }

            c += mo_calc_light_color(ldir, att * u_light_color[i], u_diffuse_exp);
        }

        return c;
    }


#endif // !MO_FRAGMENT_LIGHTING

#else // MO_ENABLE_LIGHTING

vec4 mo_light_color() { return vec4(0.); }

#endif


vec4 mo_toon_color()
{
    float d = max(0.0, dot(-v_cam_dir, v_normal_eye));
    float f = smoothstep(0.3,0.32,d);
    return vec4(f, f, f, 1.0);
}


void main()
{
    // 'ambient' or base color
    vec4 ambcol = mo_ambient_color();

    // adding the light to the base color
    vec4 col = ambcol + mo_light_color();
    //vec4 col = vec4(v_cam_dir, 1.0);
    //vec4 col = vec4(v_pos * 0.5 + 0.5, 1.0);
    //col *= mo_toon_color();

    // final color
    color = vec4(clamp(col, 0.0, 1.0));

    //gl_DepthRangeParameters.
}
