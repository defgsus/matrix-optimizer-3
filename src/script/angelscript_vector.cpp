/** @file anglescript_vector.cpp

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

/** @todo NEEDS ALL BE TESTED PROPERLY */


namespace MO {

namespace {

template <class V> struct vectraits;
template <>
struct vectraits<Vec2> { static const int num = 2; };
template <>
struct vectraits<Vec3> { static const int num = 3; };
template <>
struct vectraits<Vec4> { static const int num = 4; };


namespace native {

//-----------------------
// AngelScript functions
//-----------------------

/** Wraps some functions and makes them non-overloading
    to avoid nasty syntax when registering */
template <typename Vec>
struct vecfunc
{
    static StringAS VecToString(Vec * self) { std::stringstream s; s << *self; return s.str(); }

    static void VecDefaultConstructor(Vec *self) { new(self) Vec(); }
    static void VecConvConstructor(Float x, Vec *self) { new(self) Vec(x); }
    static void VecCopyConstructor(const Vec &other, Vec *self) { new(self) Vec(other); }
    //static void VecCopyConstructor32(const Vec3 &other, Vec2 *self) { new(self) Vec3(other); }
    static void VecListConstructor2(Float *list, Vec2 *self) { new(self) Vec2(list[0], list[1]); }
    static void VecListConstructor3(Float *list, Vec3 *self) { new(self) Vec3(list[0], list[1], list[2]); }
    static void VecListConstructor4(Float *list, Vec4 *self) { new(self) Vec4(list[0], list[1], list[2], list[3]); }

    static void VecInitConstructor2(Float x, Float y, Vec2 *self) { new(self) Vec2(x,y); }
    static void VecInitConstructor3(Float x, Float y, Float z, Vec3 *self) { new(self) Vec3(x,y,z); }
    static void VecInitConstructor4(Float x, Float y, Float z, Float w, Vec4 *self) { new(self) Vec4(x,y,z,w); }

    static Vec& assignFloat(Vec * self, Float f) { *self = Vec(f); return *self; }

    static bool VecEqualsVec(Vec * self, const Vec& v) { return *self == v; }
    static Vec VecAddVec(Vec * self, const Vec& v) { return *self + v; }
    static Vec VecSubVec(Vec * self, const Vec& v) { return *self - v; }
    static Vec VecMulVec(Vec * self, const Vec& v) { return *self * v; }
    static Vec VecDivVec(Vec * self, const Vec& v) { return *self / v; }

    static Vec VecAddFloat(Vec * self, Float v) { return *self + v; }
    static Vec VecSubFloat(Vec * self, Float v) { return *self - v; }
    static Vec VecMulFloat(Vec * self, Float v) { return *self * v; }
    static Vec VecDivFloat(Vec * self, Float v) { return *self / v; }

    static Vec VecRotated(Vec * self, const Vec3& a, Float deg) { return MATH::rotate(*self, a, deg); }
    static Vec VecRotatedX(Vec * self, Float deg) { return MATH::rotateX(*self, deg); }
    static Vec VecRotatedY(Vec * self, Float deg) { return MATH::rotateY(*self, deg); }
    static Vec VecRotatedZ(Vec * self, Float deg) { return MATH::rotateZ(*self, deg); }

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
    static Vec abs(const Vec& v) { return glm::abs(v); }

    static Vec mix(const Vec& a, const Vec& b, float x) { return glm::mix(a, b, x); }


    static Float length(const Vec& v) { return glm::length(v); }
    static Float distance(const Vec& v, const Vec& n) { return glm::distance(v, n); }
    static Float dot(const Vec& a, const Vec& b) { return glm::dot(a, b); }

    static Float smallest(const Vec& v)
        { Float m = v[0]; for (int i=1; i<vectraits<Vec>::num; ++i) m = std::min(m, v[i]); return m; }
    static Float largest(const Vec& v)
        { Float m = v[0]; for (int i=1; i<vectraits<Vec>::num; ++i) m = std::max(m, v[i]); return m; }

    static Float noise(const Vec3& v) { return MATH::advanced<float>::noise_3(v.x, v.y, v.z); }

    // ------- color conv -------

