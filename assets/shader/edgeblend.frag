#version 330

in vec4 v_texCoord;
out vec4 fragColor;

uniform int u_index;
uniform float u_dome_radius;
uniform float u_margin;
uniform vec2  u_nearFar[MO_NUM_PROJECTORS];
uniform mat4  u_projection[MO_NUM_PROJECTORS];
uniform mat4  u_inverseProjection[MO_NUM_PROJECTORS];
uniform mat4  u_view[MO_NUM_PROJECTORS];
uniform mat4  u_inverseView[MO_NUM_PROJECTORS];

const float PI = 3.14159265358979;
const float HALF_PI = 1.5707963268;

const float MULT = 1.0;

// implementation after povray source :)
bool intersect_ray_sphere(in vec3 ray_origin,
                          in vec3 ray_direction,
                          in vec3 sphere_center,
                          float sphere_radius,
                          out float depth1,
                          out float depth2)
{
    vec3 origin_to_center = sphere_center - ray_origin;

    float oc_squared = dot(origin_to_center, origin_to_center);

    float closest = dot(origin_to_center, ray_direction);

    float radius2 = sphere_radius * sphere_radius;

    if (oc_squared >= radius2 && closest < 0.00001)
        return false;

    float half_chord2 = radius2 - oc_squared + closest * closest;

    if (half_chord2 > 0.00001)
    {
        float half_chord = sqrt(half_chord2);

        depth1 = closest + half_chord;
        depth2 = closest - half_chord;

        return true;
    }

    return false;
}





void get_ray(in int index, in vec2 slice, out vec3 pos, out vec3 dir)
{
    vec4 p = u_inverseProjection[index] * vec4(slice, -u_nearFar[index].x, 1.);

    vec4 dirf = u_inverseProjection[index] * vec4(slice, -u_nearFar[index].y, 1.);

    pos = vec3(u_view[index] * (p / p.w));
    dir = vec3(u_view[index] * vec4(normalize(dirf.xyz / dirf.w), 0.));
}

vec3 map_to_dome(in int index, in vec2 slice)
{
    vec3 pos, dir;
    get_ray(index, slice, pos, dir);

    float depth1, depth2;
    if (!intersect_ray_sphere(pos, dir, vec3(0,0,0), u_dome_radius, depth1, depth2))
    {
        // outside dome...
        return pos + dir * 0.001;
    }

    return pos + dir * depth1;
}


vec2 map_from_dome(in int index, in vec3 pdome)
{
    // project into projector's view space
    vec4 pscr = u_projection[index] * u_inverseView[index] * vec4(pdome, 1.);

    return pscr.xy / pscr.w;
}

float inside_distance(vec2 slice)
{
    return float(abs(slice.x)<=1) * float(abs(slice.y)<=1) * min(1-abs(slice.x),1-abs(slice.y));
}

bool inside(vec2 slice)
{
    return abs(slice.x) < 1. && abs(slice.y) < 1.;
}

float sqr(float x) { return x*x; }

const float EPS = 0.0001;

