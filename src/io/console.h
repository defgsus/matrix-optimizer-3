/** @file console.h

    @brief Console out functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 4/28/2014</p>
*/

#ifndef MOSRC_IO_CONSOLE_H
#define MOSRC_IO_CONSOLE_H

#include <iostream>
#include <qglobal.h> // for Q_OS_.. flags

namespace MO {

// --------------- colored console output ----------

#ifdef Q_OS_UNIX

    /** Defines a color for console output.
        @code
        // use like
        out << streamColor(1) << "hello";
        // or better
        out << streamColor::Red << "hello";
        @endcode
    */
    struct streamColor
    {
        static bool enabled;

        static streamColor Default;
        static streamColor Debug;
        static streamColor Warning;
        static streamColor Error;

        int col;

        streamColor(int col) : col(col) { };
    };

    std::ostream& operator<<(std::ostream& os, const streamColor& cs);

#endif // Q_OS_UNIX


#if Q_OS_WIN

    #include <windows.h>

    /** Defines a color for console output.
        @code
        // use like
        out << streamColor(1) << "hello";
        // or better
        out << streamColor::Red << "hello";
        @endcode
    */
    struct streamColor
    {
        static streamColor Default;
        static streamColor Debug;
        static streamColor Warning;
        static streamColor Error;

        static HANDLE hConsole;

        int col;

        streamColor(int col) : col(col) { };
    };

    std::ostream& operator<<(std::ostream& os, const ColorStream& cs);

#endif // Q_OS_WIN



} // namespace MO


#endif // CONSOLE_H
