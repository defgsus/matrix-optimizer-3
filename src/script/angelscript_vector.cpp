/** @file angelscript_vector.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>
#include <cstring> // strstr
#include <cmath>
#include <sstream>

#include <QString>
#include <QColor> // for hsv stuff

#include "angelscript_vector.h"
#include "script/angelscript.h"
#include "types/vector.h"
#include "math/constants.h"
#include "math/vector.h"
#include "math/advanced.h"
#include "math/intersection.h"

/** @todo NEEDS ALL BE TESTED PROPERLY */


/* Here we register vec2,vec3,vec4 and (currently) mat3 and mat4 types
   and their associated functions for AngelScript (a.t.m. float only).
   The structure of this file is a littlebit complicated. There's a lot
   of template and macro magic, simpily to not having to write almost the
   same stuff multiple times. */

namespace MO {

namespace {

/** Simple traits class to unify some functions for all vector types.
    (Sure there is something in glm for this as well) */
template <class V> struct vectraits;
template <>
struct vectraits<Vec2> { static const int num = 2; };
template <>
struct vectraits<Vec3> { static const int num = 3; };
template <>
struct vectraits<Vec4> { static const int num = 4; };

/** Native function binding.
    XXX I'll never rewrite this for generic function binding!! */
namespace native {

//-----------------------
// AngelScript functions
//-----------------------

/** Wraps some functions and makes them non-overloading
    to avoid nasty syntax when registering.

    XXX This struct is a bit messy, not all of it's functions can be instantiated for all types.
    To avoid conflicts with overly paranoid compilers the specific types are declared explicitly. */
template <typename Vec>
struct vecfunc
{
    static StringAS toString(Vec * self) { std::stringstream s; s << *self; return s.str(); }

    static void vecDefaultConstructor(Vec *self) { new(self) Vec(); }
    static void vecCopyConstructor(Vec *self, const Vec &other) { new(self) Vec(other); }

    static void vecListConstructor2(Vec2 *self, Float *list) { new(self) Vec2(list[0], list[1]); }
    static void vecListConstructor3(Vec3 *self, Float *list) { new(self) Vec3(list[0], list[1], list[2]); }
    static void vecListConstructor4(Vec4 *self, Float *list) { new(self) Vec4(list[0], list[1], list[2], list[3]); }

    static void vecInitConstructor2(Vec2 *self, Float x, Float y) { new(self) Vec2(x,y); }
    static void vecInitConstructor3(Vec3 *self, Float x, Float y, Float z) { new(self) Vec3(x,y,z); }
    static void vecInitConstructor4(Vec4 *self, Float x, Float y, Float z, Float w) { new(self) Vec4(x,y,z,w); }

    static void vecConvConstructor(Vec *self, Float x) { new(self) Vec(x); }
    template <typename V>
    static void vecConvConstructorVec(Vec *self, const V& v) { new(self) Vec(v); }
    static void vecConvConstructorVec32(Vec3 *self, const Vec2& v, float z) { new(self) Vec3(v.x, v.y, z); }
    static void vecConvConstructorVec42(Vec4 *self, const Vec2& v, float z, float w) { new(self) Vec4(v.x, v.y, z, w); }
    static void vecConvConstructorVec43(Vec4 *self, const Vec3& v, float w) { new(self) Vec4(v.x, v.y, v.z, w); }

    static Vec& assignFloat(Vec * self, Float f) { for (int i=0; i<vectraits<Vec>::num; ++i) (*self)[i] = f; return *self; }

    static bool vecEqualsVec(Vec * self, const Vec& v) { return *self == v; }
    static Vec vecAddVec(Vec * self, const Vec& v) { return *self + v; }
    static Vec vecSubVec(Vec * self, const Vec& v) { return *self - v; }
    static Vec vecMulVec(Vec * self, const Vec& v) { return *self * v; }
    static Vec vecDivVec(Vec * self, const Vec& v) { return *self / v; }

    static Vec vecAddFloat(Vec * self, Float v) { return *self + v; }
    static Vec vecSubFloat(Vec * self, Float v) { return *self - v; }
    static Vec vecMulFloat(Vec * self, Float v) { return *self * v; }
    static Vec vecDivFloat(Vec * self, Float v) { return *self / v; }

    static Vec vecRotated(Vec * self, const Vec3& a, Float deg) { return MATH::rotate(*self, a, deg); }
    static Vec vecRotatedX(Vec * self, Float deg) { return MATH::rotateX(*self, deg); }
    static Vec vecRotatedY(Vec * self, Float deg) { return MATH::rotateY(*self, deg); }
    static Vec vecRotatedZ(Vec * self, Float deg) { return MATH::rotateZ(*self, deg); }

    // not clear yet if this should be a member
    // not used yet
    static Float minimum(Vec * self)
        { Float m = (*self)[0]; for (int i=1; i<vectraits<Vec>::num; ++i) m = std::min(m, (*self)[i]); return m; }

    // -- nonmembers --

    static Vec floatAddVec(const Vec& v, Float f) { return v + f; }

    static Vec rotate(const Vec& v, const Vec3& a, Float deg) { return MATH::rotate(v, a, deg); }
    static Vec rotateX(const Vec& v, Float deg) { return MATH::rotateX(v, deg); }
    static Vec rotateY(const Vec& v, Float deg) { return MATH::rotateY(v, deg); }
    static Vec rotateZ(const Vec& v, Float deg) { return MATH::rotateZ(v, deg); }

    static Vec normalize(const Vec& v) { return MATH::normalize_safe(v); }
    static Vec cross(const Vec& v, const Vec& n) { return glm::cross(v, n); }
    static Vec reflect(const Vec& v, const Vec& n) { return glm::reflect(v, n); }
    static Vec refract(const Vec& v, const Vec& n, Float eta) { return glm::refract(v, n, eta); }

    static Vec min_vv(const Vec& v, const Vec& n) { return glm::min(v, n); }
    static Vec min_vf(const Vec& v, Float f) { return glm::min(v, f); }
    static Vec min_fv(Float f, const Vec& v) { return glm::min(v, f); }
    static Vec max_vv(const Vec& v, const Vec& n) { return glm::max(v, n); }
    static Vec max_vf(const Vec& v, Float f) { return glm::max(v, f); }
    static Vec max_fv(Float f, const Vec& v) { return glm::max(v, f); }
    static Vec clamp(const Vec& v, float mi, float ma) { return glm::clamp(v, mi, ma); }

    static Vec mix(const Vec& a, const Vec& b, float x) { return glm::mix(a, b, x); }


    static Float length(const Vec& v) { return glm::length(v); }
    static Float distance(const Vec& v, const Vec& n) { return glm::distance(v, n); }
    static Float dot(const Vec& a, const Vec& b) { return glm::dot(a, b); }

