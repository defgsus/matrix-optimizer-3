/** @file datastream.cpp

    @brief binary (de-)serializer and helper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "datastream.h"
#include "io/error.h"

namespace MO {
namespace IO {

DataStream::DataStream()
{
    setDefaultSettings();
}

DataStream::DataStream(QIODevice * iod)
    : QDataStream(iod)
{
    setDefaultSettings();
}

DataStream::DataStream(QByteArray * a, QIODevice::OpenMode flags)
    : QDataStream(a, flags)
{
    setDefaultSettings();
}

DataStream::DataStream(const QByteArray & a)
    : QDataStream(a)
{
    setDefaultSettings();
}


void DataStream::setDefaultSettings()
{
    setVersion(Qt_4_6);
    setFloatingPointPrecision(DoublePrecision);
}

void DataStream::writeHeader(const QString& id, qint32 version)
{
    *this << id << version;
}

qint32 DataStream::readHeader(const QString& id, qint32 version)
{
    QString head;
    *this >> head;
    if (head != id)
        MO_IO_ERROR(VERSION_MISMATCH, "expected '" << id << "' header, but got '" << head << "'");

    qint32 ver;
    *this >> ver;
    if (ver > version)
        MO_IO_ERROR(VERSION_MISMATCH, "unknown " << id << " version " << ver);

    return ver;
}


} // namespace IO
} // namespace MO
