/** @file log.h

    @brief debug output

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSRC_IO_LOG_H
#define MOSRC_IO_LOG_H

#include <QString>
#include "io/console.h"


#define MO_DEBUG(stream_arg__) \
    { std::cerr << streamColor::Debug << stream_arg__ << streamColor::Default << std::endl; }

#define MO_DEBUGF(stream_arg__) \
    { std::cerr << streamColor::Debug << stream_arg__ << streamColor::Default << std::endl; }

/** Enables std::ostream << QString */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, const QString& s)
{
    o << s.toStdString();
    return o;
}


#endif // MOSRC_IO_LOG_H
