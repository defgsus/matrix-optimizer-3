/** @file network_fwd.h

    @brief forward declarations of all network classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#ifndef MOSRC_NETWORK_NETWORK_FWD_H
#define MOSRC_NETWORK_NETWORK_FWD_H

namespace MO
{
    class AbstractNetEvent;
    class NetEventRequest;
    class NetworkManager;
#ifndef MO_CLIENT
    class TcpServer;
    class ServerEngine;
    class ClientInfo;
#else
    class Client;
    class ClientEngine;
#endif

} // namespace MO


#endif // MOSRC_NETWORK_NETWORK_FWD_H
