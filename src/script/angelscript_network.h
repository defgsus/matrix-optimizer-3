/** @file angelscript_network.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_NETWORK_H
#define MOSRC_SCRIPT_ANGELSCRIPT_NETWORK_H

class asIScriptEngine;

namespace MO {

class UdpConnection;

/** AngelScript wrapper */
class UdpConnectionAS;

/** Put the network related types and functions into the namespace.
    Dependency: string */
void registerAngelScript_network(asIScriptEngine *engine);

/** Make a connection object accessible to the script.
    If you want to pass ownership to the script then release your reference on @p con after the call. */
//void registerAngelScript_connection(asIScriptEngine * engine, UdpConnection * con, bool writeable);

} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_NETWORK_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
