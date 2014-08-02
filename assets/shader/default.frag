#version 130

// input from vertex shader
in vec3 v_pos;
in vec4 v_color;
in vec3 v_normal;

// lightsource
/*
const vec3 u_light_pos = vec3(1000.0, 2000.0, 800.0);
const vec3 u_light_color = vec3(1.0, 1.0, 1.0);
const float u_shinyness = 2.0;
*/

vec3 getLightColor(in vec3 light_pos, in vec3 color, in float shinyness)
{
    // normal to light source
    vec3 light_normal = normalize( light_pos - v_pos );
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(v_normal, light_normal) );
    // shaping the linear light influence
    float lighting = pow(d, 1.0 + shinyness);

    return lighting * color;
}

// output to rasterizer
out vec4 color;

void main()
{
    // 'ambient' or base color
    vec3 ambcol = v_color.xyz;

    // adding the light to the base color
    vec3 col = ambcol
            + getLightColor(vec3(1000., 2000., 800.), vec3(1., 1., 1.), 1.5)
            + getLightColor(vec3(-2000., -1000., 1200.), vec3(0.2, 0.5, 1.), 2.0)
            + getLightColor(vec3(2000., -500., 1500.), vec3(1., .5, .2)*0.4, 3.0);

    // final color
    color = vec4(clamp(col, 0.0, 1.0), 1.0);

    //gl_DepthRangeParameters.
}
