/** @file imagereader.cpp

    @brief Wrapper for QImageReader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/09/2015</p>
*/

#include <cstddef> // for size_t  }
#include <cstdio> // for FILE     } dependencies of jpeglib.h
#include <jpeglib.h>

#include <QImageReader>

#include "imagereader.h"

namespace MO {

struct ImageReader::Private
{
    Private()
    {

    }

    QImage readLibJpeg();

    QString filename, errorStr;
};

ImageReader::ImageReader()
    : p_        (new Private())
{

}

ImageReader::~ImageReader()
{
    delete p_;
}

void ImageReader::setFilename(const QString & fn)
{
    p_->filename = fn;
}

QString ImageReader::errorString() const
{
    return p_->errorStr;
}

QImage ImageReader::read()
{
    p_->errorStr.clear();

    QImageReader reader;
    reader.setFileName(p_->filename);
    reader.setDecideFormatFromContent(true);

    // read with Qt
    QImage img = reader.read();

    if (img.isNull())
    {
        // filter for strange errors
        const bool strange =
                  reader.error() == QImageReader::InvalidDataError
               || reader.error() == QImageReader::UnknownError;

        if (!strange)
        {
            p_->errorStr = reader.errorString();
            return img;
        }

        // try libjpeg
        img = p_->readLibJpeg();
    }

    return img;
}


QImage ImageReader::Private::readLibJpeg()
{
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;

    FILE * infile;
    JSAMPROW row_p[1];	/* pointer to one row of JSAMPLEs */
    int row_stride;		/* physical row width in output buffer */

    infile = fopen(filename.toStdString().c_str(), "rb");
    if (!infile)
    {
        errorStr = QObject::tr("Can not open file");
        return QImage();
    }

    // setup the decompressing object

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);

    // start reading

    jpeg_read_header(&cinfo, TRUE);

    // override defaults

    cinfo.out_color_space = JCS_RGB;
    cinfo.output_components = 3;

    jpeg_start_decompress(&cinfo);

    // init image
    QImage img(cinfo.image_width, cinfo.image_height,
               QImage::Format_RGB32);
    img.fill(Qt::black); // clear the unused alpha channel

    // get the pixel-data

    row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPLE * buffer = (JSAMPLE*) malloc(row_stride);
    row_p[0] = buffer;

    while ((int)cinfo.output_scanline < img.height())
    {
        // read scanline into 'buffer'
        jpeg_read_scanlines(&cinfo, row_p, 1);

        // copy pixels per scanline
        const int Y = cinfo.output_scanline - 1;
        // (QImage stores scanlines upside down, same as jpeg)
        QRgb * dst = (QRgb*)img.scanLine(Y);
        const unsigned char *src = buffer;
        for (int X = 0; X < img.width(); ++X, ++dst, src += 3)
        {
            *dst = qRgb(src[0], src[1], src[2]);
        }
    }

    // good bye
    free(buffer);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return img;
}



} // namespace MO
