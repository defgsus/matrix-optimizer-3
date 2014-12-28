/** @file angelscript_geometry.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <sstream>

#include "angelscript_geometry.h"
#include "script/angelscript.h"
#include "object/object.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {


//-----------------------
// AngelScript functions
//-----------------------

class GeometryAS
{
    GeometryAS(const GeometryAS&);
    void operator = (GeometryAS&);

public:

    GEOM::Geometry * g;
    int ref;
    StringAS p_name;

    // -------------- factory for script -----------

    GeometryAS()
        : g     (new GEOM::Geometry()),
          ref   (1)
    {
        //MO_DEBUG("GeometryAS("<<this<<")::GeometryAS()");
    }

    ~GeometryAS()
    {
        //MO_DEBUG("GeometryAS("<<this<<")::~GeometryAS()")
        delete g;
    }

    static GeometryAS * factory() { return new GeometryAS(); }

    void addRef() { ++ref; }

    void releaseRef() { if (--ref == 0) delete this; }

    // ----------------- interface ----------------

    std::string toString() const { std::stringstream s; s << "Geometry(" << (void*)this << ")"; return s.str(); }

    // ----- getter -----
    uint getClosestVertex(const Vec3& v);

    const StringAS& name() const { return p_name; }
    uint vertexCount() const { return g->numVertices(); }
    uint lineCount() const { return g->numLines(); }
    uint triangleCount() const { return g->numTriangles(); }

    Vec4 color() const { return Vec4(g->currentRed(), g->currentGreen(), g->currentBlue(), g->currentAlpha()); }
    Vec3 normal() const { return Vec3(g->currentNormalX(), g->currentNormalY(), g->currentNormalZ()); }
    Vec2 texCoord() const { return Vec2(g->currentTexCoordX(), g->currentTexCoordY()); }

    Vec3 vertexI(uint i) const { return i >= g->numVertices() ? Vec3(0) : g->getVertex(i); }
    Vec3 normalI(uint i) const { return i >= g->numVertices() ? Vec3(0,0,1) : g->getNormal(i); }
    Vec2 texCoordI(uint i) const { return i >= g->numVertices() ? Vec2(0) : g->getTexCoord(i); }
    Vec4 colorI(uint i) const { return i >= g->numVertices() ? Vec4(0) : g->getColor(i); }

    bool intersects(const Vec3& ray, const Vec3& dir) { return g->intersects(ray, dir); }
    bool intersects_p(const Vec3& ray, const Vec3& dir, Vec3& pos) { return g->intersects(ray, dir, &pos); }

    // ---- setter ----

    void setName(const StringAS& n) { p_name = n; }

    void setColor4(float r_, float g_, float b_, float a_) { g->setColor(r_, g_, b_, a_); }
    void setColor3(float r_, float g_, float b_) { g->setColor(r_, g_, b_, 1); }
    void setColor2(float r_, float a_) { g->setColor(r_, r_, r_, a_); }
    void setColor1(float r_) { g->setColor(r_, r_, r_, 1); }
    void setColorV3(const Vec3& v) { g->setColor(v.x, v.y, v.z, 1); }
    void setColorV4(const Vec4& v) { g->setColor(v.x, v.y, v.z, v.w); }

    void setTexCoord(float s, float t) { g->setTexCoord(s, t); }
    void setTexCoordV2(const Vec2& v) { g->setTexCoord(v.x, v.y); }

    void setNormal(float x, float y, float z) { g->setNormal(x, y, z); }
    void setNormalV3(const Vec3& v) { g->setNormal(v.x, v.y, v.z); }

    uint addVertex(const Vec3& v) { return g->addVertex(v.x, v.y, v.z); }
    uint addVertexF(float x, float y, float z) { return g->addVertex(x, y, z); }

    void setVertexIV(uint i, const Vec3& v) const { if (i < g->numVertices()) g->setVertex(i, v); }
    void setNormalIV(uint i, const Vec3& v) const { if (i < g->numVertices()) g->setNormal(i, v); }
    void setTexCoordIV(uint i, const Vec2& v) const { if (i < g->numVertices()) g->setTexCoord(i, v); }
    void setColorIV(uint i, const Vec4& v) const { if (i < g->numVertices()) g->setColor(i, v); }

    void createBox1(float sl) { GEOM::GeometryFactory::createBox(g, sl, sl, sl, true); }
    void createBox3(float x, float y, float z) { GEOM::GeometryFactory::createBox(g, x, y, z, true); }
    void createBox3v(const Vec3& s) { GEOM::GeometryFactory::createBox(g, s.x, s.y, s.z, true); }
    void createSphere2(uint segu, uint segv) { GEOM::GeometryFactory::createUVSphere(g, 1, segu, segv, true); }
    void createSphere3(float rad, uint segu, uint segv) { GEOM::GeometryFactory::createUVSphere(g, rad, segu, segv, true); }
    void createTorus4(float rad_outer, float rad_inner, uint segu, uint segv)
        { GEOM::GeometryFactory::createTorus(g, rad_outer, rad_inner, segu, segv, true); }

#define MO__IDX(i__) (i__ < g->numVertices())

    void addLineI(uint v1, uint v2) { if (MO__IDX(v1) && MO__IDX(v2)) g->addLine(v1, v2); }
    void addLine(const Vec3& a, const Vec3& b)
    {
        auto    v1 = g->addVertex(a.x, a.y, a.z),
                v2 = g->addVertex(b.x, b.y, b.z);
        g->addLine(v1, v2);
    }

    void addTriangleI(uint v1, uint v2, uint v3)
        { if (MO__IDX(v1) && MO__IDX(v2) && MO__IDX(v3)) g->addTriangleChecked(v1, v2, v3); }
    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c)
    {
        // check degenerate triangles before creating the vertices
        if (!GEOM::Geometry::checkTriangle(a, b, c))
            return;
        auto    v1 = g->addVertex(a.x, a.y, a.z),
                v2 = g->addVertex(b.x, b.y, b.z),
                v3 = g->addVertex(c.x, c.y, c.z);
        g->addTriangle(v1, v2, v3);
    }

    void addQuadI(uint bl, uint br, uint tr, uint tl)
    {
        if (MO__IDX(bl) && MO__IDX(br) && MO__IDX(tr) && MO__IDX(tl))
            { g->addTriangle(bl, tr, tl); g->addTriangle(bl, br, tr); }
    }
    void addQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d)
    {
        auto    bl = g->addVertex(a.x, a.y, a.z),
                br = g->addVertex(b.x, b.y, b.z),
                tr = g->addVertex(c.x, c.y, c.z),
                tl = g->addVertex(d.x, d.y, d.z);
        g->addTriangle(bl, tr, tl);
        g->addTriangle(bl, br, tr);
    }
#undef MO__IDX

    void addText(const StringAS& text) { GEOM::GeometryFactory::createText(g, Mat4(1), QString::fromUtf8(text.c_str())); }
    void addTextP(const StringAS& text, const Vec3& p)
        { GEOM::GeometryFactory::createText(g, glm::translate(Mat4(1), p), QString::fromUtf8(text.c_str())); }

    void addGeometry(const GeometryAS& o) { g->addGeometry( *o.g ); }
    void addGeometryP(const GeometryAS& o, const Vec3& p) { g->addGeometry( *o.g, p ); }

    void rotate(const Vec3& a, Float deg) { g->applyMatrix( MATH::rotate(Mat4(1), deg, a) ); }
    void rotateX(Float deg) { g->applyMatrix( MATH::rotate(Mat4(1), deg, Vec3(1, 0, 0)) ); }
    void rotateY(Float deg) { g->applyMatrix( MATH::rotate(Mat4(1), deg, Vec3(0, 1, 0)) ); }
    void rotateZ(Float deg) { g->applyMatrix( MATH::rotate(Mat4(1), deg, Vec3(0, 0, 1)) ); }
    void scaleV(const Vec3& s) { g->applyMatrix( glm::scale(Mat4(1), s) ); }
    void scale(Float s) { g->applyMatrix( glm::scale(Mat4(1), Vec3(s,s,s)) ); }
    void translate(const Vec3& v) { g->translate(v.x, v.y, v.z); }
    void translateX(Float v) { g->translate(v, 0, 0); }
    void translateY(Float v) { g->translate(0, v, 0); }
    void translateZ(Float v) { g->translate(0, 0, v); }

    void clear() { g->clear(); }
    void setShared(bool b) { g->setSharedVertices(b); }
    void tesselateTriangles(uint level) { g->tesselateTriangles(level); }
    void tesselateLines(uint level) { g->tesselateLines(level); }
    void calculateNormals() { g->calculateTriangleNormals(); }
    void invertNormals() { g->invertNormals(); }
    void convertToLines() { if (g->numLines()) g->convertToLines(); }
};


uint GeometryAS::getClosestVertex(const Vec3& v)
{
    if (!g->numVertices())
        return 0;

    int best = 0;
    Float bestd = 1000000000000.f,
        dx, dy, dz;
    auto ptr = g->vertices();
    for (uint i=0; i<g->numVertices(); ++i)
    {
        dx = v.x - *ptr++;
        dy = v.y - *ptr++;
        dz = v.z - *ptr++;
        dx += dx * dx + dy * dy + dz * dz;
        if (dx < bestd)
        {
            bestd = dx;
            best = i;
        }
    }
    return best;
}


//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_geometry_native(asIScriptEngine *engine)
{
    int r;

    // register the type
    r = engine->RegisterObjectType("Geometry", 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------

    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_FACTORY,
        "Geometry@ f()", asFUNCTION(GeometryAS::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_ADDREF,
        "void f()", asMETHOD(GeometryAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_RELEASE,
        "void f()", asMETHOD(GeometryAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Geometry", decl__, asMETHOD(GeometryAS,name__), asCALL_THISCALL); assert( r >= 0 );

    // getter
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("const string& name() const", name);
    MO__REG_METHOD("uint vertexCount() const", vertexCount);
    MO__REG_METHOD("uint lineCount() const", lineCount);
    MO__REG_METHOD("uint triangleCount() const", triangleCount);
    MO__REG_METHOD("vec4 color() const", color);
    MO__REG_METHOD("vec3 normal() const", normal);
    MO__REG_METHOD("vec2 texCoord() const", texCoord);

    MO__REG_METHOD("vec3 vertex(uint vertex_index) const", vertexI);
    MO__REG_METHOD("vec4 color(uint vertex_index) const", colorI);
    MO__REG_METHOD("vec3 normal(uint vertex_index) const", normalI);
    MO__REG_METHOD("vec2 texCoord(uint vertex_index) const", texCoordI);

    MO__REG_METHOD("bool intersects(const vec3 &in ray_pos, const vec3 &in ray_dir) const", intersects);
    MO__REG_METHOD("bool intersects(const vec3 &in ray_pos, const vec3 &in ray_dir, vec3 &out hit_pos) const", intersects_p);

    // setter
    MO__REG_METHOD("void clear()", clear);
    MO__REG_METHOD("void calculateNormals()", calculateNormals);
    MO__REG_METHOD("void invertNormals()", invertNormals);
    MO__REG_METHOD("void tesselateTriangles(uint level = 1)", tesselateTriangles);
    MO__REG_METHOD("void tesselateLines(uint level = 1)", tesselateLines);
    MO__REG_METHOD("void convertToLines()", convertToLines);

    MO__REG_METHOD("void setShared(bool = true)", setShared);
    MO__REG_METHOD("void setName(const string& in)", setName);
    MO__REG_METHOD("void setColor(float, float, float, float)", setColor4);
    MO__REG_METHOD("void setColor(float, float, float)", setColor3);
    MO__REG_METHOD("void setColor(float, float)", setColor2);
    MO__REG_METHOD("void setColor(float)", setColor1);
    MO__REG_METHOD("void setColor(const vec3 &in)", setColorV3);
    MO__REG_METHOD("void setColor(const vec4 &in)", setColorV4);
    MO__REG_METHOD("void setTexCoord(float s, float t)", setTexCoord);
    MO__REG_METHOD("void setTexCoord(const vec2 &in st)", setTexCoordV2);
    MO__REG_METHOD("void setNormal(float x, float y, float z)", setNormal);
    MO__REG_METHOD("void setNormal(const vec3 &in)", setNormalV3);

    MO__REG_METHOD("uint addVertex(const vec3 &in)", addVertex);
    MO__REG_METHOD("uint addVertex(float, float, float)", addVertexF);
    MO__REG_METHOD("void addLine(const vec3 &in, const vec3 &in)", addLine);
    MO__REG_METHOD("void addLine(uint, uint)", addLineI);
    MO__REG_METHOD("void addTriangle(const vec3 &in, const vec3 &in, const vec3 &in)", addTriangle);
    MO__REG_METHOD("void addTriangle(uint, uint, uint)", addTriangleI);
    MO__REG_METHOD("void addQuad(const vec3 &in, const vec3 &in, const vec3 &in, const vec3 &in)", addQuad);
    MO__REG_METHOD("void addQuad(uint bl, uint br, uint tr, uint tl)", addQuadI);
    MO__REG_METHOD("void addGeometry(const Geometry &in)", addGeometry);
    MO__REG_METHOD("void addGeometry(const Geometry &in, const vec3 &in)", addGeometryP);
    MO__REG_METHOD("void add(const Geometry &in)", addGeometry);
    MO__REG_METHOD("void add(const Geometry &in, const vec3 &in)", addGeometryP);
    MO__REG_METHOD("void addText(const string &in)", addText);
    MO__REG_METHOD("void addText(const string &in, const vec3 &in)", addTextP);

    MO__REG_METHOD("void setVertex(uint vertex_index, const vec3 &in)", setVertexIV);
    MO__REG_METHOD("void setNormal(uint vertex_index, const vec3 &in)", setNormalIV);
    MO__REG_METHOD("void setTexCoord(uint vertex_index, const vec2 &in)", setTexCoordIV);
    MO__REG_METHOD("void setColor(uint vertex_index, const vec4 &in)", setColorIV);

    MO__REG_METHOD("void createBox(float sidelength = 1)", createBox1);
    MO__REG_METHOD("void createBox(float sidelength_x, float sidelength_y, float sidelength_z)", createBox3);
    MO__REG_METHOD("void createBox(const vec3 &in sidelengths)", createBox3v);
    MO__REG_METHOD("void createSphere(uint segments_u = 12, uint segments_v = 12)", createSphere2);
    MO__REG_METHOD("void createSphere(float radius, uint segments_u = 12, uint segments_v = 12)", createSphere3);
    MO__REG_METHOD("void createTorus(float radius_outer, float radius_inner, uint segments_u = 12, uint segments_v = 12)",
                   createTorus4);

    MO__REG_METHOD("void rotate(const vec3 &in axis, float degree)", rotate);
    MO__REG_METHOD("void rotateX(float degree)", rotateX);
    MO__REG_METHOD("void rotateY(float degree)", rotateY);
    MO__REG_METHOD("void rotateZ(float degree)", rotateZ);
    MO__REG_METHOD("void scale(float)", scale);
    MO__REG_METHOD("void scale(const vec3 &in)", scaleV);
    MO__REG_METHOD("void translate(const vec3 &in)", translate);
    MO__REG_METHOD("void translateX(float)", translateX);
    MO__REG_METHOD("void translateY(float)", translateY);
    MO__REG_METHOD("void translateZ(float)", translateZ);
    MO__REG_METHOD("uint closestVertex(const vec3 &in)", getClosestVertex);


#undef MO__REG_METHOD


    // ------------ non-member object functions ----------------

#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(decl__, asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );

//    MO__REG_FUNC("vec3 rotate(const vec3 &in, const vec3 &in, float)", vecfunc<Vec3>::rotate);


#undef MO__REG_FUNC

}




// ----------------------------------------------

GEOM::Geometry * getGeometry(const GeometryAS * as)
{
    return as ? as->g : 0;
}

void registerAngelScript_geometry(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"Geometry type for Angelscript currently not supported on this platform");
    }
    else
        registerAngelScript_geometry_native(engine);
}




} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT



#ifdef _stuff_that_was_tried_in_the_geometry_editor_

// angelscript test

/* deterministic noise function */
vec3 randomVec(const vec3 &in v)
{
    return vec3(noise(v.x, v.y, v.z),
                noise(v.x+3, 3-v.y, v.z+5),
                noise(3-v.x, v.y+7, 7-v.z));
}


