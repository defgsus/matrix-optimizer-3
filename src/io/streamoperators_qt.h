/** @file streamoperators.h

    @brief Enable specific Qt types for streaming (<</>>) with std::basic_(i/o)stream

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_STREAMOPERATORS_QT_H
#define MOSRC_IO_STREAMOPERATORS_QT_H

#include <QString>
#include <QObject>
#include <QModelIndex>

/** std::ostream << QString */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, const QString& s)
{
    o << s.toStdString();
    return o;
}


/** std::ostream << QObject* */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, const QObject * obj)
{
    if (obj)
        o << obj->metaObject()->className() << "(" << (void*)obj << ")";
    else
        o << "QObject(0x0)";
    return o;
}

#if (0)
/** See if a QObject is behind ptr */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, void * ptr)
{
    //if (QObject * obj = dynamic_cast<QObject*>((QObject*)ptr))
    //    o << obj;
    //else
        o << "0x" << std::hex << (size_t)ptr << std::dec;
    return o;
}
#endif

/** std::ostream << QModelIndex */
template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, const QModelIndex& idx)
{
    o << " QModelIndex(" << idx.row() << ", " << idx.column() << ", "
        << idx.internalPointer() << ", " << idx.model() << ") ";
    return o;
}

#endif // MOSRC_IO_STREAMOPERATORS_QT_H
