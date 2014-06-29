/** @file stringmanip.cpp

    @brief string manipulation functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "stringmanip.h"

namespace MO {


void increase_id_number(QString& str, int init)
{
    if (str.isEmpty()) return;

    // find last digit
    int end = str.length()-1;
    while (end && !str.at(end).isDigit()) --end;

    // no number found
    if (!str.at(end).isDigit())
    {
        if (init<0) return;

        // add the init digit
        str.append(QString::number(init));
        return;
    }

    // find start of number
    int start = end;
    while (start>1 && str.at(start-1).isDigit()) --start;

    // extract integer
    int value = str.mid(start).toInt();

    value += 1;

    // replace number
    str.replace(start, end-start+1, QString::number(value));
}

} // namespace MO
