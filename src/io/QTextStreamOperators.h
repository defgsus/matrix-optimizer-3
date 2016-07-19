/** @file qtextstreamoperators.h

    @brief Enables specific qt types to be streamed into QTextStream

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.10.2014</p>
*/

#ifndef MOSRC_IO_QTEXTSTREAMOPERATORS_H
#define MOSRC_IO_QTEXTSTREAMOPERATORS_H

#include <string>
#include <QTextStream>

QTextStream& operator << (QTextStream& o, const QRect& r);
QTextStream& operator << (QTextStream& o, const QObject * obj);
QTextStream& operator << (QTextStream& o, const QModelIndex& idx);
QTextStream& operator << (QTextStream& o, const QColor& c);
QTextStream& operator << (QTextStream& o, const std::string& s);


#endif // MOSRC_IO_QTEXTSTREAMOPERATORS_H
