#version 330

// --- input from vertex shader ---

in vec3 v_instance;
in vec3 v_pos;
in vec3 v_pos_world;
in vec3 v_pos_eye;
in vec3 v_cam_dir;
in vec3 v_normal;
in vec3 v_normal_eye;
in vec4 v_color;
in vec4 v_ambient_color;
in vec2 v_texCoord;
in mat3 v_normal_space;

#if MO_NUM_LIGHTS
    #ifndef MO_FRAGMENT_LIGHTING
        in vec4 v_light_dir[MO_NUM_LIGHTS];
    #endif
#endif

// --- output to rasterizer ---

out vec4 o_out_color;


// --- uniforms ---

uniform float u_time;
uniform vec3 u_cam_pos;
uniform mat4 u_projection;                  // projection matrix
uniform mat4 u_cubeViewTransform;           // cube-map * view * transform
uniform mat4 u_viewTransform;               // view * transform
uniform mat4 u_transform;                   // transformation only

uniform vec4 u_distance;
uniform vec4 u_fade_dist;                   // x=min, y=max, z=exp
uniform vec4 u_offset_scale;                // xy=offset, zw=scale

#if MO_NUM_LIGHTS
    uniform vec4 u_light_amt;
    uniform float u_light_diffuse_exp[MO_NUM_LIGHTS];
    uniform vec4 u_light_color[MO_NUM_LIGHTS];
    uniform vec3 u_light_pos[MO_NUM_LIGHTS];
    #ifdef MO_FRAGMENT_LIGHTING
        uniform vec3 u_light_direction[MO_NUM_LIGHTS];
        uniform vec3 u_light_direction_param[MO_NUM_LIGHTS]; // range min, range max, mix
    #endif
#endif

//%mo_user_uniforms%

// ------------------- defines --------------------------

//%mo_config_defines%

#define SKYBOX_AXIS_X_P 0 // positive x
#define SKYBOX_AXIS_X_N 1 // negative x
#define SKYBOX_AXIS_Y_P 2 // positive y
#define SKYBOX_AXIS_Y_N 3 // ...
#define SKYBOX_AXIS_Z_P 4
#define SKYBOX_AXIS_Z_N 5

#define SKYBOX_SHAPE_SPHERE 0
#define SKYBOX_SHAPE_PLANE 1
#define SKYBOX_SHAPE_CYLINDER 2

#define SKYBOX_CONTENT_GLSL 0
#define SKYBOX_CONTENT_TEXTURE 1

#if SKYBOX_AXIS == SKYBOX_AXIS_X_P
    const vec3 SKYBOX_AXIS_VEC = vec3( 1.,  0.,  0.);
#elif SKYBOX_AXIS == SKYBOX_AXIS_X_N
    const vec3 SKYBOX_AXIS_VEC = vec3(-1.,  0.,  0.);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_P
    const vec3 SKYBOX_AXIS_VEC = vec3( 0.,  1.,  0.);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_N
    const vec3 SKYBOX_AXIS_VEC = vec3( 0., -1.,  0.);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_P
    const vec3 SKYBOX_AXIS_VEC = vec3( 0.,  0.,  1.);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_N
    const vec3 SKYBOX_AXIS_VEC = vec3( 0.,  0., -1.);
#endif

#define SKYBOX_CENTER vec3(0.)//u_cam_pos

// ------------------- functions ------------------------

// --------------- PLANE ---------------------

vec3 skybox_plane_xyz(in vec3 axis, in vec3 dir)
{
    float t = dot(axis, axis * u_distance.x - SKYBOX_CENTER) / dot(axis, dir);
        return dir * t;
}

/** Returns the uv coordinates on the plane for view direction 'dir' */
vec2 skybox_plane_uv(in vec3 dir)
{
    vec3 pos = skybox_plane_xyz(SKYBOX_AXIS_VEC, dir);
#if SKYBOX_AXIS == SKYBOX_AXIS_X_P
    return pos.zy - .5;
#elif SKYBOX_AXIS == SKYBOX_AXIS_X_N
    return pos.zy * vec2(-1., 1.) - .5;
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_P
    return pos.xz - .5;
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_N
    return pos.xz * vec2(1., -1.) - .5;
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_P
    return pos.xy * vec2(-1., 1.) - .5;
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_N
    return pos.xy - .5;
#endif
}

// -------------- CYLINDER -------------------

/** Implementation from
    http://www.geometrictools.com/GTEngine/Include/GteIntrLine3Cylinder3.h

        This is general purpose,
        could be simplified for the case of always inside
*/
vec3 skybox_cylinder_xyz(in vec3 axis, in vec3 dir)
{
    float dz = dot(axis, dir);

    // ortogonals
    vec3 v1 = (abs(axis.x) > abs(axis.y)) ?
                            vec3(-axis.z, 0., axis.x) : vec3(0., axis.z, -axis.y);
    vec3 v2 = cross(axis, v1);
    mat3 basis = mat3(axis, v1, v2);

    float rad2 = u_distance.x * u_distance.x;

    vec3 diff = SKYBOX_CENTER;// - tube_center;
    vec3 P = vec3(dot(basis[1], diff), dot(basis[2], diff), dot(basis[0], diff));
    vec3 D = vec3(dot(basis[1], dir), dot(basis[2], dir), dz);

    float a0 = P[0] * P[0] + P[1] * P[1] - rad2;
    float a1 = P[0] * D[0] + P[1] * D[1];
    float a2 = D[0] * D[0] + D[1] * D[1];
    float dr = a1 * a1 - a0 * a2;
    float r = sqrt(dr);
    float T = a0 > 0. ? (-a1 - r) / a2 : (-a1 + r) / a2;

    return T * dir;
}

