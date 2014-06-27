/** @file stringmanip.h

    @brief string manipulation functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_TOOL_STRINGMANIP_H
#define MOSRC_TOOL_STRINGMANIP_H

#include <QString>

namespace MO {

/** Increases the last number found in @p str.
    If there is no number in the string and @p init is >= 0,
    the number @p init will be appended.
    Nothing is done for an empty string. */
void increase_id_number(QString& str, int init = -1);


} // namespace MO

#endif // MOSRC_TOOL_STRINGMANIP_H
