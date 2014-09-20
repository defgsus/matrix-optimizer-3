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
#include "gl/opengl.h"

class QImage;

namespace MO {

class Image
{
public:

    typedef unsigned char Color;

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

    /** Returns maximum color */
    static Color max() { return 255; }

    bool isEmpty() const { return width_ == 0 || height_ == 0; }

    uint width() const { return width_; }
    uint height() const { return height_; }
    Format format() const { return format_; }

    /** Returns a opengl enum like GL_LUMINANCE or GL_RGB */
    gl::GLenum glEnumForFormat() const;
    /** Returns the opengl enum for the data type (GL_UNSIGNED_BYTE) */
    gl::GLenum glEnumForType() const;

    uint channelSizeInBytes() const;
    uint pixelSizeInBytes() const;
    uint sizeInBytes() const { return width_ * height_ * pixelSizeInBytes(); }

    // ------------- pixel getter --------------

    /** Get pointer to consecutive data (top-left pixel) */
    const Color * data() const { return &data_[0]; }

    /** Returns read access to the given pixel */
    const Color * pixel(uint x, uint y) const;

    /** Returns write access to the given pixel */
    Color * pixel(uint x, uint y);

    /** Returns the average brightness at given position */
    Color average(uint x, uint y) const;

    // ------------- setter --------------------

    /** Resizes the image (leaves contents as-is) */
    void resize(uint width, uint height, Format format);

    // ------------- io ------------------------

    /** Loads an image (via QImage) */
    bool loadImage(const QString& filename);

    // ---------- QImage conversion ------------

    /** Creates an image from QImage (most formats are handled) */
    bool createFrom(const QImage&);

    /** Creates a QImage from the contents of this image.
        If @p qimage_format == 0, the format will be automatically defined. */
    QImage toQImage() const;

private:

    void resize_();

    uint width_, height_;
    Format format_;

    Color * data_;
    std::vector<Color> dataVec_;
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
