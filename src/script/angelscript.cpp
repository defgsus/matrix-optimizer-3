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
