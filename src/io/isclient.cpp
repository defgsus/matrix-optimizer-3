/** @file isclient.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/28/2014</p>
*/

#include "isclient.h"
#include "Application.h"
#include "io/error.h"

namespace MO {


namespace { static bool isClient_ = false; }


bool isClient()
{
#ifdef MO_DISABLE_CLIENT
    return false;
#else
    return isClient_;
#endif
}

bool isServer()
{
#ifdef MO_DISABLE_SERVER
    return false;
#else
    return !isClient_;
#endif
}


#ifndef MO_DISABLE_CLIENT
void setThisApplicationToClient()
{
    MO_ASSERT(!application(), "Can't set to client mode after creation of Application");

    isClient_ = true;
}
#endif

} // namespace MO
