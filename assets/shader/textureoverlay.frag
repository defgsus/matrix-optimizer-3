#version 130

in vec4 v_texCoord;
in mat3 v_dir_matrix;

// fragment output
out vec4 color;

uniform sampler2D u_tex;
uniform vec4 u_color;
uniform float u_cam_angle;
uniform vec3 u_sphere_offset;

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;


// get spherical coordinate from screen coords [-1,1]
vec3 spherical(vec2 scr)
{
    float
        rad = length(scr),
        phi;

    if (rad == 0.0)
        phi = 0.0;
    else
    {	if (scr.x < 0.0)
            phi = PI - asin(scr.y / rad);
        else
            phi = asin(scr.y / rad);
    }

    // XXX
    float angle = u_cam_angle;// - 11.0 * abs(scr.x)*abs(scr.y);

    float
        theta = rad * angle * HALF_PI / 180.0,

        cx = cos(phi),
        cy = cos(theta),
        sx = sin(phi),
        sy = sin(theta);

    return vec3( cx * sy, sx * sy, -cy );
}

// transform spherical to cartesian coords
vec2 cartesian(vec3 pos)
{
    vec3 posn = normalize(pos);
    float u = atan(posn.y, posn.x),
          v = 2.0 * acos(-posn.z) / PI;
    return vec2(
        cos(u) * v,
        sin(u) * v
        );
}

// get texture color from xy [-1,1]
vec4 mo_texture(vec2 xy)
{
    return texture2D(u_tex, xy * 0.5 + 0.5);
}


void main(void)
{
    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

#ifdef MO_EQUIRECT
    vec3 sphere = (spherical(scr) + u_sphere_offset) * v_dir_matrix;
    scr = cartesian(sphere);

    float ang = atan(scr.x, scr.y) / PI;
    float dist = length(scr.xy);
    vec2 uv = 1.0 - vec2(ang, dist);
#else
    vec2 uv = scr;
#endif
    vec4 col = mo_texture(uv);


    color = clamp(u_color * col, 0.0, 1.0);
}
