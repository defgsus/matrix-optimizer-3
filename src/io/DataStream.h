/** @file datastream.h

    @brief binary (de-)serializer and helper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#ifndef MOSRC_IO_DATASTREAM_H
#define MOSRC_IO_DATASTREAM_H

#include <QDataStream>
#include <QStringList>

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

    /** Begins a skip section.
        The current file position is saved and a temp marker is written. */
    qint64 beginSkip();

    /** Ends a skip section.
        The length of the skip section is written at the position where
        beginSkip() was called. */
    void endSkip(qint64 begin_pos);

    /** Skips the given number of bytes. */
    void skip(qint64 length);

    qint64 reserveFutureValueInt();
    void writeFutureValue(qint64 future, qint64 value);

    /** Writes the string and the version number to the stream. */
    void writeHeader(const QString& id, qint32 version);

    /** Reads the header data consisting of a string and a version number.
        If the read string does not equal @p expected_id or
        if the read version is larger than @p expected_max_version,
        an Exception::VERSION_MISMATCH is thrown.
        @returns the read version number. */
    qint32 readHeader(const QString& expected_id, qint32 expected_max_version);

    /** Reads a QString and returns the index if it's found in @p enumIds.
        If not, false is returned and @p enumerator is set to @p defaultEnum. */
    template <typename ENUM>
    bool readEnum(ENUM & enumumerator, ENUM defaultEnum, const QStringList& enumIds);

    /** Reads a QString and returns the value from @p enumValues,
        if the string was found in @p enumIds.
        If not, false is returned and @p enumerator is set to @p defaultEnum. */
    template <typename ENUM>
    bool readEnum(ENUM & enumumerator, ENUM defaultEnum,
                  const QStringList& enumIds, const QList<ENUM>& enumValues);

    /* NOTE: We can't have any new members here,
     * QDataStream does not have a virtual destructor. */
};



template <typename ENUM>
bool DataStream::readEnum(ENUM & enumerator, ENUM defaultEnum, const QStringList& enumIds)
{
    QString id;
    *this >> id;

    const int index = enumIds.indexOf(id);
    if (index >= 0)
    {
        enumerator = (ENUM)index;
        return true;
    }
    else
    {
        enumerator = defaultEnum;
        return false;
    }
}


template <typename ENUM>
bool DataStream::readEnum(ENUM & enumerator, ENUM defaultEnum,
                          const QStringList& enumIds, const QList<ENUM>& enumValues)
{
    QString id;
    *this >> id;

    const int index = enumIds.indexOf(id);
    if (index >= 0 && index < enumValues.size())
    {
        enumerator = (ENUM)enumValues.at(index);
        return true;
    }
    else
    {
        enumerator = defaultEnum;
        return false;
    }
}

} // namespace IO
} // namespace MO


#endif // MOSRC_IO_DATASTREAM_H