    static Float beta(const Vec& v)
    {
        Float x = v[0]*v[0]; for (int i=1; i<vectraits<Vec>::num; ++i) x += v[i]*v[i];
        x = Float(1) - x; return x > 0 ? std::sqrt(x) : 0;
    }

    static Float smallest(const Vec& v)
        { Float m = v[0]; for (int i=1; i<vectraits<Vec>::num; ++i) m = std::min(m, v[i]); return m; }
    static Float largest(const Vec& v)
        { Float m = v[0]; for (int i=1; i<vectraits<Vec>::num; ++i) m = std::max(m, v[i]); return m; }

    static Float noisef_2(const Vec2& v) { return MATH::advanced<float>::noise_2(v.x, v.y); }
    static Float noisef_3(const Vec3& v) { return MATH::advanced<float>::noise_3(v.x, v.y, v.z); }
    static Vec2 noisev2_1(float f) { return Vec2(MATH::advanced<float>::noise(f),
                                                 MATH::advanced<float>::noise(-f - 57.f)); }
    static Vec3 noisev3_1(float f) { return Vec3(MATH::advanced<float>::noise(f),
                                                 MATH::advanced<float>::noise(-f - 57.f),
                                                 MATH::advanced<float>::noise(f + 1007.f)); }
    static Vec4 noisev4_1(float f) { return Vec4(MATH::advanced<float>::noise(f),
                                                 MATH::advanced<float>::noise(-f - 57.f),
                                                 MATH::advanced<float>::noise(f + 1007.f),
                                                 MATH::advanced<float>::noise(-f - 885.f)); }

    static Float voronoi_2(const Vec2& v) { return MATH::advanced<float>::voronoi_2(v.x, v.y); }
    static Float voronoi_3(const Vec3& v) { return MATH::advanced<float>::voronoi_3(v.x, v.y, v.z); }

    static Float svoronoi_2(const Vec2& v, Float sm) { return MATH::advanced<float>::svoronoi_2(v.x, v.y, sm); }
    static Float svoronoi_3(const Vec3& v, Float sm) { return MATH::advanced<float>::svoronoi_3(v.x, v.y, v.z, sm); }

    static Vec3 euclidean2polar(const Vec3& euclidean)
    {
        const Float length = glm::length(euclidean);
        if (length < std::numeric_limits<Float>::epsilon())
            return Vec3(0);
        const Vec3 tmp = euclidean / length;
        const Float xz_dist = std::sqrt(tmp.x * tmp.x + tmp.z * tmp.z);

        return Vec3(
            MATH::rad_to_deg(std::atan2(xz_dist, tmp.y)),	// latitude
            MATH::rad_to_deg(std::atan2(tmp.x, tmp.z)),     // longitude
            xz_dist);                                       // xz distance
    }

    static Vec3 polar2euclidean(const Vec3& polar)
    {
        const Float
                latitude = MATH::deg_to_rad(polar.x),
                longitude = MATH::deg_to_rad(polar.y);
        return Vec3(
            std::cos(latitude) * std::sin(longitude),
            std::sin(latitude),
            std::cos(latitude) * std::cos(longitude));
    }

    static Vec2 ulam_spiral(int i)
    {
        int x,y;
        MATH::advanced_int<int>::ulam_spiral_inv(i, x, y);
        return Vec2(x,y);
    }

    // ------- color conv -------

    // wrapper for QColor (not directly used by script)
    static Vec3 toVecRgb3(const QColor& c) { return Vec(c.redF(), c.greenF(), c.blueF()); }
    static Vec3 toVecHsv3(const QColor& c) { return Vec(c.hueF(), c.saturationF(), c.valueF()); }
    static Vec4 toVecRgb4(const QColor& c) { return Vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }
    static Vec4 toVecHsv4(const QColor& c) { return Vec4(c.hueF(), c.saturationF(), c.valueF(), c.alphaF()); }

    static Vec3 hsv2rgb_3f(float h, float s, float v) { return hsv2rgb_3(Vec3(h,s,v)); }
    static Vec4 hsv2rgb_4f(float h, float s, float v, float a) { return hsv2rgb_4(Vec4(h,s,v,a)); }
    static Vec3 rgb2hsv_3f(float r, float g, float b) { return rgb2hsv_3(Vec3(r,g,b)); }
    static Vec4 rgb2hsv_4f(float r, float g, float b, float a) { return rgb2hsv_4(Vec4(r,g,b,a)); }

    static Vec3 hsv2rgb_3(const Vec3& c) { return toVecRgb3(QColor::fromHsvF(MATH::moduloSigned(c.x, Float(1)),
                                                                            glm::clamp(c.y, Float(0), Float(1)),
                                                                            glm::clamp(c.z, Float(0), Float(1)))); }
    static Vec3 rgb2hsv_3(const Vec3& c) { return toVecHsv3(QColor::fromRgbF(glm::clamp(c.x, Float(0), Float(1)),
                                                                            glm::clamp(c.y, Float(0), Float(1)),
                                                                            glm::clamp(c.z, Float(0), Float(1)))); }

    static Vec4 hsv2rgb_4(const Vec4& c) { return toVecRgb4(QColor::fromHsvF(MATH::moduloSigned(c.x, Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)),
                                                                           glm::clamp(c.w, Float(0), Float(1)))); }
    static Vec4 rgb2hsv_4(const Vec4& c) { return toVecHsv4(QColor::fromRgbF(glm::clamp(c.x, Float(0), Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)),
                                                                           glm::clamp(c.w, Float(0), Float(1)))); }

    static bool intersect_ray_sphere(const Vec3& ray_origin,
                                     const Vec3& ray_direction,
                                     const Vec3& sphere_center,
                                     Float sphere_radius)
    { return MATH::intersect_ray_sphere(ray_origin, ray_direction, sphere_center, sphere_radius); }

    static bool intersect_ray_sphere_dd(const Vec3& ray_origin,
                                     const Vec3& ray_direction,
                                     const Vec3& sphere_center,
                                     Float sphere_radius,
                                     Float & depth1,
                                     Float & depth2)
    { return MATH::intersect_ray_sphere(ray_origin, ray_direction, sphere_center, sphere_radius, &depth1, &depth2); }

    static bool intersect_ray_triangle(const Vec3& ray_origin,
                                       const Vec3& ray_direction,
                                       const Vec3& t0, const Vec3& t1, const Vec3& t2)
    { return MATH::intersect_ray_triangle(ray_origin, ray_direction, t0,t1,t2); }

