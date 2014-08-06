#version 130

/* defines
 * MO_ENABLE_TEXTURE
 * MO_ENABLE_NORMALMAP
 */

// input from vertex shader
in vec3 v_pos;
in vec4 v_color;
in vec3 v_normal;
in vec2 v_texCoord;

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
    return v_normal
#ifdef MO_ENABLE_NORMALMAP
            * normalmap
#endif
            ;
}


vec4 getLightColor(in vec3 light_pos, in vec3 color, in float shinyness)
{
    // normal to light source
    vec3 light_normal = normalize( light_pos - v_pos );
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(mo_normal(), light_normal) );
    // shaping the linear light influence
    float lighting = pow(d, 1.0 + shinyness);

    return vec4(lighting * color, 0.);
}

// output to rasterizer
out vec4 color;

void main()
{
    // 'ambient' or base color
    vec4 ambcol = mo_ambient_color();

    // adding the light to the base color
    vec4 col = ambcol
            + getLightColor(vec3(1000., 2000., 800.), vec3(1., 1., 1.), 1.5)
            + getLightColor(vec3(-2000., -1000., 1200.), vec3(0.2, 0.5, 1.), 2.0)
            + getLightColor(vec3(2000., -500., 1500.), vec3(1., .5, .2)*0.4, 3.0);

    // final color
    color = vec4(clamp(col, 0.0, 1.0));

    //gl_DepthRangeParameters.
}
