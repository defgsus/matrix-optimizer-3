#version 330

in vec4 v_texCoord;
in mat3 v_dir_matrix;

// fragment output
out vec4 color;

uniform sampler2D u_tex;
uniform vec4 u_color;
uniform float u_cam_angle;
uniform float u_cube_hack; // XXX
uniform vec3 u_sphere_offset;

#ifdef MO_FLAT
uniform mat4 u_local_transform;
#endif

#ifdef MO_POST_PROCESS
uniform vec3 u_post_transform; // grayscale, invert, +/-shift
uniform vec3 u_post_bright; // brightness, contrast, threshold
uniform vec4 u_post_alpha; // rgb color and to-alpha amount
uniform float u_post_alpha_edge;
#endif

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
    //float angle = u_cam_angle - 11.0 * abs(scr.x)*abs(scr.y);
    float rad2 = mix(rad, pow(rad,0.59), u_cube_hack * smoothstep(0,1,rad));

    float
        theta = rad2 * u_cam_angle * HALF_PI / 180.0,

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
    return texture(u_tex, xy * 0.5 + 0.5);
}

vec4 mo_color(vec2 xy)
{
    vec4 col = mo_texture(xy);

    return col;
}


void main(void)
{
    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

    // -------- equi-rect projection ------------
#ifdef MO_EQUIRECT
    vec3 sphere = (spherical(scr) + u_sphere_offset) * v_dir_matrix;
    scr = cartesian(sphere);

    float ang = atan(scr.x, scr.y) / PI;
    float dist = length(scr.xy);
    vec2 uv = 1.0 - vec2(ang, dist);

    vec4 col = mo_color(uv);
#endif

#ifdef MO_FISHEYE
    vec3 sphere = (spherical(scr) + u_sphere_offset) * v_dir_matrix;
    vec4 col1 = mo_color( cartesian(sphere) );
    sphere.z = -sphere.z;
    vec4 col2 = mo_color( cartesian(sphere) );

    vec4 col = mix(col2, col1, smoothstep(-0.1,0.1, sphere.z));
#endif

    // ------- flat on screen -------------------
#ifdef MO_FLAT
    vec4 newpos = u_local_transform * vec4(scr, 0.0, 1.0);
    vec2 uv = newpos.xy;

    vec4 col = mo_color(uv);
#endif



#ifdef MO_POST_PROCESS
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

#endif

    color = clamp(u_color * col, 0.0, 1.0);
}
