/** @file angelscript_image.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.01.2015</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H
#define MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H

class asIScriptEngine;

namespace MO {

/** Put the image related types and functions into the namespace.
    Dependency: vector, math */
void registerAngelScript_image(asIScriptEngine *engine);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
