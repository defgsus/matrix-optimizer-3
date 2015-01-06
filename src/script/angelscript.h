/** @file angelscript.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_H
#define MOSRC_SCRIPT_ANGELSCRIPT_H

#include <string>
#include <angelscript.h>
#include <QString>

namespace MO {

class Object;

/** The utf8 string type currently used in communication with angelscript */
typedef std::string StringAS;

QString toString(const StringAS&);
StringAS toStringAS(const QString&);

/** Registers all default types and functions. */
void registerDefaultAngelScript(asIScriptEngine *);

/** Exports all functions available to a xml file */
void exportAngelScriptFunctions(const QString& filename);

class AngelScriptAutoPtr
{
public:
    /** Binds a module for DiscardModule() */
    AngelScriptAutoPtr(asIScriptEngine * e, asIScriptModule*m)
        : p_engine  (e),
          p_module  (m),
          p_context (0)
    { }

    /** Binds an engine for Release() */
    AngelScriptAutoPtr(asIScriptEngine * e)
        : p_engine  (e),
          p_module  (0),
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
        if (p_engine)
            p_engine->Release();
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
