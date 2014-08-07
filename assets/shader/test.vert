#version 130

#define MO_NUM_LIGHTS 3

const float PI = 3.14159265358979;

/* defines
 * MO_ENABLE_TEXTURE
 * MO_ENABLE_NORMALMAP
 */

//#define MO_FULLDOME_BEND

// --- vertex attributes ---

in vec4 a_position;
in vec4 a_color;
in vec3 a_normal;
in vec2 a_texCoord;


// --- shader uniforms ---

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_orgView;
uniform vec3 u_light_pos[MO_NUM_LIGHTS];
uniform vec3 u_light_color[MO_NUM_LIGHTS];


// --- output of vertex shader ---

out vec4 v_color;


#if (1)
void main()
{
    mat3 normal_matrix = transpose(inverse(mat3(u_orgView)));

    vec3 norm_eye = normalize( normal_matrix * a_normal );

    vec4 pos_eye = u_orgView * a_position;

    vec4 light = vec4(u_light_pos[0], 1.);
    vec4 light_eye = u_view * vec4(u_light_pos[0], 1.);
    vec3 light_dir = normalize(vec3(light - pos_eye));

    float l = pow(max(0., dot(light_dir, norm_eye)), 5.0);

    v_color = vec4(0.5+l, 0.5+l, 0.5+l, 1.0);

    gl_Position = u_projection * u_view * a_position;
}

#else

void main()
{
    vec3 cam_pos = -u_orgView[3].xyz;

    mat3 normal_matrix = transpose(inverse(mat3(u_orgView)));

    vec3 norm_eye = normalize( normal_matrix * a_normal );

    //vec4 pos_eye = u_view * a_position;
    vec3 pos_eye = a_position.xyz - cam_pos;

    vec3 light = u_light_pos[0];
    vec3 light_eye = u_light_pos[0] - cam_pos;
    //vec3 light_dir = normalize(vec3(light_eye - pos_eye));
    vec3 light_dir = normalize(light - pos_eye);

    float l = pow(max(0., dot(light_dir, norm_eye)), 5.0);

    vec3 col = norm_eye*0.5+0.5 + l;

    v_color = vec4(col, 1.0);

    gl_Position = u_projection * u_view * a_position;
}
#endif
