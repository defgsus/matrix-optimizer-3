/** @file angelscript_geometry.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H
#define MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H

class asIScriptEngine;

namespace MO {

class Object;
class Scene;

/** Put the object type and related functions into the namespace */
void registerAngelScript_geometry(asIScriptEngine *engine);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_GEOMETRY_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
