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

    std::string toString() const { std::stringstream s; s << "Geometry(" << (void*)this << ")"; return s.str(); }

    std::string name() const { return "geometry"; }
    uint vertexCount() const { return g->numVertices(); }

    void setRGBA(float r_, float g_, float b_, float a_) { g->setColor(r_, g_, b_, a_); }
    void setBrightAlpha(float r_, float a_) { g->setColor(r_, r_, r_, a_); }
    void setBright(float r_) { g->setColor(r_, r_, r_, 1); }
    void setColor3(const Vec3& v) { g->setColor(v.x, v.y, v.z, 1.f); }

    void addLine(const Vec3& a, const Vec3& b)
    {
        auto    v1 = g->addVertex(a.x, a.y, a.z),
                v2 = g->addVertex(b.x, b.y, b.z);
        g->addLine(v1, v2);
    }

    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c)
    {
        auto    v1 = g->addVertex(a.x, a.y, a.z),
                v2 = g->addVertex(b.x, b.y, b.z),
                v3 = g->addVertex(c.x, c.y, c.z);
        g->addTriangle(v1, v2, v3);
    }

    void addGeometry(const GeometryAS& o) { g->addGeometry( *o.g ); }

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

};


//--------------------------------
// Registration
//-------------------------------------

static void registerAngelScript_geometry_native(asIScriptEngine *engine)
{
    int r;

    // register the type
    //r = engine->RegisterObjectType("Geometry", sizeof(GEOM::Geometry), asOBJ_VALUE | asOBJ_APP_CLASS_CD); assert( r >= 0 );
    r = engine->RegisterObjectType("Geometry", 0, asOBJ_REF); assert( r >= 0 );

    // ----------------- constructor ---------------------------


//    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(constructGeometry), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    //r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_CONSTRUCT,  "void f(const Geometry &in)",  asFUNCTION(copyConstructGeometry), asCALL_CDECL_OBJLAST); assert( r >= 0 );
//    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(destructGeometry),  asCALL_CDECL_OBJLAST); assert( r >= 0 );

    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_FACTORY, "Geometry@ f()", asFUNCTION(GeometryAS::factory), asCALL_CDECL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_ADDREF, "void f()", asMETHOD(GeometryAS,addRef), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("Geometry", asBEHAVE_RELEASE, "void f()", asMETHOD(GeometryAS,releaseRef), asCALL_THISCALL); assert( r >= 0 );

    // --------------- the object methods ----------------------

#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod("Geometry", decl__, asMETHOD(GeometryAS,name__), asCALL_THISCALL); assert( r >= 0 );
    //r = engine->RegisterObjectMethod("Geometry", decl__, asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // getter
    MO__REG_METHOD("string opImplConv() const", toString);
    MO__REG_METHOD("string name() const", name);
    MO__REG_METHOD("int vertexCount() const", vertexCount);

    // setter
    MO__REG_METHOD("void setColor(float, float, float, float)", setRGBA);
    MO__REG_METHOD("void setColor(float, float)", setBrightAlpha);
    MO__REG_METHOD("void setColor(float)", setBright);
    MO__REG_METHOD("void setColor(const vec3 &in)", setColor3);
    MO__REG_METHOD("void addLine(const vec3 &in, const vec3 &in)", addLine);
    MO__REG_METHOD("void addTriangle(const vec3 &in, const vec3 &in, const vec3 &in)", addTriangle);
    MO__REG_METHOD("void addGeometry(const Geometry &in)", addGeometry);
    MO__REG_METHOD("void add(const Geometry &in)", addGeometry);
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
