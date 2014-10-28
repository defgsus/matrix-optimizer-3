/** @file version.h

    @brief Version information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#ifndef MOSRC_IO_VERSION_H
#define MOSRC_IO_VERSION_H

#include <QString>

#define MO_VERSION_MAJOR 3
#define MO_VERSION_MINOR 0
#define MO_VERSION_TINY  1
#define MO_VERSION_MICRO 3

/** Constructs a number from the smaller parts (major = most-significant) */
#define MO_VERSION_COMBINE(maj__, min__, tiny__, micro__) \
                (  (maj__)  * 1000000 \
                 + (min__)  * 10000   \
                 + (tiny__) * 100     \
                 + (micro__)          )

/** The program version as one number (major = most-significant) */
#define MO_VERSION                                                            \
    MO_VERSION_COMBINE(                                                       \
        MO_VERSION_MAJOR, MO_VERSION_MINOR, MO_VERSION_TINY, MO_VERSION_MICRO )


namespace MO {


/** Returns a string in the form of 'x.y.z.www' */
const QString& versionString();

/** Returns "Matrix Optimizer [version] [suffix] */
const QString& applicationName();

} // namespace MO

#endif // MOSRC_IO_VERSION_H