    static bool intersect_ray_triangle_p(const Vec3& ray_origin,
                                         const Vec3& ray_direction,
                                         const Vec3& t0, const Vec3& t1, const Vec3& t2,
                                         Vec3& pos)
    { return MATH::intersect_ray_triangle(ray_origin, ray_direction, t0,t1,t2, &pos); }

    static Vec3 closest_point_on_line(const Vec3& p, const Vec3& a, const Vec3& b)
        { return MATH::closest_point_on_line(p, a, b); }

    static float mandel(const Vec2& ri) { return MATH::fractal<Float,uint>::mandel(ri.x, ri.y); }
    static uint mandeli(const Vec2& ri) { return MATH::fractal<Float,uint>::mandeli(ri.x, ri.y); }
    static float mandel3(const Vec2& ri, uint maxiter) { return MATH::fractal<Float,uint>::mandel_3(ri.x, ri.y, maxiter); }
    static uint mandeli3(const Vec2& ri, uint maxiter) { return MATH::fractal<Float,uint>::mandeli_3(ri.x, ri.y, maxiter); }

    static float julia(const Vec2& ri, const Vec2& start) { return MATH::fractal<Float,uint>::julia(start.x, start.y, ri.x, ri.y); }
    static uint juliai(const Vec2& ri, const Vec2& start) { return MATH::fractal<Float,uint>::juliai(start.x, start.y, ri.x, ri.y); }

    static float duckball(const Vec2& ri, const Vec2& start, Float bailout) { return MATH::fractal<Float,uint>::duckball(ri.x, ri.y, start.x, start.y, bailout); }

