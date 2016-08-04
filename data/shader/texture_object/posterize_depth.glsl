/* ShaderToy compatible

 uniforms:
   vec3  iResolution;              // resolution of output texture in pixels
   float iGlobalTime;              // scene time in seconds
   float iChannelTime[4];          // playback of channel in seconds (not defined yet)
   vec3  iChannelResolution[4];    // resolution per channel in pixels
   vec4  iMouse;                   // (not defined yet)
   vec4  iDate;                    // year, month, day, time in seconds
   float iSampleRate;              // sound sampling rate in Hertz
   sampler2D iChannel0-3;          // input texture
*/

vec3 posterize(in vec3 color, in vec2 pos)
{
	//float s = texture(u_scale,pos.x/1000.).x;
	float d = texture(iChannel1, v_texCoord).x;
	d = 1.-pow(d,40.);
	//return vec3(d);
	float l = length(fract(pos/(5.+25.*d))-.5);
	return vec3(step(l, color.x), step(l, color.y), step(l, color.z));
}

vec2 edge(in sampler2D tex, in vec2 pos)
{
	float d = texture(tex, pos).x
			+ texture(tex, pos+u_resolution.zw).x;
	return vec2(dFdx(d), dFdy(d));
}

vec3 depthEdge(in vec2 pos)
{
	vec2 e = edge(iChannel1, pos);
	float f = smoothstep(0., 0.0000001, dot(e,e));
	return vec3(f);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    fragColor = vec4(posterize(texture(iChannel0, fragCoord/iResolution.xy).xyz, fragCoord), 1.);
	fragColor.xyz += depthEdge(fragCoord/iResolution.xy);
}

