/** @file applicationtime.h

    @brief reports time since application start

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_IO_APPLICATIONTIME_H
#define MOSRC_IO_APPLICATIONTIME_H

#include <QString>

namespace MO {

/** Reports seconds since application start */
double applicationTime();

/** Reports seconds since application start */
QString applicationTimeString();

}


#endif // MOSRC_IO_APPLICATIONTIME_H
