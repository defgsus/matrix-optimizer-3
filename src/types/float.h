/** @file float.h

    @brief float type and utility

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 4/28/2014</p>
*/

#ifndef MOSRC_TYPES_FLOAT_H
#define MOSRC_TYPES_FLOAT_H

#include <qglobal.h> // for Q_STATIC_ASSERT

namespace MO {

    // ------------ basic numeric types -------------

    /** General float type for vectors and whathaveyou.
        Generally 32 bit. */
    typedef float Float;

    /** General double type for precise vectors and whathaveyou.
        Generally 64 bit. */
    typedef double Double;

    /** The garuanteed-to-be-32bit-float-type */
    typedef float F32;

    /** The garuanteed-to-be-64bit-float-type */
    typedef double F64;

    Q_STATIC_ASSERT(sizeof(F32) == 4);
    Q_STATIC_ASSERT(sizeof(F64) == 8);


} // namespace MO

#endif // MOSRC_TYPES_FLOAT_H
