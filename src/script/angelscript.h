/** @file angelscript.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_H
#define MOSRC_SCRIPT_ANGELSCRIPT_H

#include <angelscript.h>

namespace MO {

typedef std::string AngelScriptString;

/** Registers default types and functions.
    Mainly mathematics */
void registerDefaultAngelscript(asIScriptEngine *);

class AngelScriptAutoPtr
{
public:
    /** Binds a module for DiscardModule() */
    AngelScriptAutoPtr(asIScriptEngine * e, asIScriptModule*m)
        : p_engine  (e),
          p_module  (m),
          p_context (0)
    { }

    /** Binds a module for DiscardModule() */
    AngelScriptAutoPtr(asIScriptContext * c)
        : p_engine  (0),
          p_module  (0),
          p_context (c)
    { }

    ~AngelScriptAutoPtr()
    {
        if (p_module && p_engine)
            p_engine->DiscardModule(p_module->GetName());
        if (p_context)
            p_context->Release();
    }

private:
    asIScriptEngine * p_engine;
    asIScriptModule * p_module;
    asIScriptContext * p_context;
};


} // namespace MO

#endif // MOSRC_SCRIPT_ANGELSCRIPT_H


#endif // #ifndef MO_DISABLE_ANGELSCRIPT
