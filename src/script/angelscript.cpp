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

namespace {

    void angelPrint(const std::string& s)
    {
        MO_DEBUG("angelscript: " << QString::fromUtf8(&s[0]));
    }

} // namespace

void registerDefaultAngelscript(asIScriptEngine * engine)
{
    RegisterScriptArray(engine, true);
    RegisterStdString(engine);
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

    void testArray()
    {
        array<int> a = { 5, 4, 3, 2, 1 };

        for (uint i = 0; i < a.length(); ++i)
            print(""+i);
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

        testVec();
    }


*************************************************************************/