    static Vec toVecRgb3(const QColor& c) { return Vec(c.redF(), c.greenF(), c.blueF()); }
    static Vec toVecHsv3(const QColor& c) { return Vec(c.hueF(), c.saturationF(), c.valueF()); }
    static Vec hsv2rgb_3(const Vec& c) { return toVecRgb3(QColor::fromHsvF(MATH::moduloSigned(c.x, Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)))); }
    static Vec rgb2hsv_3(const Vec& c) { return toVecHsv3(QColor::fromRgbF(glm::clamp(c.x, Float(0), Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)))); }

    static Vec4 toVecRgb4(const QColor& c) { return Vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }
    static Vec4 toVecHsv4(const QColor& c) { return Vec4(c.hueF(), c.saturationF(), c.valueF(), c.alphaF()); }
    static Vec4 hsv2rgb_4(const Vec4& c) { return toVecRgb4(QColor::fromHsvF(MATH::moduloSigned(c.x, Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)),
                                                                           glm::clamp(c.w, Float(0), Float(1)))); }
    static Vec4 rgb2hsv_4(const Vec4& c) { return toVecHsv4(QColor::fromRgbF(glm::clamp(c.x, Float(0), Float(1)),
                                                                           glm::clamp(c.y, Float(0), Float(1)),
                                                                           glm::clamp(c.z, Float(0), Float(1)),
                                                                           glm::clamp(c.w, Float(0), Float(1)))); }

};

//--------------------------------
// Registration
//-------------------------------------

// replaces %1 with typ and returns const char*
#define MO__STR(str__) (QString(str__).arg(typ).toUtf8().constData())

// to register one of the above statics as class method
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


