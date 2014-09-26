#version 330

in vec4 v_texCoord;
in mat3 v_dir_matrix;

// fragment output
out vec4 color;

uniform sampler2D u_tex;
uniform vec4 u_color;
uniform float u_cam_angle;
uniform float u_is_cube;
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

// http://stackoverflow.com/questions/2656899/mapping-a-sphere-to-a-cube

vec2 cube_map(in vec2 v)
{
    float l = length(v);
    return v*sqrt(2.0);

    /*
    float a2 = v.x * v.x;
    float b2 = v.y * v.y;
    float inner1 = 2.0 * a2 - 2.0 * b2 - 3.0;
    float inner2 = -sqrt(inner1 * inner1 - 24.0 * a2);
    return vec2(
                sqrt(inner2 + 2.0 * a2 - 2.0 * b2 + 3.0),
                sqrt(inner2 - 2.0 * a2 + 2.0 * b2 + 3.0)
                ) / sqrt(2.0)
//            * vec2(v.x>0.0? 1. : -1, v.y>0.0? 1. : -1.)
            ;
*/
    //s = sqrt(-sqrt((2 a^2-2 b^2-3)^2-24 a^2)+2 a^2-2 b^2+3)/sqrt(2)
    //t = sqrt(-sqrt((2 a^2-2 b^2-3)^2-24 a^2)-2 a^2+2 b^2+3)/sqrt(2)
}


// transforms a 3d vector on a unit-sphere surface to a unit-cube position
// XXX not working
vec3 to_cube(in vec3 s)
{
    float beta = sqrt(1.0 - (s.x*s.x - s.y*s.y));
    return vec3(s.x, s.y, s.z+beta);

    /*
    vec3 as = abs(s);

    if (as.z > as.x && as.z > as.y)
        return vec3(cube_map(s.xy), s.z > 0.0? 1.0 : -1.0);
    if (as.x > as.y && as.x > as.z)
        return vec3(s.x > 0.0? 1.0 : -1.0, cube_map(s.yz));
    if (as.y > as.x && as.y > as.z)
    {
        vec2 c = cube_map(s.xz);
        return vec3(c.x, s.y > 0.0? 1.0 : -1.0, c.y);
    }

    return s;
    */
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

float beta(in vec2 v) { return sqrt(1. -v.x*v.x - v.y*v.y); }

void main(void)
{
    vec2 scr = v_texCoord.xy * 2.0 - 1.0;

    // -------- equi-rect projection ------------
#ifdef MO_EQUIRECT

    vec3 sphere = spherical(scr) + u_sphere_offset;

    // XXX u_sphere_offset not working correctly
    vec3 cube = //to_cube(normalize(sphere));
                //normalize(sphere * vec3(1.,1.,beta(sphere.xy)));
                clamp(vec3(scr, -1.) + u_sphere_offset, -1., 1.0);

    vec3 surface = mix(sphere, cube, u_is_cube) * v_dir_matrix;

    scr = cartesian(surface);

    float ang = atan(scr.x, scr.y) / PI;
    float dist = length(scr.xy);
    vec2 uv = 1.0 - vec2(ang, dist);

    vec4 col = mo_color(uv);
    /*
    {
        vec3 cube = clamp(vec3(v_texCoord.xy * 2.0 - 1.0, -1.) + u_sphere_offset, -1., 1.0);

        scr = cartesian(cube * v_dir_matrix);

        float ang = atan(scr.x, scr.y) / PI;
        float dist = length(scr.xy);
        vec2 uv = 1.0 - vec2(ang, dist);

        col = (col + mo_color(uv)) * 0.5;
    }*/
#endif

#ifdef MO_FISHEYE
    vec3 sphere = spherical(scr) + u_sphere_offset;

    vec3 cube = vec3(scr, -1.) + u_sphere_offset;

    vec3 surface = mix(sphere, cube, 0.000001*u_is_cube) * v_dir_matrix;

    vec4 col1 = mo_color( cartesian(surface) );
    surface.z = -surface.z;
    vec4 col2 = mo_color( cartesian(surface) );

    vec4 col = mix(col2, col1, smoothstep(-0.1,0.1, sphere.z));

    {
        vec3 cube = clamp(vec3(v_texCoord.xy * 2.0 - 1.0, -1.) + u_sphere_offset, -1., 1.0);

        scr = cartesian(cube * v_dir_matrix);

        col = (col + mo_color(scr)) * 0.5;
    }
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
