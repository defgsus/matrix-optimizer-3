#version 330

// -- inputs from program --

//%user_uniforms%                       // uniform parameters are added here
                                        // (don't touch)

uniform vec4        u_resolution;       // width, height, 1./width, 1./height
uniform vec2        u_samplerate;       // hz, 1./hz
uniform int         u_buffersize;       // size of one dsp-block
uniform float       u_time;             // scene time in seconds
// shadertoy compatibility
uniform float       iSampleRate;        // hz

// -- inputs from vertex shader --

in vec2 v_texCoord;                     // the texture uv position, range [0,1]

// -- output to framebuffer --

out vec4 fragColor;                     // color output


//%mo_user_function%

vec4 mo_MainSoundCast(in float s) { return vec4(s,s,0.,0.); }
vec4 mo_MainSoundCast(in vec2 s) { return vec4(s,0.,0.); }
vec4 mo_MainSoundCast(in vec3 s) { return vec4(s,0.); }
vec4 mo_MainSoundCast(in vec4 s) { return s; }

void main()
{
    vec2 pix = floor(v_texCoord * u_resolution.xy);
    float time = u_time + pix.x * u_samplerate.y;
    fragColor = pix.x < float(u_buffersize)
            ? mo_MainSoundCast(mainSound(time)) : vec4(0.);
}
