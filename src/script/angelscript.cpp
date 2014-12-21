/** @file angelscript.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#include "angelscript.h"
#include "angelscript_vector.h"
#include "angelscript_math.h"
//#include "3rd/angelscript/scriptmath.h"
#include "3rd/angelscript/scriptmathcomplex.h"

namespace MO {

/** Registers default types and functions.
    Mainly mathematics */
void registerDefaultAngelscript(asIScriptEngine * engine)
{
    //RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);
    registerAngelScript_math(engine);
    registerAngelScript_vector(engine);
}

} // namespace MO



#endif // #ifndef MO_DISABLE_ANGELSCRIPT