/** Registers constructors, functions and operators common to all vec types (e.g. 2 - 4). */
template <class Vec>
void register_vector_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r;

    // Register the type
    r = engine->RegisterObjectType(typ, sizeof(Vec), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );

    // ----------- object properties ------------

    r = engine->RegisterObjectProperty(typ, "float x", asOFFSET(Vec, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float y", asOFFSET(Vec, y)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float z", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float w", asOFFSET(Vec, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float r", asOFFSET(Vec, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float g", asOFFSET(Vec, y)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float b", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float a", asOFFSET(Vec, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float s", asOFFSET(Vec, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty(typ, "float t", asOFFSET(Vec, y)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float u", asOFFSET(Vec, z)); assert( r >= 0 );
    //r = engine->RegisterObjectProperty(typ, "float v", asOFFSET(Vec, w)); assert( r >= 0 );

    // ------------- constructors ---------------------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,             ("void f()"),                      asFUNCTION(vecfunc<Vec>::VecDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,      MO__STR("void f(const %1 &in)"),          asFUNCTION(vecfunc<Vec>::VecCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,             ("void f(float)"),                 asFUNCTION(vecfunc<Vec>::VecConvConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // -------------- operator overloads ---------------

    MO__REG_METHoD("string opImplConv() const", vecfunc<Vec>::VecToString);

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

    MO__REG_METHOD("%1 opAdd(const %1 &in) const", vecfunc<Vec>::VecAddVec);
    MO__REG_METHOD("%1 opSub(const %1 &in) const", vecfunc<Vec>::VecSubVec);
    MO__REG_METHOD("%1 opMul(const %1 &in) const", vecfunc<Vec>::VecMulVec);
    MO__REG_METHOD("%1 opDiv(const %1 &in) const", vecfunc<Vec>::VecDivVec);

    MO__REG_METHOD("%1 opAdd(float) const", vecfunc<Vec>::VecAddFloat);
    MO__REG_METHOD("%1 opSub(float) const", vecfunc<Vec>::VecSubFloat);
    MO__REG_METHOD("%1 opMul(float) const", vecfunc<Vec>::VecMulFloat);
    MO__REG_METHOD("%1 opDiv(float) const", vecfunc<Vec>::VecDivFloat);

    // ---------- methods -----------------------


    // ------ non-member object functions -------

    //MO__REG_FUNC("%1 cross(const %1 &in, const %1& in)", vecfunc<Vec>::cross);
    MO__REG_FUNC("%1 reflect(const %1 &in, const %1 &in)", vecfunc<Vec>::reflect);
    MO__REG_FUNC("%1 refract(const %1 &in, const %1 &in, float)", vecfunc<Vec>::refract);

    MO__REG_FUNC("%1 abs(const %1 &in)", vecfunc<Vec>::abs);
    MO__REG_FUNC("%1 min(const %1 &in, const %1 &in)", vecfunc<Vec>::min_vv);
    MO__REG_FUNC("%1 min(const %1 &in, float)", vecfunc<Vec>::min_vf);
    MO__REG_FUNC("%1 min(float, const %1 &in)", vecfunc<Vec>::min_fv);
    MO__REG_FUNC("%1 max(const %1 &in, const %1 &in)", vecfunc<Vec>::max_vv);
    MO__REG_FUNC("%1 max(const %1 &in, float)", vecfunc<Vec>::max_vf);
    MO__REG_FUNC("%1 max(float, const %1 &in)", vecfunc<Vec>::max_fv);
    MO__REG_FUNC("%1 clamp(const %1 &in, float, float)", vecfunc<Vec>::clamp);
    MO__REG_FUNC("%1 mix(const %1 &in, const %1 &in, float t)", vecfunc<Vec>::mix);


    MO__REG_FUNC("float dot(const %1 &in)", vecfunc<Vec>::dot);
    MO__REG_FUNC("float length(const %1 &in)", vecfunc<Vec>::length);
    MO__REG_FUNC("float distance(const %1 &in, const %1 &in)", vecfunc<Vec>::distance);

    MO__REG_FUNC("float smallest(const %1 &in)", vecfunc<Vec>::smallest);
    MO__REG_FUNC("float largest(const %1 &in)", vecfunc<Vec>::largest);

    MO__REG_FUNC("float noise(const %1 &in)", vecfunc<Vec>::noise);

}

/** Specific stuff for 2 */
void register_vector_2(asIScriptEngine *engine, const char * typ = "vec2")
{
    int r;

    // ---------- constructors ----------
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_CONSTRUCT,      ("void f(float, float)"),
                                        asFUNCTION(vecfunc<Vec2>::VecInitConstructor2), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec2", asBEHAVE_LIST_CONSTRUCT, ("void f(const int &in) {float, float}"),
                                        asFUNCTION(vecfunc<Vec2>::VecListConstructor2), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // --------------------- methods ------------------
    MO__REG_METHOD("%1 rotated(float)", vecfunc<Vec2>::VecRotatedZ);
    MO__REG_METHOD("%1 rotatedZ(float)", vecfunc<Vec2>::VecRotatedZ);

    // ------------------ non-member functions --------
    MO__REG_FUNC("%1 rotate(%1 &in, float)", vecfunc<Vec2>::rotateZ);
    MO__REG_FUNC("%1 rotateZ(%1 &in, float)", vecfunc<Vec2>::rotateZ);
}

/** Specific stuff for 3 */
void register_vector_3(asIScriptEngine *engine)
{
    const char * typ = "vec3";
    int r;

    // --------- constructors ----------

    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_CONSTRUCT,      ("void f(float, float, float)"),
                                        asFUNCTION(vecfunc<Vec3>::VecInitConstructor3), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec3", asBEHAVE_LIST_CONSTRUCT, ("void f(const int &in) {float, float, float}"),
                                            asFUNCTION(vecfunc<Vec3>::VecListConstructor3), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // ------------ properties ---------

    r = engine->RegisterObjectProperty("vec3", "float z", asOFFSET(Vec3, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float b", asOFFSET(Vec3, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec3", "float u", asOFFSET(Vec3, z)); assert( r >= 0 );

    // --------------------- methods ------------------

    // ------------------ non-member functions --------

    MO__REG_FUNC("%1 hsv2rgb(const %1 &in)", vecfunc<Vec3>::hsv2rgb_3);
    MO__REG_FUNC("%1 rgb2hsv(const %1 &in)", vecfunc<Vec3>::rgb2hsv_3);
    MO__REG_FUNC("%1 cross(const %1 &in, const %1 &in)", vecfunc<Vec3>::cross);
}


/** Registers stuff for 3 and 4 only */
template <class Vec>
void register_vector_34_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r;

    // --------------------- methods ------------------

    MO__REG_METHOD("%1 rotated(const vec3 &in, float)", vecfunc<Vec>::VecRotated);
    MO__REG_METHOD("%1 rotatedX(float)", vecfunc<Vec>::VecRotatedX);
    MO__REG_METHOD("%1 rotatedY(float)", vecfunc<Vec>::VecRotatedY);
    MO__REG_METHOD("%1 rotatedZ(float)", vecfunc<Vec>::VecRotatedZ);

    // ------------------ non-member functions --------

    MO__REG_FUNC("%1 rotate(const %1 &in, const vec3 &in axis, float degree)", vecfunc<Vec>::rotate);
    MO__REG_FUNC("%1 rotateX(const %1 &in, float)", vecfunc<Vec>::rotateX);
    MO__REG_FUNC("%1 rotateY(const %1 &in, float)", vecfunc<Vec>::rotateY);
    MO__REG_FUNC("%1 rotateZ(const %1 &in, float)", vecfunc<Vec>::rotateZ);
    MO__REG_FUNC("%1 normalize(const %1 &in)", vecfunc<Vec>::normalize);
}

/** Specific stuff for 4 */
void register_vector_4(asIScriptEngine *engine)
{
    const char * typ = "vec4";
    int r;

    // --------- constructors ----------

    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT,      ("void f(float, float, float, float)"),
                                        asFUNCTION(vecfunc<Vec4>::VecInitConstructor4), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("vec4", asBEHAVE_LIST_CONSTRUCT, ("void f(const int &in) {float, float, float, float }"),
                                        asFUNCTION(vecfunc<Vec4>::VecListConstructor4), asCALL_CDECL_OBJLAST); assert( r >= 0 );

    // ------------ properties ---------

    r = engine->RegisterObjectProperty("vec4", "float z", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float w", asOFFSET(Vec4, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float b", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float a", asOFFSET(Vec4, w)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float u", asOFFSET(Vec4, z)); assert( r >= 0 );
    r = engine->RegisterObjectProperty("vec4", "float v", asOFFSET(Vec4, w)); assert( r >= 0 );

    // --------------------- methods ------------------

    MO__REG_FUNC("%1 hsv2rgb(const %1 &in)", vecfunc<Vec3>::hsv2rgb_4);
    MO__REG_FUNC("%1 rgb2hsv(const %1 &in)", vecfunc<Vec3>::rgb2hsv_4);

    // ------------------ non-member functions --------

}





/** Registers the three vector types */
void registerAngelScript_vector(asIScriptEngine *engine)
{
    // ---------------- vec2 ---------------

    register_vector_tmpl<Vec2>(engine, "vec2");
    register_vector_2(engine);

    // ---------------- vec3 ---------------

    register_vector_tmpl<Vec3>(engine, "vec3");
    register_vector_34_tmpl<Vec3>(engine, "vec3");
    register_vector_3(engine);


    // ---------------- vec4 ---------------

    register_vector_tmpl<Vec4>(engine, "vec4");
    register_vector_34_tmpl<Vec4>(engine, "vec4");
    register_vector_4(engine);

}





// ###################################### once again for matrix types #####################################


template <class Mat>
struct matfunc
{
    typedef typename Mat::value_type T;

    static StringAS toString(Mat * self) { std::stringstream s; s << *self; return s.str(); }

    static void defaultConstructor(Mat *self) { new(self) Mat(); }
    static void convConstructor(Mat *self, Float x) { new(self) Mat(x); }
    static void copyConstructor(Mat *self, const Mat &other) { new(self) Mat(other); }

    static void initConstructor4F(  Mat4 * self,
                                    T const & x0, T const & y0, T const & z0, T const & w0,
                                    T const & x1, T const & y1, T const & z1, T const & w1,
                                    T const & x2, T const & y2, T const & z2, T const & w2,
                                    T const & x3, T const & y3, T const & z3, T const & w3)
    { new(self) Mat4(x0,y0,z0,w0, x1,y1,z1,w1, x2,y2,z2,w2, x3,y3,z3,w3); }

    static void initConstructor4V(  Mat4 * self,
                                    Vec4 const & v0,
                                    Vec4 const & v1,
                                    Vec4 const & v2,
                                    Vec4 const & v3)
    { new(self) Mat4(v0, v1, v2, v3); }

    // vector multiplication

    static Vec4 mat4MulVec4(Mat4 * self, const Vec4& v) { return *self * v; }
    static Vec4 vec4MulMat4(const Mat4& m, const Vec4& v) { return v * m; }

    // convenience stuff (member)
    static Mat4& mat_rotate4(Mat4 * self, const Vec3& axis, Float degree) { return *self = MATH::rotate(*self, degree, axis); }
    static Mat4& mat_rotateX4(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(1,0,0)); }
    static Mat4& mat_rotateY4(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(0,1,0)); }
    static Mat4& mat_rotateZ4(Mat4 * self, Float degree) { return *self = MATH::rotate(*self, degree, Vec3(0,0,1)); }
    static Mat4& mat_translate4(Mat4 * self, const Vec3& p) { return *self = glm::translate(*self, p); }
    static Mat4& mat_scale4(Mat4 * self, const Vec3& s) { return *self = glm::scale(*self, s); }
    static Mat4& mat_scaleF4(Mat4 * self, Float s) { return *self = glm::scale(*self, Vec3(s,s,s)); }


    // convenience stuff (non-member)
    static Mat4 rotate4(const Mat4& m, const Vec3& axis, Float degree) { return MATH::rotate(m, degree, axis); }
    static Mat4 rotateX4(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(1,0,0)); }
    static Mat4 rotateY4(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(0,1,0)); }
    static Mat4 rotateZ4(const Mat4& m, Float degree) { return MATH::rotate(m, degree, Vec3(0,0,1)); }
    static Mat4 translate4(const Mat4& m, const Vec3& pos) { return glm::translate(m, pos); }
    static Mat4 scale4(const Mat4& m, const Vec3& s) { return glm::scale(m, s); }
    static Mat4 scaleF4(const Mat4& m, Float s) { return glm::scale(m, Vec3(s,s,s)); }
};


