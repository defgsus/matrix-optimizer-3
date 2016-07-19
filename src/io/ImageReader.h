/** @file imagereader.h

    @brief Wrapper for QImageReader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/09/2015</p>
*/

#ifndef MOSRC_IO_IMAGEREADER_H
#define MOSRC_IO_IMAGEREADER_H

#include <QString>
#include <QImage>

namespace MO {

/** Wrapper for QImageReader
    (because it can't load specific jpegs in Qt5.5) */
class ImageReader
{
public:
    ImageReader();
    ~ImageReader();

    /** Returns the error string, or empty string */
    QString errorString() const;

    /** Sets the filename to load */
    void setFilename(const QString&);

    /** Loads the file given by setFilename().
        On error, the returned QImage is null and errorString() is set. */
    QImage read();

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_IO_IMAGEREADER_H
