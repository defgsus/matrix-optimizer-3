/** @file currenttime.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2014</p>
*/

#include "currenttime.h"
#include "applicationtime.h"

namespace MO {


Double currentTime()
{
    return applicationTime();
}


}