    static Vec3 duckball3(const Vec3& pos, const Vec3& param = Vec3(-.5f, -.4f, -1.578f), uint iter = 32)
    {
        Vec3 p = pos;
        for (int i = 0; i < iter; ++i)
        {
            Float mag = glm::dot(p, p);
            p = glm::abs(p);
            if (mag != 0)
                 p /= mag;
            p += param;
        }
        return p;
    }
};

//--------------------------------
// Registration
//-------------------------------------

// replaces %1 with typ (const char*) and returns utf8 as const char*
#define MO__STR(str__) (QString(str__).arg(typ).toUtf8().constData())

// to register a static function as class method for typ
#define MO__REG_METHOD(decl__, name__) \
    r = engine->RegisterObjectMethod(typ, MO__STR(decl__), asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
// same but without the string replacement
#define MO__REG_METHoD(decl__, name__) \
    r = engine->RegisterObjectMethod(typ, (decl__), asFUNCTION(name__), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

// to register a true member method of the class
#define MO__REG_TRUE_METHOD(decl__, asMETHOD__) \
    r = engine->RegisterObjectMethod(typ, MO__STR(decl__), asMETHOD__, asCALL_THISCALL); assert( r >= 0 );

// reg a non-member function
#define MO__REG_FUNC(decl__, name__) \
    r = engine->RegisterGlobalFunction(MO__STR(decl__), asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );
// same without string replacement
#define MO__REG_FuNC(decl__, name__) \
    r = engine->RegisterGlobalFunction((decl__), asFUNCTION(name__), asCALL_CDECL); assert( r >= 0 );


/** Registers constructors, functions and operators common to all vec types (e.g. 2 - 4). */
template <class Vec>
void register_vector_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // ----------- object properties ------------

    //r = engine->RegisterObjectProperty(typ, "float z", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float w", asOFFSET(Vec, w)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float b", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float a", asOFFSET(Vec, w)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float u", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float v", asOFFSET(Vec, w)); assert( r >= 0 );

    // ------------- constructors ---------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        "void f()",
        asFUNCTION(vecfunc<Vec>::vecDefaultConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        MO__STR("void f(const %1 &in)"),
        asFUNCTION(vecfunc<Vec>::vecCopyConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        "void f(float)",
        asFUNCTION(vecfunc<Vec>::vecConvConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // -------------- operator overloads ---------------

    MO__REG_METHoD("string opImplConv() const", vecfunc<Vec>::toString);

    MO__REG_TRUE_METHOD("%1 &opAssign(const %1 &in)", asMETHODPR(Vec, operator=, (const Vec &), Vec&));
    MO__REG_METHOD("%1 &opAssign(float)", vecfunc<Vec>::assignFloat);

    MO__REG_TRUE_METHOD("%1 &opAddAssign(const %1 &in)", asMETHODPR(Vec, operator+=, (const Vec &), Vec&));
    MO__REG_TRUE_METHOD("%1 &opSubAssign(const %1 &in)", asMETHODPR(Vec, operator-=, (const Vec &), Vec&));
    MO__REG_TRUE_METHOD("%1 &opMulAssign(const %1 &in)", asMETHODPR(Vec, operator*=, (const Vec &), Vec&));
    MO__REG_TRUE_METHOD("%1 &opDivAssign(const %1 &in)", asMETHODPR(Vec, operator/=, (const Vec &), Vec&));

    MO__REG_TRUE_METHOD("%1 &opAddAssign(float)", asMETHODPR(Vec, operator+=, (float), Vec&));
    MO__REG_TRUE_METHOD("%1 &opSubAssign(float)", asMETHODPR(Vec, operator-=, (float), Vec&));
    MO__REG_TRUE_METHOD("%1 &opMulAssign(float)", asMETHODPR(Vec, operator*=, (float), Vec&));
    MO__REG_TRUE_METHOD("%1 &opDivAssign(float)", asMETHODPR(Vec, operator/=, (float), Vec&));
    // vec + vec
    MO__REG_METHOD("%1 opAdd(const %1 &in) const", vecfunc<Vec>::vecAddVec);
    MO__REG_METHOD("%1 opSub(const %1 &in) const", vecfunc<Vec>::vecSubVec);
    MO__REG_METHOD("%1 opMul(const %1 &in) const", vecfunc<Vec>::vecMulVec);
    MO__REG_METHOD("%1 opDiv(const %1 &in) const", vecfunc<Vec>::vecDivVec);
    // vec + float
    MO__REG_METHOD("%1 opAdd(float) const", vecfunc<Vec>::vecAddFloat);
    MO__REG_METHOD("%1 opSub(float) const", vecfunc<Vec>::vecSubFloat);
    MO__REG_METHOD("%1 opMul(float) const", vecfunc<Vec>::vecMulFloat);
    MO__REG_METHOD("%1 opDiv(float) const", vecfunc<Vec>::vecDivFloat);
    // float + vec
    MO__REG_METHOD("%1 opAdd_r(float) const", vecfunc<Vec>::vecAddFloat);
    MO__REG_METHOD("%1 opSub_r(float) const", vecfunc<Vec>::vecSubFloat);
    MO__REG_METHOD("%1 opMul_r(float) const", vecfunc<Vec>::vecMulFloat);
    MO__REG_METHOD("%1 opDiv_r(float) const", vecfunc<Vec>::vecDivFloat);

    // ---------- methods -----------------------


    // ------ non-member functions -------

    MO__REG_FUNC("%1 normalize(const %1 &in)", vecfunc<Vec>::normalize);
    MO__REG_FUNC("%1 reflect(const %1 &in, const %1 &in)", vecfunc<Vec>::reflect);
    MO__REG_FUNC("%1 refract(const %1 &in, const %1 &in, float)", vecfunc<Vec>::refract);

    MO__REG_FUNC("%1 min(const %1 &in, const %1 &in)", vecfunc<Vec>::min_vv);
    MO__REG_FUNC("%1 min(const %1 &in, float)", vecfunc<Vec>::min_vf);
    MO__REG_FUNC("%1 min(float, const %1 &in)", vecfunc<Vec>::min_fv);
    MO__REG_FUNC("%1 max(const %1 &in, const %1 &in)", vecfunc<Vec>::max_vv);
    MO__REG_FUNC("%1 max(const %1 &in, float)", vecfunc<Vec>::max_vf);
    MO__REG_FUNC("%1 max(float, const %1 &in)", vecfunc<Vec>::max_fv);
    MO__REG_FUNC("%1 clamp(const %1 &in, float, float)", vecfunc<Vec>::clamp);
    MO__REG_FUNC("%1 mix(const %1 &in, const %1 &in, float t)", vecfunc<Vec>::mix);

    MO__REG_FUNC("float dot(const %1 &in, const %1 &in)", vecfunc<Vec>::dot);
    MO__REG_FUNC("float length(const %1 &in)", vecfunc<Vec>::length);
    MO__REG_FUNC("float distance(const %1 &in, const %1 &in)", vecfunc<Vec>::distance);

    MO__REG_FUNC("float smallest(const %1 &in)", vecfunc<Vec>::smallest);
    MO__REG_FUNC("float largest(const %1 &in)", vecfunc<Vec>::largest);

    MO__REG_FUNC("float beta(const %1 &in)", vecfunc<Vec>::beta);
}

/** Specific stuff for 2 */
void register_vector_2(asIScriptEngine *engine, const char * typ = "vec2")
{
    int r; Q_UNUSED(r);

    // ---------- constructors ----------
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_CONSTRUCT,
        "void f(float, float)",
        asFUNCTION(vecfunc<Vec2>::vecInitConstructor2), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_LIST_CONSTRUCT,
        "void f(const int &in) { float, float }",
        asFUNCTION(vecfunc<Vec2>::vecListConstructor2), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_CONSTRUCT,
        "void f(const vec3 &in)",
        asFUNCTION(vecfunc<Vec2>::vecConvConstructorVec<Vec3>), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_CONSTRUCT,
        "void f(const vec4 &in)",
        asFUNCTION(vecfunc<Vec2>::vecConvConstructorVec<Vec4>), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // ------------------ properties ------------------

    r = engine->RegisterObjectProperty(typ, "float x", asOFFSET(Vec2, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float y", asOFFSET(Vec2, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float r", asOFFSET(Vec2, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float g", asOFFSET(Vec2, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float s", asOFFSET(Vec2, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float t", asOFFSET(Vec2, y)); assert( r >= 0 );

    // --------------------- methods ------------------
    MO__REG_METHOD("%1 rotated(float)", vecfunc<Vec2>::vecRotatedZ);
    MO__REG_METHOD("%1 rotatedZ(float)", vecfunc<Vec2>::vecRotatedZ);

    // ------------------ non-member functions --------
    MO__REG_FUNC("%1 rotate(%1 &in, float)", vecfunc<Vec2>::rotateZ);
    MO__REG_FUNC("%1 rotateZ(%1 &in, float)", vecfunc<Vec2>::rotateZ);

    MO__REG_FUNC("float noise(const %1 &in)", vecfunc<Vec2>::noisef_2);
    MO__REG_FUNC("%1 noise2(float)", vecfunc<Vec2>::noisev2_1);
    MO__REG_FUNC("float voronoi(const %1 &in)", vecfunc<Vec2>::voronoi_2);
    MO__REG_FUNC("float svoronoi(const %1 &in, float sm = 32.f)", vecfunc<Vec2>::svoronoi_2);

    MO__REG_FUNC("%1 ulam_spiral(int n)", vecfunc<Vec2>::ulam_spiral);

    MO__REG_FUNC("float mandel(const %1 &in real_and_imag)", vecfunc<Vec2>::mandel);
    MO__REG_FUNC("float mandel(const %1 &in real_and_imag, uint max_iterations)", vecfunc<Vec2>::mandel3);
    MO__REG_FUNC("uint mandeli(const %1 &in real_and_imag)", vecfunc<Vec2>::mandeli);
    MO__REG_FUNC("uint mandeli(const %1 &in real_and_imag, uint max_iterations)", vecfunc<Vec2>::mandeli3);
    MO__REG_FUNC("float julia(const %1 &in real_and_imag, const %1 &in start)", vecfunc<Vec2>::julia);
    MO__REG_FUNC("uint juliai(const %1 &in real_and_imag, const %1 &in start)", vecfunc<Vec2>::juliai);
    MO__REG_FUNC("float duckball(const %1 &in pos, const %1 &in start = vec2(0.5), float bailout = 10)", vecfunc<Vec2>::duckball);

}

/** Specific stuff for 3 */
void register_vector_3(asIScriptEngine *engine)
{
    const char * typ = "vec3";
    int r; Q_UNUSED(r);

    // --------- constructors ----------

    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,
        "void f(float, float, float)",
        asFUNCTION(vecfunc<Vec3>::vecInitConstructor3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_LIST_CONSTRUCT,
        "void f(const int &in) {float, float, float}",
        asFUNCTION(vecfunc<Vec3>::vecListConstructor3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,
        "void f(const vec2 &in xy, float z = 0)",
        asFUNCTION(vecfunc<Vec3>::vecConvConstructorVec32), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,
        "void f(const vec4 &in)",
        asFUNCTION(vecfunc<Vec3>::vecConvConstructorVec<Vec4>), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // ------------ properties ---------

    r = engine->RegisterObjectProperty(typ, "float x", asOFFSET(Vec3, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float y", asOFFSET(Vec3, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float r", asOFFSET(Vec3, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float g", asOFFSET(Vec3, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float s", asOFFSET(Vec3, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float t", asOFFSET(Vec3, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float z", asOFFSET(Vec3, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float b", asOFFSET(Vec3, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float u", asOFFSET(Vec3, z)); assert( r >= 0 );

    // --------------------- methods ------------------

    // ------------------ non-member functions --------

    MO__REG_FUNC("%1 hsv2rgb(const %1 &in)", vecfunc<Vec3>::hsv2rgb_3);
    MO__REG_FUNC("%1 hsv2rgb(float h, float s, float v)", vecfunc<Vec3>::hsv2rgb_3f);
    MO__REG_FUNC("%1 rgb2hsv(const %1 &in)", vecfunc<Vec3>::rgb2hsv_3);
    MO__REG_FUNC("%1 rgb2hsv(float r, float g, float b)", vecfunc<Vec3>::rgb2hsv_3f);
    MO__REG_FUNC("%1 hsv(const %1 &in)", vecfunc<Vec3>::rgb2hsv_3);
    MO__REG_FUNC("%1 hsv(float r, float g, float b)", vecfunc<Vec3>::rgb2hsv_3f);

    MO__REG_FUNC("%1 cross(const %1 &in, const %1 &in)", vecfunc<Vec3>::cross);

    MO__REG_FUNC("float noise(const %1 &in)", vecfunc<Vec3>::noisef_3);
    MO__REG_FUNC("%1 noise3(float)", vecfunc<Vec3>::noisev3_1);
    MO__REG_FUNC("float voronoi(const %1 &in)", vecfunc<Vec3>::voronoi_3);
    MO__REG_FUNC("float svoronoi(const %1 &in, float sm = 32.f)", vecfunc<Vec3>::svoronoi_3);

    MO__REG_FUNC("%1 duckball3(const %1 &in pos, const %1 &in param = %1(-.5, -.4, -1.578), uint iterations = 32)", vecfunc<Vec3>::duckball3);

    MO__REG_FUNC("%1 euclidean2polar(const %1 &in)", vecfunc<Vec3>::euclidean2polar);
    MO__REG_FUNC("%1 polar2euclidean(const %1 &in)", vecfunc<Vec3>::polar2euclidean);

    MO__REG_FUNC("%1 closest_point_on_line(const %1 &in point, const %1 &in lineA, const %1 &in lineB)",
                                         vecfunc<Vec3>::closest_point_on_line);
    MO__REG_FUNC("bool intersect_ray_triangle(const %1 &in ray_origin, "
                                         "const %1 &in ray_direction, "
                                         "const %1 &in t0, const %1 &in t1, const %1 &in t2)",
                                         vecfunc<Vec3>::intersect_ray_triangle);
    MO__REG_FUNC("bool intersect_ray_triangle(const %1 &in ray_origin, "
                                         "const %1 &in ray_direction, "
                                         "const %1 &in t0, const %1 &in t1, const %1 &in t2, "
                                         "%1 &out pos)",
                                         vecfunc<Vec3>::intersect_ray_triangle_p);
    MO__REG_FUNC("bool intersect_ray_sphere(const %1 &in ray_origin, "
                                         "const %1 &in ray_direction, "
                                         "const %1 &in sphere_center, "
                                         "float sphere_radius, float &out depth1, float &out depth2)",
                                         vecfunc<Vec3>::intersect_ray_sphere_dd);
    MO__REG_FUNC("bool intersect_ray_sphere(const %1 &in ray_origin, "
                                         "const %1 &in ray_direction, "
                                         "const %1 &in sphere_center, "
                                         "float sphere_radius)",
                                         vecfunc<Vec3>::intersect_ray_sphere);
}


/** Registers stuff for 3 and 4 only */
template <class Vec>
void register_vector_34_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // --------------------- methods ------------------

    MO__REG_METHOD("%1 rotated(const vec3 &in, float)", vecfunc<Vec>::vecRotated);
    MO__REG_METHOD("%1 rotatedX(float)", vecfunc<Vec>::vecRotatedX);
    MO__REG_METHOD("%1 rotatedY(float)", vecfunc<Vec>::vecRotatedY);
    MO__REG_METHOD("%1 rotatedZ(float)", vecfunc<Vec>::vecRotatedZ);

    // ------------------ non-member functions --------

    MO__REG_FUNC("%1 rotate(const %1 &in, const vec3 &in axis, float degree)", vecfunc<Vec>::rotate);
    MO__REG_FUNC("%1 rotateX(const %1 &in, float)", vecfunc<Vec>::rotateX);
    MO__REG_FUNC("%1 rotateY(const %1 &in, float)", vecfunc<Vec>::rotateY);
    MO__REG_FUNC("%1 rotateZ(const %1 &in, float)", vecfunc<Vec>::rotateZ);
}

/** Specific stuff for 4 */
void register_vector_4(asIScriptEngine *engine)
{
    const char * typ = "vec4";
    int r; Q_UNUSED(r);

    // --------- constructors ----------

    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT,
        "void f(float, float, float, float)",
        asFUNCTION(vecfunc<Vec4>::vecInitConstructor4), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_LIST_CONSTRUCT,
        "void f(const int &in) { float, float, float, float }",
        asFUNCTION(vecfunc<Vec4>::vecListConstructor4), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT,
        "void f(const vec2 &in xy, float z = 0, float w = 0)",
        asFUNCTION(vecfunc<Vec4>::vecConvConstructorVec42), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT,
        "void f(const vec3 &in xyz, float w = 0)",
        asFUNCTION(vecfunc<Vec4>::vecConvConstructorVec43), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // ------------ properties ---------

    r = engine->RegisterObjectProperty(typ, "float x", asOFFSET(Vec4, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float y", asOFFSET(Vec4, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float r", asOFFSET(Vec4, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float g", asOFFSET(Vec4, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float s", asOFFSET(Vec4, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float t", asOFFSET(Vec4, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float z", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float w", asOFFSET(Vec4, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float b", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float a", asOFFSET(Vec4, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float u", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float v", asOFFSET(Vec4, w)); assert( r >= 0 );

    // --------------------- methods ------------------

    MO__REG_FUNC("%1 hsv2rgb(const %1 &in)", vecfunc<Vec4>::hsv2rgb_4);
    MO__REG_FUNC("%1 hsv2rgb(float h, float s, float v, float alpha)", vecfunc<Vec4>::hsv2rgb_4f);
    MO__REG_FUNC("%1 rgb2hsv(const %1 &in)", vecfunc<Vec4>::rgb2hsv_4);
    MO__REG_FUNC("%1 rgb2hsv(float r, float g, float b, float alpha)", vecfunc<Vec4>::rgb2hsv_4f);
    MO__REG_FUNC("%1 hsv(const %1 &in)", vecfunc<Vec4>::rgb2hsv_4);
    MO__REG_FUNC("%1 hsv(float r, float g, float b, float alpha)", vecfunc<Vec4>::rgb2hsv_4f);

    // ------------------ non-member functions --------

    MO__REG_FUNC("%1 noise4(float)", vecfunc<Vec4>::noisev4_1);
}




template <class Vec>
void register_vector_mathwrapper_tmpl(asIScriptEngine *engine, const char * typ)
{
    // creates a function 'name__'(const Vec3 &in) calling 'func__' for each component
    // XXX a bit hard on the compiler but good for us people
#define MO__MATHWRAP(name__, func__)                    \
    struct _mathwrap##name__                            \
    {                                                   \
        static Vec func(const Vec& v)                   \
        {                                               \
            Vec r;                                      \
            for (uint i=0; i<vectraits<Vec>::num; ++i)  \
                r[i] = func__(v[i]); return r;          \
        }                                               \
    };                                                  \
    MO__REG_FUNC("%1 " #name__ "(const %1 &in)",        \
        _mathwrap##name__::func);


    int r; Q_UNUSED(r);
    MO__MATHWRAP(abs, MATH::advanced<Float>::abs);
    MO__MATHWRAP(acos, MATH::advanced<Float>::acos);
    MO__MATHWRAP(acosh, MATH::advanced<Float>::acosh);
    MO__MATHWRAP(asin, MATH::advanced<Float>::asin);
    MO__MATHWRAP(asinh, MATH::advanced<Float>::asinh);
    MO__MATHWRAP(atan, MATH::advanced<Float>::atan);
    MO__MATHWRAP(atanh, MATH::advanced<Float>::atanh);
    MO__MATHWRAP(ceil, MATH::advanced<Float>::ceil);
    MO__MATHWRAP(cos, MATH::advanced<Float>::cos);
    MO__MATHWRAP(cosh, MATH::advanced<Float>::cosh);
    MO__MATHWRAP(erf, MATH::advanced<Float>::erf);
    MO__MATHWRAP(erfc, MATH::advanced<Float>::erfc);
    MO__MATHWRAP(exp, MATH::advanced<Float>::exp);
    MO__MATHWRAP(floor, MATH::advanced<Float>::floor);
    MO__MATHWRAP(frac, MATH::advanced<Float>::frac);
    MO__MATHWRAP(log, MATH::advanced<Float>::log);
    MO__MATHWRAP(log2, MATH::advanced<Float>::log2);
    MO__MATHWRAP(log10, MATH::advanced<Float>::log10);
    MO__MATHWRAP(logistic, MATH::advanced<Float>::logistic);
    MO__MATHWRAP(note2freq, MATH::advanced<Float>::note2freq_1);
    MO__MATHWRAP(ramp, MATH::advanced<Float>::ramp);
    MO__MATHWRAP(round, MATH::advanced<Float>::round);
    MO__MATHWRAP(saw, MATH::advanced<Float>::saw);
    MO__MATHWRAP(sin, MATH::advanced<Float>::sin);
    MO__MATHWRAP(sinc, MATH::advanced<Float>::sinc);
    MO__MATHWRAP(sinh, MATH::advanced<Float>::sinh);
    MO__MATHWRAP(sqrt, MATH::advanced<Float>::sqrt);
    MO__MATHWRAP(square, MATH::advanced<Float>::square);
    MO__MATHWRAP(tan, MATH::advanced<Float>::tan);
    MO__MATHWRAP(tanh, MATH::advanced<Float>::tanh);
    MO__MATHWRAP(tri, MATH::advanced<Float>::tri);
    MO__MATHWRAP(zeta, MATH::advanced<Float>::zeta);


#undef MO__MATHWRAP
}




/** Registers the three vector types */
void registerAngelScript_vector(asIScriptEngine *engine)
{
    int r; Q_UNUSED(r);

    // forward-declare types
    r = engine->RegisterObjectType("vec2", sizeof(Vec2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS);
    assert( r >= 0 );
    r = engine->RegisterObjectType("vec3", sizeof(Vec3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS);
    assert( r >= 0 );
    r = engine->RegisterObjectType("vec4", sizeof(Vec4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS);
    assert( r >= 0 );

    // ---------------- vec2 ---------------

    register_vector_tmpl<Vec2>(engine, "vec2");
    register_vector_mathwrapper_tmpl<Vec2>(engine, "vec2");
    register_vector_2(engine);

    // ---------------- vec3 ---------------

    register_vector_tmpl<Vec3>(engine, "vec3");
    register_vector_mathwrapper_tmpl<Vec3>(engine, "vec3");
    register_vector_34_tmpl<Vec3>(engine, "vec3");
    register_vector_3(engine);

    // ---------------- vec4 ---------------

    register_vector_tmpl<Vec4>(engine, "vec4");
    register_vector_mathwrapper_tmpl<Vec4>(engine, "vec4");
    register_vector_34_tmpl<Vec4>(engine, "vec4");
    register_vector_4(engine);

}





// ###################################### once again for matrix types #####################################

// mat3 and mat4 have not much in common,
// so we specialize for each type
template <class Mat>
struct matfunc
{
};

template <>
struct matfunc<Mat4>
{
    typedef typename Mat4::value_type T;

    static StringAS toString(Mat4 * self) { std::stringstream s; s << *self; return s.str(); }

    static void defaultConstructor(Mat4 *self) { new(self) Mat4(); }
    static void convConstructor(Mat4 *self, Float x) { new(self) Mat4(x); }
    static void copyConstructor(Mat4 *self, const Mat4 &other) { new(self) Mat4(other); }

    static void initConstructorF(   Mat4 * self,
                                    T const & x0, T const & y0, T const & z0, T const & w0,
                                    T const & x1, T const & y1, T const & z1, T const & w1,
                                    T const & x2, T const & y2, T const & z2, T const & w2,
                                    T const & x3, T const & y3, T const & z3, T const & w3)
    { new(self) Mat4(x0,y0,z0,w0, x1,y1,z1,w1, x2,y2,z2,w2, x3,y3,z3,w3); }

    static void initConstructorV(   Mat4 * self,
                                    Vec4 const & v0,
                                    Vec4 const & v1,
                                    Vec4 const & v2,
                                    Vec4 const & v3)
    { new(self) Mat4(v0, v1, v2, v3); }

    // vector multiplication

    static Vec4 matMulVec4(Mat4 * self, const Vec4& v) { return *self * v; }
    static Vec4 vec4MulMat(const Mat4& m, const Vec4& v) { return v * m; }
    static Vec3 matMulVec3(Mat4 * self, const Vec3& v) { return Vec3(*self * Vec4(v,1)); }
    static Vec3 vec3MulMat(const Mat4& m, const Vec3& v) { return Vec3(Vec4(v,1) * m); }

    // convenience stuff (member)
    static Mat4& mat_rotate(Mat4 * self, const Vec3& axis, Float degree) { return *self = MATH::rotate(*self, degree, axis); }
    static Mat4& mat_rotateX(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(1,0,0)); }
    static Mat4& mat_rotateY(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(0,1,0)); }
    static Mat4& mat_rotateZ(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(0,0,1)); }
    static Mat4& mat_translate(Mat4 * self, const Vec3& p) { return *self = glm::translate(*self, p); }
    static Mat4& mat_scale(Mat4 * self, const Vec3& s) { return *self = glm::scale(*self, s); }
    static Mat4& mat_scaleF(Mat4 * self, Float s) { return *self = glm::scale(*self, Vec3(s,s,s)); }

    static Vec4 getVector(Mat4 * self, int index) { return index < 4 ? (*self)[index] : Vec4(0.); }
    static Vec4& getVectorRef(Mat4 * self, int index) { return (*self)[std::min(3, index)]; }

    // convenience stuff (non-member)
    static Mat4 rotate(const Mat4& m, const Vec3& axis, Float degree) { return MATH::rotate(m, degree, axis); }
    static Mat4 rotateX(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(1,0,0)); }
    static Mat4 rotateY(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(0,1,0)); }
    static Mat4 rotateZ(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(0,0,1)); }
    static Mat4 translate(const Mat4& m, const Vec3& pos) { return glm::translate(m, pos); }
    static Mat4 scale(const Mat4& m, const Vec3& s) { return glm::scale(m, s); }
    static Mat4 scaleF(const Mat4& m, Float s) { return glm::scale(m, Vec3(s,s,s)); }

    static Mat4 inverse(const Mat4& m) { return glm::inverse(m); }
    static Mat4 transpose(const Mat4& m) { return glm::transpose(m); }

    static Mat4 ortho(Float l, Float r, Float b, Float t, Float n, Float f) { return glm::ortho(l,r,b,t,n,f); }
    static Mat4 perspective(Float fovy_degree, Float aspect, Float znear, Float zfar)
        { return MATH::perspective(fovy_degree, aspect, znear, zfar); }
};


template <>
struct matfunc<Mat3>
{
    typedef typename Mat3::value_type T;

    static StringAS toString(Mat3 * self) { std::stringstream s; s << *self; return s.str(); }

    static void defaultConstructor(Mat3 *self) { new(self) Mat3(); }
    static void convConstructor(Mat3 *self, Float x) { new(self) Mat3(x); }
    static void copyConstructor(Mat3 *self, const Mat3 &other) { new(self) Mat3(other); }

    static void initConstructorF(   Mat3 * self,
                                    T const & x0, T const & y0, T const & z0,
                                    T const & x1, T const & y1, T const & z1,
                                    T const & x2, T const & y2, T const & z2)
    { new(self) Mat3(x0,y0,z0, x1,y1,z1, x2,y2,z2); }

    static void initConstructorV(   Mat3 * self,
                                    Vec3 const & v0,
                                    Vec3 const & v1,
                                    Vec3 const & v2)
    { new(self) Mat3(v0, v1, v2); }

    // vector multiplication
    static Vec4 matMulVec4(Mat3 * self, const Vec4& v)
    {
        return Vec4((*self)[0][0] * v.x + (*self)[1][0] * v.y + (*self)[2][0] * v.z,
                    (*self)[0][1] * v.x + (*self)[1][1] * v.y + (*self)[2][1] * v.z,
                    (*self)[0][2] * v.x + (*self)[1][2] * v.y + (*self)[2][2] * v.z, v.w);
    }
    static Vec4 vec4MulMat(const Mat3& m, const Vec4& v)
    {
        return Vec4(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z, v.w);
    }
    static Vec3 matMulVec3(Mat3 * self, const Vec3& v)
    {
        return Vec3((*self)[0][0] * v.x + (*self)[1][0] * v.y + (*self)[2][0] * v.z,
                    (*self)[0][1] * v.x + (*self)[1][1] * v.y + (*self)[2][1] * v.z,
                    (*self)[0][2] * v.x + (*self)[1][2] * v.y + (*self)[2][2] * v.z);
    }
    static Vec3 vec3MulMat(const Mat3& m, const Vec3& v)
    {
        return Vec3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                    m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                    m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
    }

    // convenience stuff (member)
    // XXX two conversions: Mat3>Mat4 and back is not very cool
    // maybe reimplement these functions someday
    static Mat3& mat_rotate(Mat3 * self, const Vec3& axis, Float degree) { return *self = Mat3(matfunc<Mat4>::rotate(Mat4(*self), axis, degree)); }
    static Mat3& mat_rotateX(Mat3 * self, Float degree) { return *self = Mat3(matfunc<Mat4>::rotateX(Mat4(*self), degree)); }
    static Mat3& mat_rotateY(Mat3 * self, Float degree) { return *self = Mat3(matfunc<Mat4>::rotateY(Mat4(*self), degree)); }
    static Mat3& mat_rotateZ(Mat3 * self, Float degree) { return *self = Mat3(matfunc<Mat4>::rotateZ(Mat4(*self), degree)); }
    static Mat3& mat_scale(Mat3 * self, const Vec3& s) { return *self = Mat3(matfunc<Mat4>::scale(Mat4(*self), s)); }
    static Mat3& mat_scaleF(Mat3 * self, Float s) { return *self = Mat3(matfunc<Mat4>::scaleF(Mat4(*self), s)); }

    // convenience stuff (non-member)
    static Mat3 rotate(const Mat3& m, const Vec3& axis, Float degree) { return Mat3(MATH::rotate(Mat4(m), degree, axis)); }
    static Mat3 rotateX(const Mat3& m, Float degree) { return Mat3(MATH::rotate(Mat4(m), degree, Vec3(1,0,0))); }
    static Mat3 rotateY(const Mat3& m, Float degree) { return Mat3(MATH::rotate(Mat4(m), degree, Vec3(0,1,0))); }
    static Mat3 rotateZ(const Mat3& m, Float degree) { return Mat3(MATH::rotate(Mat4(m), degree, Vec3(0,0,1))); }
    static Mat3 scale(const Mat3& m, const Vec3& s) { return Mat3(glm::scale(Mat4(m), s)); }
    static Mat3 scaleF(const Mat3& m, Float s) { return Mat3(glm::scale(Mat4(m), Vec3(s,s,s))); }

    static Mat4 inverse(const Mat4& m) { return glm::inverse(m); }
    static Mat4 transpose(const Mat4& m) { return glm::transpose(m); }
};


/** Registers constructors, functions and operators common to all matrix types. */
template <class Mat>
void register_matrix_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r; Q_UNUSED(r);

    // ----------- object properties ------------

//    r = engine->RegisterObjectProperty(typ, "float x", asOFFSET(Vec, x)); assert( r >= 0 );

    // ------------- constructors ---------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
            ("void f()"),
            asFUNCTION(matfunc<Mat>::defaultConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
            MO__STR("void f(const %1 &in)"),
            asFUNCTION(matfunc<Mat>::copyConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
            ("void f(float)"),
            asFUNCTION(matfunc<Mat>::convConstructor), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // -------------- operator overloads ---------------

    MO__REG_METHoD("string opImplConv() const", matfunc<Mat>::toString);

    MO__REG_TRUE_METHOD("%1 &opAddAssign(const %1 &in)", asMETHODPR(Mat, operator+=, (const Mat &), Mat&));
    MO__REG_TRUE_METHOD("%1 &opSubAssign(const %1 &in)", asMETHODPR(Mat, operator-=, (const Mat &), Mat&));
    MO__REG_TRUE_METHOD("%1 &opMulAssign(const %1 &in)", asMETHODPR(Mat, operator*=, (const Mat &), Mat&));
    MO__REG_TRUE_METHOD("%1 &opDivAssign(const %1 &in)", asMETHODPR(Mat, operator/=, (const Mat &), Mat&));

    MO__REG_TRUE_METHOD("%1 &opAddAssign(float)", asMETHODPR(Mat, operator+=, (float), Mat&));
    MO__REG_TRUE_METHOD("%1 &opSubAssign(float)", asMETHODPR(Mat, operator-=, (float), Mat&));
    MO__REG_TRUE_METHOD("%1 &opMulAssign(float)", asMETHODPR(Mat, operator*=, (float), Mat&));
    MO__REG_TRUE_METHOD("%1 &opDivAssign(float)", asMETHODPR(Mat, operator/=, (float), Mat&));

    MO__REG_METHoD("vec3 opMul(const vec3& in) const", matfunc<Mat>::matMulVec3);
    MO__REG_METHoD("vec4 opMul(const vec4& in) const", matfunc<Mat>::matMulVec4);
    MO__REG_FUNC("vec3 opMul(const vec3& in, const %1 &in)", matfunc<Mat>::vec3MulMat);
    MO__REG_FUNC("vec4 opMul(const vec4& in, const %1 &in)", matfunc<Mat>::vec4MulMat);

    // ------------ members -------------------

    MO__REG_METHOD("%1& rotate(const vec3 &in axis, float degree)", matfunc<Mat4>::mat_rotate);
    MO__REG_METHOD("%1& rotateX(float degree)", matfunc<Mat4>::mat_rotateX);
    MO__REG_METHOD("%1& rotateY(float degree)", matfunc<Mat4>::mat_rotateY);
    MO__REG_METHOD("%1& rotateZ(float degree)", matfunc<Mat4>::mat_rotateZ);
    MO__REG_METHOD("%1& scale(const vec3 &in)", matfunc<Mat4>::mat_scale);
    MO__REG_METHOD("%1& scale(float)", matfunc<Mat4>::mat_scaleF);

    // ---- non-members -------

    MO__REG_FUNC("%1 rotate(const %1 &in, const vec3 &in axis, float degree)", matfunc<Mat4>::rotate);
    MO__REG_FUNC("%1 rotateX(const %1 &in, float degree)", matfunc<Mat4>::rotateX);
    MO__REG_FUNC("%1 rotateY(const %1 &in, float degree)", matfunc<Mat4>::rotateY);
    MO__REG_FUNC("%1 rotateZ(const %1 &in, float degree)", matfunc<Mat4>::rotateZ);
    MO__REG_FUNC("%1 scale(const %1 &in, const vec3 &in)", matfunc<Mat4>::scale);
    MO__REG_FUNC("%1 scale(const %1 &in, float)", matfunc<Mat4>::scaleF);

    MO__REG_FUNC("%1 inverse(const %1 &in)", matfunc<Mat>::inverse);
    MO__REG_FUNC("%1 transpose(const %1 &in)", matfunc<Mat>::transpose);
}


/** Stuff for 3x3 matrix only */
void register_matrix3(asIScriptEngine * engine)
{
    const char * typ = "mat3";
    int r; Q_UNUSED(r);

    // -------- constructors ----------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(float, float, float, float, float, float, float, float, float)"),
        asFUNCTION(matfunc<Mat3>::initConstructorF), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(const vec3 &in column0, const vec3 &in column1, const vec3 &in column2)"),
        asFUNCTION(matfunc<Mat3>::initConstructorV), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // -------- member funcs ---------------------


    // -------- non-member operators -------------


    // -------- non-member funcs -----------------

}


/** Stuff for 4x4 matrix only */
void register_matrix4(asIScriptEngine * engine)
{
    const char * typ = "mat4";
    int r; Q_UNUSED(r);

    // -------- constructors ----------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)"),
        asFUNCTION(matfunc<Mat4>::initConstructorF), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(const vec4 &in column0, const vec4 &in column1, const vec4 &in column2, const vec4 &in column3)"),
        asFUNCTION(matfunc<Mat4>::initConstructorV), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // -------- member funcs ---------------------

    MO__REG_METHOD("%1& translate(const vec3 &in)", matfunc<Mat4>::mat_translate);
    //MO__REG_METHOD("vec4& vec(uint index)", matfunc<Mat4>::getVector);
    MO__REG_METHoD("vec4& vec(uint index)", matfunc<Mat4>::getVectorRef);

    // -------- operators -------------


    // -------- non-member funcs -----------------

    MO__REG_FUNC("%1 translate(const %1 &in, const vec3 &in)", matfunc<Mat4>::translate);

    MO__REG_FUNC("%1 ortho(float left, float right, float bottom, float top, float znear, float zfar)", matfunc<Mat4>::ortho);
    MO__REG_FUNC("%1 perspective(float fovy_degree, float aspect, float znear, float zfar)", matfunc<Mat4>::perspective);
}

void registerAngelScript_matrix(asIScriptEngine * engine)
{
    int r; Q_UNUSED(r);

    // forward-declare the types
    r = engine->RegisterObjectType("mat3", sizeof(Mat3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS);
    assert( r >= 0 );
    r = engine->RegisterObjectType("mat4", sizeof(Mat4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS);
    assert( r >= 0 );


    register_matrix_tmpl<Mat3>(engine, "mat3");
    register_matrix3(engine);
    register_matrix_tmpl<Mat4>(engine, "mat4");
    register_matrix4(engine);
}







#undef MO__REF_FuNC
#undef MO__REG_FUNC
#undef MO__REG_METHOD
#undef MO__REG_METHoD
#undef MO__STR

} // namespace native
} // namespace


void registerAngelScript_vector(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        assert(!"vector & matrix for AngelScript definitely not supported on this platform");
    }
    else
    {
        native::registerAngelScript_vector(engine);
        native::registerAngelScript_matrix(engine);
    }
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
