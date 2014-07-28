#version 130

// input from vertex shader
in vec3 v_pos;
in vec4 v_color;
in vec3 v_normal;

// lightsource
const vec3 u_light_pos = vec3(1000.0, 2000.0, -500.0);
const vec3 u_light_color = vec3(1.0, 1.0, 1.0);
const float u_shinyness = 2.0;


// output to rasterizer
out vec4 color;

void main()
{
    // 'ambient' or base color
    vec3 ambcol = v_color.xyz;

    // normal to light source
    vec3 light_normal = normalize( u_light_pos - v_pos );
    // dot-product of light normal and vertex normal gives linear light influence
    float d = max(0.0, dot(v_normal, light_normal) );
    // shaping the linear light influence
    float lighting = pow(d, 1.0 + u_shinyness);
    // adding the light to the base color
    vec3 col = ambcol + lighting * u_light_color;

    // final color
    color = vec4(clamp(col, 0.0, 1.0), 1.0);
}