float white_mh(in vec2 slice)
{
    float black = 0.0;

    // own pixel on dome surface
    vec3 pdome = map_to_dome(u_index, slice);

    int num_shared = 0;
    for(int i=0; i< MO_NUM_PROJECTORS; ++i) {
        vec2 oslice = map_from_dome(i, pdome);
        if(inside(oslice))
            ++num_shared;
    }

    int num_covered = 0;
    for (int i = 0; i < u_index; ++i) {
        vec2 oslice = map_from_dome(i, pdome);
        if(inside(oslice))
            ++num_covered;
    }

    bool  pure_slice = inside(slice);
    for (int i = 0; i < MO_NUM_PROJECTORS; ++i)
    if (i != u_index)
    {
        vec2 oslice = map_from_dome(i, pdome);
        if(inside(oslice)) pure_slice = false;
    }
    float final_outer_section = pure_slice ? 1.0 : 0.0;

    for (int i = 0; i < MO_NUM_PROJECTORS; ++i)
    if (i != u_index)
    {
        //float edged = min(1. - abs(slice.x), 1. - abs(slice.y));

        // others pixel in slice-space [-1,1]
        vec2 oslice = map_from_dome(i, pdome);

        float oedged = inside_distance(oslice);
        float edged  = inside_distance(slice);

        float black_so_far = 0.0;
        float intersection_color = (1.0 / (float(num_shared) + 3.0*(float(num_shared-2))));
        float max_color = intersection_color * (float(num_covered+1) /*+ 3.0*(float(num_covered-1))*/);
        float min_color = intersection_color * (float(num_covered));
        float inner_section = ((edged >= u_margin) && (oedged >= u_margin))
                ? intersection_color
                : 0.0;
        float outer_section =  ((edged <= 1.0) && (oedged == 0.0))
                ? 1.0
                : 0.0; //intersection_color;
        float blending_out  = (((edged == 0.0) || (oedged >= u_margin)) &&
                               ((edged <= u_margin) || (oedged <= u_margin)) &&
                               !((edged < u_margin) && (oedged < u_margin)))
                ? clamp(intersection_color * (edged / u_margin), 0.0, 1.0 )
                : 0.0;
        float blending_in   = ((oedged > 0.0) && (oedged <= u_margin) && (edged > 0.0) &&
                               !((edged <= u_margin) && (oedged <= u_margin)))
                ? clamp(intersection_color * (1.0 - oedged / u_margin) + intersection_color, 0.0, 1.0)
                : 0.0;
//        float rest_in       = ((edged <= u_margin) && (oedged <= u_margin) &&
//                               (oedged > 0.0) && (u_index < i))
//                ? clamp(intersection_color * (edged / u_margin), 0.0, 1.0)
//                : 0.0;
        float rest_in_1     = ((edged <= u_margin) && (oedged <= u_margin) && (oedged >= edged) &&
                               (oedged > 0.0) && (u_index < i))
                ? clamp(intersection_color * (edged / u_margin), 0.0, 1.0)
                : 0.0;
        float rest_in_2     = ((edged <= u_margin) && (oedged <= u_margin) && (oedged <= edged) &&
                               (oedged > 0.0) && (u_index < i))
                ? clamp(intersection_color * (edged/u_margin) + intersection_color * ((edged / u_margin) - ( oedged / u_margin)), 0.0, 1.0)
                : 0.0;
//        float rest_out      = ((edged <= u_margin) && (oedged <= u_margin) &&
//                               (oedged > 0.0) && (u_index > i))
//                ? clamp(intersection_color * (1.0 - oedged / u_margin) + intersection_color, 0.0, 1.0)
//                : 0.0;
        float rest_out_1    = ((edged <= u_margin) && (oedged <= u_margin) && (oedged <= edged) &&
                               (oedged > 0.0) && (u_index > i))
                ? clamp(max_color /*+ max_color*(1.0-edged/u_margin)*/ - intersection_color * (oedged / u_margin), 0.0, 1.0)
                : 0.0;
        float rest_out_2    = ((edged <= u_margin) && (oedged <= u_margin) && (oedged >= edged) &&
                               (oedged > 0.0) && (u_index > i))
                ? clamp(intersection_color * (edged / u_margin) + max_color*(1.0-oedged / u_margin), 0.0, 1.0)
                : 0.0;
        black_so_far = ( blending_out + blending_in +
                        /*rest_in  +*/ rest_in_1 + rest_in_2 +
                        /*rest_out +*/ rest_out_1 + rest_out_2 +
                        inner_section
                        );
        black_so_far = clamp(black_so_far, 0.0, 1.0);

        //Version 1
        //bool oinside = abs(oslice.x) < 1. && abs(oslice.y) < 1.;
        //white = max(0, white - (oinside ? 0.5 : 0));

        black += black_so_far;
    }

    //black = clamp(black, 0.0, 1.0) ;
    black += clamp(final_outer_section, 0.0, 1.0);
    //black = clamp(black,0.0,1.0);// - ((MULT +0.1)/2)*(float(num_shared-2)));

    //return smoothstep(0, 0.2, edged);
    return black;
}



float white_sb(in vec2 slice)
{
    // own pixel on dome surface
    vec3 pdome = map_to_dome(u_index, slice);

    int num_shared = 0;
    for (int i=0; i< MO_NUM_PROJECTORS; ++i)
    {
        vec2 oslice = map_from_dome(i, pdome);
        if(inside(oslice))
            ++num_shared;
    }

    float intersection_color = 1.0 / float(num_shared);

    float white = 1.0;

    for (int i=0; i < MO_NUM_PROJECTORS; ++i)
    if (i != u_index)
    {
        vec2 oslice = map_from_dome(i, pdome);

        float oedged = inside_distance(oslice);
        float edged  = inside_distance(slice);

        float oedged1 = smoothstep(0.0, u_margin, oedged);

        white -= oedged1 * intersection_color;
    }

    return white;
}

void main(void)
{
#if MO_BLEND_METHOD == 0
    float w = white_sb(v_texCoord.xy * 2. - 1.);
    fragColor = vec4(w,w,w,1.);
#elif MO_BLEND_METHOD == 1
    float b = white_mh(v_texCoord.xy * 2. - 1.);
    fragColor = vec4(b,b,b,1.);
#endif
}
