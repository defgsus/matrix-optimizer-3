#version 130

const float PI = 3.14159265358979;

//#define MO_FULLDOME_BEND

// vertex attributes
in vec4 a_position;
in vec4 a_color;
in vec3 a_normal;
in vec2 a_texCoord;

// shader uniforms
uniform mat4 u_projection;
uniform mat4 u_view;

// output of vertex shader
out vec3 v_pos;
out vec4 v_color;
out vec3 v_normal;
out vec2 v_texCoord;

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
    v_normal = a_normal;
    v_texCoord = a_texCoord;

    // set final vertex position
    gl_Position = mo_ftransform(a_position);
}
