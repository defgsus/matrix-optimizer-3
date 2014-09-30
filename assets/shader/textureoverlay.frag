#version 330

in vec4 v_texCoord;
in mat3 v_dir_matrix;

// fragment output
out vec4 color;

uniform sampler2D u_tex;
uniform vec4 u_color;
uniform float u_cam_angle;
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

    float
        theta = rad * u_cam_angle * HALF_PI / 180.0,

        cx = cos(phi),
        cy = cos(theta),
        sx = sin(phi),
        sy = sin(theta);

    return vec3( cx * sy, sx * sy, -cy );
}

// transform spherical to cartesian coords
vec2 sphere_surface_to_2d_fisheye(vec3 pos)
{
    vec3 posn = normalize(pos);
    float u = atan(posn.y, posn.x),
          v = 2.0 * acos(-posn.z) / PI;
    return vec2(
        cos(u) * v,
        sin(u) * v
        );
}

/*
 *  transform spherical coordinates int cartesian coordinates
 *  on a unit sphere
 */
vec3 spherical_mh(vec2 scr)
{
    vec2 lscr = vec2 (PI*(scr.x/2.0+0.5), PI*(scr.y+1.0));

    float
        cx = cos(lscr.x),
        cy = cos(lscr.y),
        sx = sin(lscr.x),
        sy = sin(lscr.y);

    return vec3( cx * cy, cx * sy, sx );
}

/*
 * transform cartesian coordinates on a unit sphere
 * to spherical coordinates
 */
vec2 cartesian_mh(vec3 pos)
{
    float
        theta = asin(pos.z),
        phi   = atan(pos.y,pos.x);
    if(pos.x == 0 && pos.y == 0) phi = 0;
    return vec2(
        ((theta/PI)*2-1),
        (phi/PI)
        );
}

vec2 sphere_transform(vec2 xy)
{
    float
        r_squared,
        sph_squared,
        uso_squared,
        sph_times_uso,
        t;

    vec3
        sphere = spherical_mh(xy),
        sphere_offset = u_sphere_offset * v_dir_matrix;

    r_squared = dot(sphere,sphere);
    sph_squared = r_squared;
    uso_squared = dot(sphere_offset,sphere_offset);
    sph_times_uso = dot(sphere,sphere_offset);
    t = (-sph_times_uso + sqrt(r_squared*sph_squared + sph_times_uso*sph_times_uso - uso_squared*sph_squared))/sph_squared;

    vec3 new_sphere = sphere_offset + t*sphere;
    return cartesian_mh(new_sphere);
}


// get texture color from xy [-1,1]
vec4 mo_texture(vec2 xy)
{
    //return texture(u_tex, xy * 0.5 + 0.5);
    return texture(u_tex, sphere_transform(xy) * 0.5 + 0.5);
}

vec4 mo_color(vec2 xy)
{
    vec4 col = mo_texture(xy);

    return col;
}

float beta(in vec2 v) { return sqrt(1. -v.x*v.x - v.y*v.y); }

void main(void)
{
    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

    // -------- equi-rect projection ------------
#ifdef MO_EQUIRECT

    float fac = tan((u_cam_angle / 360.0)*PI);
    // XXX u_sphere_offset not working correctly
    vec3 cube = (vec3(scr * fac, -1.) + u_sphere_offset) * v_dir_matrix;

    scr = sphere_surface_to_2d_fisheye(cube);

    float ang = atan(scr.x, scr.y) / PI;
    float dist = length(scr.xy);
    vec2 uv = 1.0 - vec2(ang, dist);

    vec4 col = mo_color(uv);
#endif

#ifdef MO_FISHEYE
    // XXX
    float fac = tan((u_cam_angle / 360.0)*PI);
    vec3 cube = (vec3(scr * fac, -1.) + u_sphere_offset) * v_dir_matrix;

    vec4 col1 = mo_color( sphere_surface_to_2d_fisheye(cube) );
    cube.z = -cube.z;
    vec4 col2 = mo_color( sphere_surface_to_2d_fisheye(cube) );

    vec4 col = mix(col2, col1, smoothstep(-0.1,0.1, cube.z));

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
