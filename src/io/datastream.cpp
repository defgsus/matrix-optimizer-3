/** @file datastream.cpp

    @brief binary (de-)serializer and helper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

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


qint64 DataStream::beginSkip()
{
    // keep position to go back later
    const qint64 pos = device()->pos();
    // write temp skip marker
    (*this) << (qint64)0;
    return pos;

}

void DataStream::endSkip(qint64 startPos)
{
    const qint64 endPos = device()->pos();

    // write length of object
    device()->seek(startPos);
    (*this) << (qint64)(endPos - startPos - sizeof(qint64));
    // come back
    device()->seek(endPos);
}

void DataStream::skip(qint64 length)
{
    device()->seek( device()->pos() + length );
}

qint64 DataStream::reserveFutureValueInt()
{
    const qint64 pos = device()->pos();
    *this << qint64(0);
    return pos;
}

void DataStream::writeFutureValue(qint64 future, qint64 value)
{
    const qint64 pos = device()->pos();
    device()->seek( future );
    *this << value;
    device()->seek( pos );
}

void DataStream::writeHeader(const QString& id, qint32 version)
{
    *this << id << version;

    // once in a while check stream for errors
    if (status() != Ok)
        MO_IO_ERROR(WRITE, "error writing header '"<<id<<"' to stream.\n"
                    "QIODevice error: '"<<device()->errorString()<<"'");
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

    // once in a while check stream for errors
    if (status() != Ok)
        MO_IO_ERROR(READ, "error reading header for '" << id << "' from stream.\n"
                    "QIODevice error: '"<<device()->errorString()<<"'");

    return ver;
}


} // namespace IO
} // namespace MO
