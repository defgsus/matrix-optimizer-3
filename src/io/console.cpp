/** @file console.cpp

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 4/28/2014</p>
*/

#include "console.h"

namespace MO {

bool streamColor::enabled = false;

#ifdef Q_OS_UNIX

    streamColor streamColor::Default = streamColor(9);
    streamColor streamColor::Debug = streamColor(4);
    streamColor streamColor::Warning = streamColor(1);
    streamColor streamColor::Error = streamColor(1);

    std::ostream& operator<<(std::ostream& os, const streamColor& cs)
    {
        if (streamColor::enabled)
            os << "\33[3" << cs.col << ";49m";
        return os;
    }

#endif // Q_OS_UNIX


#ifdef Q_OS_WIN

    HANDLE ColorStream::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    streamColor streamColor::Default = streamColor(7);
    streamColor streamColor::Debug = streamColor(6);
    streamColor streamColor::Warning = streamColor(13);
    streamColor streamColor::Error = streamColor(12);

    std::ostream& operator<<(std::ostream& os, const streamColor& cs)
    {
        if (streamColor::enabled)
            SetConsoleTextAttribute(streamColor::hConsole, cs.col);
        return os;
    }

#endif // Q_OS_WIN


} // namespace MO
