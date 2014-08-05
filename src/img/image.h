/** @file image.h

    @brief Image data container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/5/2014</p>
*/

#ifndef MOSRC_IMG_IMAGE_H
#define MOSRC_IMG_IMAGE_H

#include <vector>
#include <ostream>

#include <QStringList>

#include "types/int.h"

class QImage;

namespace MO {

class Image
{
public:

    enum Format
    {
        F_MONO_8,
        F_RGB_24,
        F_RGBA_32
    };

    static const QStringList formatNames;

    // Constructs empty image
    Image();

    /** Constructs an image with uninitialized data */
    Image(uint width, uint height, Format format);

    // -------------- getter -------------------

    bool isEmpty() const { return width_ == 0 || height_ == 0; }

    uint width() const { return width_; }
    uint height() const { return height_; }
    Format format() const { return format_; }

    uint glEnumForFormat() const;
    uint glEnumForType() const;

    uint pixelSizeInBytes() const;
    uint sizeInBytes() const { return width_ * height_ * pixelSizeInBytes(); }

    /** Get pointer to consecutive data (top-left pixel) */
    const unsigned char * data() const { return &data_[0]; }

    // ------------- setter --------------------

    /** Resizes the image (leaves contents as-is) */
    void resize(uint width, uint height, Format format);

    bool loadImage(const QString& filename);

    bool createFrom(const QImage&);

private:

    void resize_();

    uint width_, height_;
    Format format_;

    std::vector<unsigned char> data_;
};

template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& out, const MO::Image& img)
{
    out << "Image(" << img.width() << ", " << img.height() << ", "
        << img.formatNames[img.format()].toStdString() << ")";
    return out;
}

} // namespace MO


#endif // MOSRC_IMG_IMAGE_H
