#version 330

// -- inputs from program --

//%user_uniforms%                       // uniform parameters are added here
                                        // (don't touch)

uniform vec4        u_resolution;       // width, height, 1./width, 1./height
uniform float       u_time;             // scene time in seconds
uniform mat4        u_transformation;   // object's own transformation matrix
                                        // (including parents)
uniform sampler2D   u_feedback;         // the previous output frame
uniform vec4        u_pass;             // current render pass
                                        //   x = current pass number
                                        //   y = current pass [0,1)
                                        //   z = current pass [0,1]
                                        //   w = number passes


// -- inputs from vertex shader --

in vec2 v_texCoord;                     // the texture uv position, range [0,1]
in vec2 v_pos;                          // the pixel position, normalized to [-1,1]
in vec2 v_screen;                       // the pixel position, true resolution
                                        //                      [0, width/height)

// -- output to framebuffer --

out vec4 fragColor;                     // color output

#include <constants>

void main()
{
    // generate color from pixel coordinate
    vec3 col = vec3(.5+.5*sin(v_pos.x*PI + u_time),
                    .5+.5*sin(v_pos.y*PI + u_time),
                    .5+.5*sin(v_pos.y*PI));

    // write to framebuffer (with alpha)
    fragColor = vec4(col, 1.);
}