/** Registers constructors, functions and operators common to all matrix types. */
template <class Mat>
void register_matrix_tmpl(asIScriptEngine *engine, const char * typ)
{
    int r;

    // Register the type
    r = engine->RegisterObjectType(typ, sizeof(Mat), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );

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
}

/** Stuff for 4x4 matrix only */
void register_matrix4(asIScriptEngine * engine)
{
    const char * typ = "mat4";
    int r;

    // -------- constructors ----------

    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)"),
        asFUNCTION(matfunc<Mat4>::initConstructor4F), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour(typ, asBEHAVE_CONSTRUCT,
        ("void f(const vec4 &in column0, const vec4 &in column1, const vec4 &in column2, const vec4 &in column3)"),
        asFUNCTION(matfunc<Mat4>::initConstructor4V), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

    // -------- member funcs ---------------------

    MO__REG_METHOD("%1& rotate(const vec3 &in axis, float degree)", matfunc<Mat4>::mat_rotate4);
    MO__REG_METHOD("%1& rotateX(float degree)", matfunc<Mat4>::mat_rotateX4);
    MO__REG_METHOD("%1& rotateY(float degree)", matfunc<Mat4>::mat_rotateY4);
    MO__REG_METHOD("%1& rotateZ(float degree)", matfunc<Mat4>::mat_rotateZ4);
    MO__REG_METHOD("%1& translate(const vec3 &in)", matfunc<Mat4>::mat_translate4);
    MO__REG_METHOD("%1& scale(const vec3 &in)", matfunc<Mat4>::mat_scale4);
    MO__REG_METHOD("%1& scale(float)", matfunc<Mat4>::mat_scaleF4);

    // -------- non-member operators -------------

    MO__REG_METHOD("vec4 opMul(const vec4& in)", matfunc<Mat4>::mat4MulVec4);
    MO__REG_FUNC("vec4 opMul(const vec4& in, const %1 &in)", matfunc<Mat4>::vec4MulMat4);

    // -------- non-member funcs -----------------

    MO__REG_FUNC("%1 rotate(const %1 &in, const vec3 &in axis, float degree)", matfunc<Mat4>::rotate4);
    MO__REG_FUNC("%1 rotateX(const %1 &in, float degree)", matfunc<Mat4>::rotateX4);
    MO__REG_FUNC("%1 rotateY(const %1 &in, float degree)", matfunc<Mat4>::rotateY4);
    MO__REG_FUNC("%1 rotateZ(const %1 &in, float degree)", matfunc<Mat4>::rotateZ4);
    MO__REG_FUNC("%1 translate(const %1 &in, const vec3 &in)", matfunc<Mat4>::translate4);
    MO__REG_FUNC("%1 scale(const %1 &in, const vec3 &in)", matfunc<Mat4>::scale4);
    MO__REG_FUNC("%1 scale(const %1 &in, float)", matfunc<Mat4>::scaleF4);

}

void registerAngelScript_matrix(asIScriptEngine * engine)
{
    register_matrix_tmpl<Mat4>(engine, "mat4");

    register_matrix4(engine);
}








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
        assert(!"vector for Angelscript currently not supported on this platform");
    }
    else
    {
        native::registerAngelScript_vector(engine);
        native::registerAngelScript_matrix(engine);
    }
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
