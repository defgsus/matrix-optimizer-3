/** @file stringmanip.h

    @brief string manipulation functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

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


QString fit_in_length(const QString& str, int max_length);

/** Returns a string where every new line starts with @p line_beginning and
    is not longer than @p max_width.
    Line-breaks are inserted before and after tokens (consecutive non-whitspace)
    unless the token would not fit on a line in which case it is broken at @p max_width.
    @note @p line_beginning should not contain the '\t' character or stuff like that,
    since then it's width in actual characters can not be determined and the lines
    may get longer than @p max_width. */
QString fit_text_block(const QString & s, int max_width, const QString& line_beginning = "");

/** Returns humand readable memory count */
QString byte_to_string(long unsigned int);

} // namespace MO

#endif // MOSRC_TOOL_STRINGMANIP_H
