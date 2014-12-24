/** @file angelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include <cassert>

#include <QString>

#include "angelscript.h"
#include "angelscript_vector.h"
#include "angelscript_math.h"
#include "angelscript_object.h"
#include "angelscript_geometry.h"
#include "3rd/angelscript/scriptmath/scriptmathcomplex.h"
#include "3rd/angelscript/scriptarray/scriptarray.h"
#include "3rd/angelscript/scriptstdstring/scriptstdstring.h"
#include "io/log.h"

namespace MO {



QString toString(const StringAS& as)
{
    return QString::fromUtf8(as.c_str());
}

StringAS toStringAS(const QString& s)
{
    return s.toUtf8().constData();
}










namespace {

    void angelPrint(const StringAS& s)
    {
        MO_DEBUG("angelscript: " << toString(s));
    }

} // namespace

void registerDefaultAngelscript(asIScriptEngine * engine)
{
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptMathComplex(engine);

    registerAngelScript_math(engine);
    registerAngelScript_vector(engine);
    registerAngelScript_object(engine);
    registerAngelScript_geometry(engine);


    //if (object)
    //    registerAngelScript_rootObject(engine, object);

    int r = engine->RegisterGlobalFunction("void print(const string & in)", asFUNCTION(angelPrint), asCALL_CDECL); assert( r >= 0 );
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT




/**************************************************************************

    // --- some working examples ---


void dumpTree(Object@ o, string pre = "")
{
    print(pre + o.name());
    pre += " ";
    for (int i=0; i<o.childrenCount(); ++i)
        dumpTree(o.children(i), pre);
}

void testObject()
{
    Object@ o = Root;
    print(o.name() + " " + o + " " + o.childrenCount());
    if (o.childrenCount() >= 1)
        print(o.children(0));

    dumpTree(Root);
}


void printArray(const array<int> &in a)
{
    string s;
    for (uint i = 0; i < a.length(); ++i)
        s += a[i] + " ";
    print(s);
}

void printArray(const array<vec3> &in a)
{
    string s;
    for (uint i = 0; i < a.length(); ++i)
        s += " " + a[i];
    print(s);
}

void testArray()
{
    array<int> a = { 5, 4, 3, 2, 1 };

    print(a.toString());

    a.resize(23);
    print(a.toString());

    array<string> as = { "hallo", "welt" };
    print(as.toString());

    array<vec3> b = { vec3(1), vec3(2) };
    print(b.toString());

    array<array<int>> aa = { a, a };
    print(aa.toString());
}

void testVec()
{
    vec2 v2(1,2);
    vec3 v3(3,4,5);
    vec4 v4(6,7,8,9);
    print(""+v2+" "+v3+" "+v4);
    v2 = v2 + 1.f;
    v3 = v3 + 1.f;
    v4 = v4 + 1.f;
    print(""+v2+" "+v3+" "+v4);
    print(""+smallest(v2)+" "+smallest(v3)+" "+smallest(v4));
    print(""+largest(v2)+" "+largest(v3)+" "+largest(v4));

    print(rgb2hsv(vec3(0.5,0.2,0.2)));
    print(hsv2rgb(vec3(0.8,1,1)));
}

void testGeometry()
{
    Geometry g;
    Geometry@ g2 = g;
    print(""+g + " " + g2);
    g.addLine(vec3(0,0,0), vec3(1,1,1));
    print(""+g.vertexCount());
}

void main()
{
    print("----------------------------");

    testArray();
    //testVec();
}


*************************************************************************/