vec2 skybox_cylinder_uv(in vec3 dir)
{
    vec3 pos = skybox_cylinder_xyz(SKYBOX_AXIS_VEC, dir);
#if SKYBOX_AXIS == SKYBOX_AXIS_X_P
    return vec2(atan(-pos.y, -pos.z)/6.283185307+.5,  pos.x - .5);
#elif SKYBOX_AXIS == SKYBOX_AXIS_X_N
    return vec2(atan( pos.y, -pos.z)/6.283185307+.5, -pos.x + .5);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_P
    return vec2(atan( pos.x, -pos.z)/6.283185307+.5,  pos.y - .5);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_N
    return vec2(atan(-pos.x, -pos.z)/6.283185307+.5, -pos.y + .5);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_P
    return vec2(atan( pos.x,  pos.y)/6.283185307+.5,  pos.z - .5);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_N
    return vec2(atan(-pos.x,  pos.y)/6.283185307+.5, -pos.z + .5);
#endif
}

// --------- SPHERE -----------------

vec3 skybox_sphere_xyz(in vec3 dir)
{
    vec3 oc = /*sphere.xyz*/ - SKYBOX_CENTER;
    float oc2 = dot(oc, oc);
    float closest = dot(oc, dir);
    float radius2 = u_distance.x * u_distance.x;
    if (oc2 >= radius2 && closest < 0.000001) discard;
    float halfc2 = radius2 - oc2 + closest * closest;
    if (halfc2 < 0.000001) discard;
    float halfc = sqrt(halfc2);
    float T = oc2 < radius2 ? closest + halfc : closest - halfc;
        return T * dir;
}

vec2 skybox_sphere_uv(in vec3 dir)
{
    vec3 pos = skybox_sphere_xyz(dir) / u_distance.x;
#if SKYBOX_AXIS == SKYBOX_AXIS_X_P
    return vec2(atan(-pos.y, -pos.z)/6.283185307+.5, acos(-pos.x)/3.14159265);
#elif SKYBOX_AXIS == SKYBOX_AXIS_X_N
    return vec2(atan( pos.y, -pos.z)/6.283185307+.5, acos( pos.x)/3.14159265);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_P
    return vec2(atan( pos.x, -pos.z)/6.283185307+.5, acos(-pos.y)/3.14159265);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Y_N
    return vec2(atan(-pos.x, -pos.z)/6.283185307+.5, acos( pos.y)/3.14159265);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_P
    return vec2(atan( pos.x,  pos.y)/6.283185307+.5, acos(-pos.z)/3.14159265);
#elif SKYBOX_AXIS == SKYBOX_AXIS_Z_N
    return vec2(atan(-pos.x,  pos.y)/6.283185307+.5, acos( pos.z)/3.14159265);
#endif
}

// ----- shape as defined ------

vec3 skybox_xyz(in vec3 dir)
{
#if SKYBOX_SHAPE == SKYBOX_SHAPE_PLANE
    return skybox_plane_xyz(SKYBOX_AXIS_VEC, dir);
#elif SKYBOX_SHAPE == SKYBOX_SHAPE_CYLINDER
    return skybox_cylinder_xyz(SKYBOX_AXIS_VEC, dir);
#else
    return skybox_sphere_xyz(dir);
#endif
}

vec2 skybox_uv(in vec3 dir)
{
#if SKYBOX_SHAPE == SKYBOX_SHAPE_PLANE
    vec2 uv = skybox_plane_uv(dir);
#elif SKYBOX_SHAPE == SKYBOX_SHAPE_CYLINDER
    vec2 uv = skybox_cylinder_uv(dir);
#else
    vec2 uv = skybox_sphere_uv(dir);
#endif
    return uv * u_offset_scale.zw + u_offset_scale.xy;
}

// ----- texture handling -----

/** Returns planar sampler2D */
vec4 skybox_texturePlane(in vec2 uv) { return texture(u_tex_color, uv); }


//%mo_user_function%


void main()
{
    vec3 direction = normalize(v_pos_world);

    // clipping for plane
#if SKYBOX_SHAPE == SKYBOX_SHAPE_PLANE
    #if SKYBOX_AXIS == SKYBOX_AXIS_X_P
        if (direction.x <= 0.) discard;
    #elif SKYBOX_AXIS == SKYBOX_AXIS_X_N
        if (direction.x >= 0.) discard;
    #elif SKYBOX_AXIS == SKYBOX_AXIS_Y_P
        if (direction.y <= 0.) discard;
    #elif SKYBOX_AXIS == SKYBOX_AXIS_Y_N
        if (direction.y >= 0.) discard;
    #elif SKYBOX_AXIS == SKYBOX_AXIS_Z_P
        if (direction.z <= 0.) discard;
    #elif SKYBOX_AXIS == SKYBOX_AXIS_Z_N
        if (direction.z >= 0.) discard;
    #endif
#endif


    vec3 pos = skybox_xyz(direction);
    vec2 uv = skybox_uv(direction);

    #if SKYBOX_CONTENT == SKYBOX_CONTENT_GLSL
        mainImage(o_out_color, direction, pos, uv);
    #else SKYBOX_CONTENT == SKYBOX_CONTENT_TEXTURE
        o_out_color = skybox_texturePlane(uv);
    #endif

#ifdef SKYBOX_ENABLE_FADE
    // distance fade-out
    float d = length(pos);
    o_out_color.a *= pow(smoothstep(u_fade_dist.y, u_fade_dist.x, d), u_fade_dist.z);
#endif

    o_out_color *= v_ambient_color;
}