/* like a local rnd generator */

float r_seed = 0;
float r_step = 1.1319834543;

vec3 randomVec(float mi, float ma)
{
    return vec3(noise(r_seed += r_step),
                noise(r_seed += r_step),
                noise(r_seed += r_step));
}



// ---------------------------- scene description --------------------------

float dist_helix(const vec3 &in pos)
{
    vec3 h = vec3(sin(pos.y), pos.y, cos(pos.y));
    return distance(pos, h) - 1;
}

float scene_dist(const vec3 &in pos)
{
    float d = 100000.0;

    // sphere
//	d = min(d, distance(pos, vec3(0,0,0)) - 1 );
//	d = min(d, distance(pos, vec3(5,0,0)) - 2 );
    // box
    d = min(d, max(abs(pos.x-4), max(abs(pos.y),abs(pos.z))) - 1 );

    d = min(d, dist_helix(pos));

    // morph space :D
    //d -= 0.2 * sin(pos.x * 4);

    return d;
}

vec3 scene_normal(const vec3 &in pos)
{
    const float e = 0.1;
    return normalize(vec3(
        scene_dist(pos + vec3(e,0,0)) - scene_dist(pos - vec3(e,0,0)),
        scene_dist(pos + vec3(0,e,0)) - scene_dist(pos - vec3(0,e,0)),
        scene_dist(pos + vec3(0,0,e)) - scene_dist(pos - vec3(0,0,e))));
}



