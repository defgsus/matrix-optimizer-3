/** @file version.h

    @brief Version information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#ifndef MOSRC_IO_VERSION_H
#define MOSRC_IO_VERSION_H

#include <QString>

/** @page version

    3.0.2.44 audio objects collapsable (change to deserialize)
    3.0.2.42 towards hamburg
    3.0.2.44 shipped
    3.0.2.46 lots new stuff; shipped
    3.0.2.47 lots of fixes
    3.0.3    start windows port (-> vioso)
    3.0.3.2  see changes.txt (-> vioso)
    3.0.3.3  -> vioso

*/

// all limited to 8 bit
#define MO_VERSION_MAJOR 3
#define MO_VERSION_MINOR 0
#define MO_VERSION_TINY  3
#define MO_VERSION_MICRO 8

/** Constructs a number from the smaller parts (major = most-significant).
    Should fit into 32 bit for a while... */
#define MO_VERSION_COMBINE(maj__, min__, tiny__, micro__) \
                 ( ((maj__)  < 24)     \
                 + ((min__)  < 16)     \
                 + ((tiny__) < 8)      \
                 +  (micro__)          )

/** The program version as one number (major == most-significant) */
#define MO_VERSION                                                            \
    MO_VERSION_COMBINE(                                                       \
        MO_VERSION_MAJOR, MO_VERSION_MINOR, MO_VERSION_TINY, MO_VERSION_MICRO )


namespace MO {


/** Returns a string in the form of 'x.y.z.www' */
const QString& versionString();

/** Returns "Matrix Optimizer [version] [suffix] */
const QString& applicationName();

/** Returns a html string of the versions of all included APIs */
QString apiVersionString();

} // namespace MO

#endif // MOSRC_IO_VERSION_H
