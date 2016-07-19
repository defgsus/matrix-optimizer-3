/** @file qtextstreamoperators.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.10.2014</p>
*/

#include "QTextStreamOperators.h"

#include <QObject>
#include <QModelIndex>
#include <QColor>
#include <QRect>


QTextStream& operator << (QTextStream& o, const QString& s)
{
    o << s.toStdString();
    return o;
}

QTextStream& operator << (QTextStream& o, const QRect& r)
{
    o << "QRect(" << r.x() << ", " << r.y() << ", " << r.width() << ", " << r.height() << ")";
    return o;
}

QTextStream& operator << (QTextStream& o, const QObject * obj)
{
    if (obj)
        o << obj->metaObject()->className() << "(" << (void*)obj << ")";
    else
        o << "QObject(0x0)";
    return o;
}

QTextStream& operator << (QTextStream& o, const QModelIndex& idx)
{
    o << " QModelIndex(" << idx.row() << ", " << idx.column() << ", "
        << idx.internalPointer() << ", " << idx.model() << ") ";
    return o;
}

QTextStream& operator << (QTextStream& o, const QColor& c)
{
    o << "QColor(" << c.red() << ", " << c.green() << ", " << c.blue()
      << ", " << c.alpha() << ")";
    return o;
}

QTextStream& operator << (QTextStream& o, const std::string& s)
{
    o << s.c_str();
    return o;
}