// ----------------------------- renderer -------------------------------

/* get a somehow closer position to the surface,
    d must be abs(scene_dist(pos)) */
vec3 getCloser(vec3 pos, float d)
{
    for (int i=0; i<5; ++i)
    {
        vec3 npos = pos + randomVec(-1,1);
        float nd = abs(scene_dist(npos));
        if (nd < d)
        {
            nd = d;
            pos = npos;
        }
    }
    return pos;
}

/** Creates points close to the 'surface' of the scene,
    by checking a quadratic region */
void createGrid(Geometry@ g, const vec3 &in c1, const vec3 &in c2, float st = 1)
{
    st = max(0.001, st);
    float s = st;
    for (float z = c1.z; z <= c2.z; z += s)
    for (float y = c1.y; y <= c2.y; y += s)
    for (float x = c1.x; x <= c2.x; x += s)
    {
        vec3 pos = vec3(x, y, z);
        //pos += randomVec(pos*33.13) * st * 2;

        // distance to scene at current point
        float d = scene_dist(pos);

        if (abs(d) < 0.1)
            g.addVertex(pos);
    }
}

/** Connects the grid points somehow */
void createLines(Geometry@ g, const vec3 &in c1, const vec3 &in c2, float st = 1)
{
    st = max(0.001, st);
    float s = st;
    for (float z = c1.z; z <= c2.z; z += s)
    for (float y = c1.y; y <= c2.y; y += s)
    for (float x = c1.x; x <= c2.x; x += s)
    {
        vec3 pos = vec3(x, y, z);
        //pos += randomVec(pos*33.13) * st * 2;

        // distance to scene at current point
        float d = scene_dist(pos);

        if (abs(d) < 0.1)
        {
            uint v1 = g.closestVertex(pos),
                 v2 = g.closestVertex(pos + randomVec(-1,1));
            if (v1 != v2)
                g.addLine(v1, v2);
        }
    }
}


void main()
{
    Geometry g;
    g.setColor(0.5, 0.5, 0.5, 1);

    vec3 start=vec3(-4,-4,-4),
         end=  vec3(5,4,4);

    createGrid(g, start, end, 0.2);
    createLines(g, start, end, 0.1);

    addGeometry(g);
}


#endif
