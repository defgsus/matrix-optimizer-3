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

/** Distance to a cone with normalize radius and length in @p c */
float sdCone( vec3 p, vec2 c )
{
    // c must be normalized
    float q = length(p.xy);
    return dot(c, vec2(q, p.z));
}

/** Distance to a plane with normal @p n.xyz and offset @p n.w */
float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot(p,n.xyz) + n.w;
}

/** Distance to triangular prism, size and length along z in @p h */
float sdTriPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

/** Distance to hexagonal prism, size and length along z in @p h */
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


// -- from https://www.shadertoy.com/view/4dKGWm --
// Created by inigo quilez - iq/2016
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0


/** Ellipsoid at position @p c, radius in @p r */
float sdEllipsoid( in vec3 p, in vec3 c, in vec3 r )
{
    return (length( (p-c)/r ) - 1.0) * min(min(r.x,r.y),r.z);
}

// http://research.microsoft.com/en-us/um/people/hoppe/ravg.pdf
float _det( vec2 a, vec2 b ) { return a.x*b.y-b.x*a.y; }
vec3 _getClosest( vec2 b0, vec2 b1, vec2 b2 )
{

  float a =     _det(b0,b2);
  float b = 2.0*_det(b1,b0);
  float d = 2.0*_det(b2,b1);
  float f = b*d - a*a;
  vec2  d21 = b2-b1;
  vec2  d10 = b1-b0;
  vec2  d20 = b2-b0;
  vec2  gf = 2.0*(b*d21+d*d10+a*d20);
        gf = vec2(gf.y,-gf.x);
  vec2  pp = -f*gf/dot(gf,gf);
  vec2  d0p = b0-pp;
  float ap = _det(d0p,d20);
  float bp = 2.0*_det(d10,d0p);
  float t = clamp( (ap+bp)/(2.0*a+b+d), 0.0 ,1.0 );
  return vec3( mix(mix(b0,b1,t), mix(b1,b2,t),t), t );
}

/** XXX */
vec2 sdBezier( vec3 a, vec3 b, vec3 c, vec3 p, out vec2 pos )
{
    vec3 w = normalize( cross( c-b, a-b ) );
    vec3 u = normalize( c-b );
    vec3 v = normalize( cross( w, u ) );

    vec2 a2 = vec2( dot(a-b,u), dot(a-b,v) );
    vec2 b2 = vec2( 0.0 );
    vec2 c2 = vec2( dot(c-b,u), dot(c-b,v) );
    vec3 p3 = vec3( dot(p-b,u), dot(p-b,v), dot(p-b,w) );

    vec3 cp = _getClosest( a2-p3.xy, b2-p3.xy, c2-p3.xy );

    pos = cp.xy;

    return vec2( sqrt(dot(cp.xy,cp.xy)+p3.z*p3.z), cp.z );
}

