/** @file version.cpp

    @brief Version information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#include "version.h"

namespace MO {


const QString& versionString()
{
    static const QString ret =
            QString("%1.%2.%3.%4")
            .arg(MO_VERSION_MAJOR)
            .arg(MO_VERSION_MINOR)
            .arg(MO_VERSION_TINY)//, 3, 10, QChar('0'))
            .arg(MO_VERSION_MICRO, 3, 10, QChar('0'));

    return ret;
}

const QString& applicationName()
{
    static const QString ret =
            "Matrix Optimizer "
//            "Client "
            + versionString()
#ifdef MO_HAMBURG
            + " (Planetarium Hamburg)"
#endif
            ;

    return ret;
}

} // namespace MO
