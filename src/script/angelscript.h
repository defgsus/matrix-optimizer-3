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

/** Registers default types and functions.
    Mainly mathematics */
void registerDefaultAngelscript(asIScriptEngine *);

} // namespace MO

#endif // MOSRC_SCRIPT_ANGELSCRIPT_H


#endif // #ifndef MO_DISABLE_ANGELSCRIPT
