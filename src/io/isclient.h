/** @file isclient.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/28/2014</p>
*/

#ifndef MOSRC_IO_ISCLIENT_H
#define MOSRC_IO_ISCLIENT_H

namespace MO {

/** Returns true when the programm runs in client mode */
bool isClient();
/** Returns true when the programm runs in server mode */
bool isServer();


#ifndef MO_DISABLE_CLIENT
/** Sets the application to client mode.
    Only callable before creation of Application */
void setThisApplicationToClient();
#endif


} // namespace MO

#endif // MOSRC_IO_ISCLIENT_H
