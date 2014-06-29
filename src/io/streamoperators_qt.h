/** @file streamoperators.h

    @brief Enable specific Qt types for streaming (<</>>) with std::basic_(i/o)stream

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_STREAMOPERATORS_QT_H
#define MOSRC_IO_STREAMOPERATORS_QT_H

#include <QString>

/** std::ostream << QString */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, const QString& s)
{
    o << s.toStdString();
    return o;
}

#endif // MOSRC_IO_STREAMOPERATORS_QT_H
