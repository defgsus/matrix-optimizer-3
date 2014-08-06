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
uniform vec3 u_light_pos[MO_NUM_LIGHTS];
uniform vec3 u_light_color[MO_NUM_LIGHTS];


// --- output of vertex shader ---

out vec3 v_pos;
out vec4 v_color;
out vec2 v_texCoord;
out vec3 v_light_dir[MO_NUM_LIGHTS];


/** Returns the matrix for the vertex normal */
mat3 mo_light_matrix()
{
    vec3 tangent =  vec3(-a_normal.z, -a_normal.y,  a_normal.x);
    vec3 binormal = vec3(-a_normal.x,  a_normal.z, -a_normal.y);
    return mat3(tangent, -binormal, a_normal);
}


// returns spherical coordinate (x,y + depth in z)
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


// returns fulldome screen coordinate (x,y + depth in z)
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
    v_color = a_color;
    v_texCoord = a_texCoord;

    // pass light-directions to fragment shader
    mat3 lightmat = mo_light_matrix();

    for (int i=0; i<MO_NUM_LIGHTS; ++i)
        v_light_dir[i] = lightmat * normalize(u_light_pos[i] - v_pos);

    /*
    v_light_dir[0] = lightmat * normalize(vec3(1000., 2000., 800.) - v_pos);
    v_light_dir[1] = lightmat * normalize(vec3(-2000., 1000., 1200.) - v_pos);
    v_light_dir[2] = lightmat * normalize(vec3(2000., -500., 1500.) - v_pos);
    */

    // set final vertex position
    gl_Position = mo_ftransform(a_position);
}
