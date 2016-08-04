// Please be aware that this interface is likely to change in the future!

// You have access to these values (! means: if available):
// -- uniforms --
// float u_time
// vec3 u_cam_pos
// float u_bump_scale
// sampler2D tex_0 !
// sampler2D tex_norm_0 !
// -- input from vertex stage --
// vec3 v_instance
// vec3 v_pos
// vec3 v_pos_world
// vec3 v_pos_eye
// vec3 v_normal
// vec3 v_normal_eye
// mat3 v_normal_space
// vec3 v_texCoord
// vec3 v_cam_dir
// vec4 v_color
// vec4 v_ambient_color
// -- lighting --
// vec3 mo_normal
// vec4 mo_light_color
// ... todo
// -- output to rasterizer --
// vec4 out_color

/** Distance to a box around origin, with extends given by @p b */
float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

/** Distance to a rounded box around origin, with extends given by @p b, and
    roundness radius in @p r. */
float udRoundBox( vec3 p, vec3 b, float r )
{
    return length(max(abs(p)-b,0.0)) - r;
}

/** Distance to a cylinder with round caps, end-points in @p a and @p b, radius in @p r */
float sdCapsule( vec3 p, vec3 a, vec3 b, vec2 r )
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - mix(r.x,r.y,h);
}


in vec3 v_center;
in vec3 v_size;

float DE_bound(in vec3 p)
{
	return sdBox(p, v_size*1.001);
}

float DE(in vec3 p)
{
	//return sdCapsule(p, vec3(-.3), vec3(.3), vec2(.2,.1));
	float rn = min(v_size.x, min(v_size.y, v_size.z)) / 5.;
	float d = udRoundBox(p, v_size/2.-rn, rn);
	vec3 dis = .5+.5*sin(p*v_size*2.);
	d += .4*(dis.x*dis.y*dis.z);
	return d;
}

vec3 DE_norm(in vec3 p, in float eps)
{
	vec2 e = vec2(eps, 0.);
	return normalize(vec3(
			 DE(p+e.xyy) - DE(p-e.xyy),
			 DE(p+e.yxy) - DE(p-e.yxy),
			 DE(p+e.yyx) - DE(p-e.yyx) ));
}

float trace(in vec3 ro, in vec3 rd)
{
	float t = 0.001;
	for (int i=0; i<30; ++i)
	{
		vec3 p = ro + t * rd;
		float d = DE_bound(p);
		if (d > 0.)
			return -.1;
		d = DE(p);
		if (d < 0.01)
			return t;
		t += d;
	}
	return -1.;
}

void mo_modify_fragment_output()
{
	vec3 ro = v_pos_world - v_center;
	vec3 rd = normalize(v_pos_world - u_cam_pos);
	
	float t = trace(ro, rd);
	if (t < 0.)
		discard;

	vec3 p = ro + t * rd;
	vec3 n = DE_norm(p, 0.001);
	vec3 ln = normalize(vec3(0,10,0) - v_pos_world);

	float fres = pow(clamp(1.+dot(rd,n),0.,1.), 5.);
	float phong = pow(max(0., dot(ln,n)), 3.);
	float spec = pow(max(0., dot(reflect(rd,n),ln)), 6.);
	
	vec3 col = vec3(.6,.9,1.) * phong *.4 + spec*.6;
	col += vec3(.3,.7,1.) * fres;

	out_color = vec4(col, 1.);
	
}
