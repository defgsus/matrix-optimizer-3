// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm


/** Distance to a sphere at origin with radius @p s */
float sdSphere( vec3 p, float s )
{
    return length(p) - s;
}

/** Unsigned distance to a box around origin, with extends given by @p b */
float udBox( vec3 p, vec3 b )
{
    return length(max(abs(p) - b, 0.0));
}

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

/** Distance to a torus with radii in @p t */
float sdTorus( vec3 p, vec2 t )
{
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

/** Distance to a cylinder expanding endlessly on y-axis, with radius in @p c.z */
float sdCylinder( vec3 p, vec3 c )
{
    return length(p.xz - c.xy) - c.z;
}

/** Distance to a cone with normalize radus and length in @p c */
float sdCone( vec3 p, vec2 c )
{
    // c must be normalized
    float q = length(p.xy);
    return dot(c, vec2(q, p.z));
}

/** Distance to a plane at with normal @p n.xyz and offset @p n.w */
float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot(p,n.xyz) + n.w;
}

/** Distance to triangular prism. */
float sdTriPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

/** Distance to hexagonal prism */
float sdHexPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max((q.x*0.866025+q.y*0.5),q.y)-h.x);
}

/** Distance to a cylinder with round caps, end-points in @p a and @p b, radius in @p r */
float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}

/** Distance to a capped cylinder, ... */
float sdCappedCylinder( vec3 p, vec2 h )
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}


float _dot2( in vec3 v ) { return dot(v,v); }

/** Unsigned distance to triangle with corners @p a, @p b, and @p c. */
float udTriangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = cross( ba, ac );

    return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
     sign(dot(cross(cb,nor),pb)) +
     sign(dot(cross(ac,nor),pc))<2.0)
     ?
     min( min(
     _dot2(ba*clamp(dot(ba,pa)/_dot2(ba),0.0,1.0)-pa),
     _dot2(cb*clamp(dot(cb,pb)/_dot2(cb),0.0,1.0)-pb) ),
     _dot2(ac*clamp(dot(ac,pc)/_dot2(ac),0.0,1.0)-pc) )
     :
     dot(nor,pa)*dot(nor,pa)/_dot2(nor) );
}


/** Unsigned distance to a quad with corners @p a, @p b, @p c and @p d. */
float udQuad( vec3 p, vec3 a, vec3 b, vec3 c, vec3 d )
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 dc = d - c; vec3 pc = p - c;
    vec3 ad = a - d; vec3 pd = p - d;
    vec3 nor = cross( ba, ad );

    return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
     sign(dot(cross(cb,nor),pb)) +
     sign(dot(cross(dc,nor),pc)) +
     sign(dot(cross(ad,nor),pd))<3.0)
     ?
     min( min( min(
     _dot2(ba*clamp(dot(ba,pa)/_dot2(ba),0.0,1.0)-pa),
     _dot2(cb*clamp(dot(cb,pb)/_dot2(cb),0.0,1.0)-pb) ),
     _dot2(dc*clamp(dot(dc,pc)/_dot2(dc),0.0,1.0)-pc) ),
     _dot2(ad*clamp(dot(ad,pd)/_dot2(ad),0.0,1.0)-pd) )
     :
     dot(nor,pa)*dot(nor,pa)/_dot2(nor) );
}
