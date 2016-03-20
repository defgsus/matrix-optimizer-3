#version 330

// -- inputs from program --

//%user_uniforms%                       // uniform parameters are added here
                                        // (don't touch)

uniform vec4        u_resolution;       // width, height, 1./width, 1./height
uniform vec2        u_samplerate;       // hz, 1./hz
uniform float       u_time;             // scene time in seconds


// -- inputs from vertex shader --

in vec2 v_texCoord;                     // the texture uv position, range [0,1]

// -- output to framebuffer --

out vec4 fragColor;                     // color output


//%mo_user_function%

void main()
{
    vec2 pix = floor(v_texCoord * u_resolution.xy);
    float time = u_time + pix.x * u_samplerate.y;
    fragColor = mainSound(time);
}
