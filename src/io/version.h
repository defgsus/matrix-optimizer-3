/** @file version.h

    @brief Version information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#ifndef MOSRC_IO_VERSION_H
#define MOSRC_IO_VERSION_H

#include <QString>

#define MO_VERSION_MAJOR int(3)
#define MO_VERSION_MINOR int(0)
#define MO_VERSION_TINY  int(1)
#define MO_VERSION_MICRO int(2)

#define MO_VERSION                \
    (  MO_VERSION_MAJOR * 1000000 \
     + MO_VERSION_MINOR * 10000   \
     + MO_VERSION_TINY  * 100     \
     + MO_VERSION_MICRO)


namespace MO {


QString versionString();


} // namespace MO

#endif // MOSRC_IO_VERSION_H
