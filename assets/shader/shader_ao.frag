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

out vec4 _fragColor_;                   // color output


//%mo_user_function%

vec4 mo_MainSoundCast(in float _s_) { return vec4(_s_,_s_,0.,0.); }
vec4 mo_MainSoundCast(in vec2 _s_) { return vec4(_s_,0.,0.); }
vec4 mo_MainSoundCast(in vec3 _s_) { return vec4(_s_,0.); }
vec4 mo_MainSoundCast(in vec4 _s_) { return _s_; }

void main()
{
    vec2 _pix_ = floor(v_texCoord * u_resolution.xy);
    float _time_ = u_time + _pix_.x * u_samplerate.y;
    _fragColor_ = _pix_.x < float(u_buffersize)
            ? mo_MainSoundCast(mainSound(_time_)) : vec4(0.);
}
