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

uniform mat4 u_projection;  // projection matrix
uniform mat4 u_view;        // to-eye-space matrix
uniform mat4 u_transform;   // transformation only
uniform vec3 u_light_pos[MO_NUM_LIGHTS];
uniform vec4 u_light_color[MO_NUM_LIGHTS];


// --- output of vertex shader ---

out vec3 v_pos;
out vec3 v_pos_eye;
out vec4 v_color;
out vec2 v_texCoord;
// w is distance attenuation
out vec4 v_light_dir[MO_NUM_LIGHTS];


/** Returns the matrix to multiply the light-direction normal */
mat3 mo_light_matrix()
{
    vec3 norm = transpose(inverse(mat3(u_transform))) * a_normal;

    vec3 tangent =  vec3(-norm.z, -norm.y,  norm.x);
    vec3 binormal = vec3(-norm.x,  norm.z, -norm.y);
    return mat3(tangent, -binormal, norm);
}


/** returns spherical coordinate (x,y + depth in z) */
vec3 mo_pos_to_fulldome(in vec3 pos)
{
    vec3 posn = normalize(pos);

    float
        d = length(pos),
        phi = atan(pos.y, pos.x),
        v = 2.0 * acos(-posn.z) / PI;// * 180.0 / u_camera_angle;

    return vec3(
        cos(phi) * v,
        sin(phi) * v,
        d
        );
}


/** returns fulldome screen coordinate (x,y + depth in z) */
vec4 mo_pos_to_fulldome_scr(in vec3 pos)
{
    vec3 posn = normalize(pos);

    float
        d = length(pos),
        phi = atan(pos.y, pos.x),
        v = 2.0 * acos(-posn.z) / PI;// * 180.0 / u_camera_angle;

    return vec4(
        cos(phi) * v,
        sin(phi) * v,
        (posn.z>0.0 && v>1.0) ? d : 0.0,
        1.0
        );
}

vec4 mo_ftransform(in vec4 pos)
{
#ifndef MO_FULLDOME_BEND
    return u_projection * u_view * pos;
#else
    return mo_pos_to_fulldome_scr((u_view * pos).xyz);
#endif
}


void main()
{
    // pass attributes to fragment shader
    v_pos = a_position.xyz;
    v_pos_eye = (u_transform * a_position).xyz;
    v_color = a_color;
    v_texCoord = a_texCoord;

    // set final vertex position
    gl_Position = mo_ftransform(a_position);


    // pass light-directions to fragment shader

    mat3 lightmat = mo_light_matrix();

    for (int i=0; i<MO_NUM_LIGHTS; ++i)
    {
        vec3 lightvec = u_light_pos[i] - v_pos_eye;
        float dist = length(lightvec);
        // calculate influence from distance attenuation
        float distatt = 1.0 / (1.0 + u_light_color[i].w * dist * dist);

        v_light_dir[i] = vec4(lightmat * (lightvec / dist), distatt);
    }
}
