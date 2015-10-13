/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#ifndef MOSRC_TOOL_LINEARIZERFLOAT_H
#define MOSRC_TOOL_LINEARIZERFLOAT_H

#include "types/float.h"

namespace MO {

/** A threadsafe timed float input with output interpolation */
class LinearizerFloat
{
public:
    LinearizerFloat();
    ~LinearizerFloat();

    /** Read the value from stack */
    Float getValue(Double time) const;

    // ---- setter ----

    void clear();

    /** Put a value into the stack.
        Threadsafe! */
    void insertValue(Double time, Float value);

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_TOOL_LINEARIZERFLOAT_H
