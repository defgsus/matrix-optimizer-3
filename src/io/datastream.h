/** @file datastream.h

    @brief binary (de-)serializer and helper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_DATASTREAM_H
#define MOSRC_IO_DATASTREAM_H

#include <QDataStream>

namespace MO {
namespace IO {

/** A QDataStream with default version and some helper functions. */
class DataStream : public QDataStream
{
public:

    DataStream();
    explicit DataStream(QIODevice * iod);
    DataStream(QByteArray * a, QIODevice::OpenMode flags);
    DataStream(const QByteArray & a);

    /** Sets the default version, automatically called by constructor.
        Currently, this sets the version to Qt_4_6 and the
        floatingPointPrecision to double. */
    void setDefaultSettings();

    /** Writes the string and the version number to the stream. */
    void writeHeader(const QString& id, qint32 version);

    /** Reads the header data consisting of a string and a version number.
        If the read string does not equal @p expected_id or
        if the read version is larger than @p expected_max_version,
        an Exception::VERSION_MISMATCH is thrown.
        @returns the read version number. */
    qint32 readHeader(const QString& expected_id, qint32 expected_max_version);

    /* NOTE: We can't have any new members here,
     * QDataStream does not have a virtual destructor. */
};

} // namespace IO
} // namespace MO


#endif // MOSRC_IO_DATASTREAM_H
