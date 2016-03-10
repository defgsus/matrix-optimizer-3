#version 330

uniform     float       u_time;
uniform     vec4        u_resolution;       // <xres,yres,1/xres,1/yres>
uniform     vec4        u_color_range_min;
uniform     vec4        u_color_range_max;

#ifdef MO_VERTEX

    uniform     mat4        u_viewTransform;

    in          vec4        a_position;
    in          vec4        a_texCoord;

    out         vec2        v_texCoord;
    out         vec2        v_pixelCoord;

#endif

#ifdef MO_FRAGMENT

    in          vec2        v_texCoord;
    in          vec2        v_pixelCoord;

    out         vec4        fragColor;

    //%mo_texture_decl%

    vec4 mo_color_range(in vec4 c)
    {
        return max(u_color_range_min, min(u_color_range_max, c ));
    }

#endif
