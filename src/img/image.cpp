/** @file image.cpp

    @brief Image data container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/5/2014</p>
*/

#include <QImage>

#include "image.h"
#include "gl/opengl.h"
#include "io/log.h"

namespace MO {

const QStringList Image::formatNames =
{ "MONO-8", "RGB-24", "RGBA-32" };

Image::Image()
    : width_        (0),
      height_       (0),
      format_       (F_RGB_24)
{
}

Image::Image(uint width, uint height, Format format)
    : width_        (width),
      height_       (height),
      format_       (format)
{
    resize_();
}


uint Image::pixelSizeInBytes() const
{
    switch (format_)
    {
        case F_MONO_8: return 1;
        case F_RGB_24: return 3;
        case F_RGBA_32: return 4;
    }
    MO_ASSERT(false, "unknown image format");
    return 0;
}

uint Image::glEnumForFormat() const
{
    switch (format_)
    {
        case F_MONO_8: return GL_LUMINANCE;
        case F_RGB_24: return GL_RGB;
        case F_RGBA_32: return GL_RGBA;
    }
    MO_ASSERT(false, "unknown image format");
    return GL_NONE;
}

uint Image::glEnumForType() const
{
    return GL_UNSIGNED_BYTE;
}

void Image::resize(uint width, uint height, Format format)
{
    width_ = width;
    height_ = height;
    format_ = format;
    resize_();
}

void Image::resize_()
{
    MO_DEBUG_GL("Image::resize_(" << width_ << ", " << height_ <<
                ", " << formatNames[format_] << ")");

    data_.resize(sizeInBytes());
}

const Image::Color * Image::pixel(uint x, uint y) const
{
    switch (format_)
    {
        case F_MONO_8:  return &data_[y * width_ + x];
        case F_RGB_24:  return &data_[(y * width_ + x) * 3];
        case F_RGBA_32: return &data_[(y * width_ + x) * 4];
    }
    MO_ASSERT(false, "unknown image format");
    return 0;
}

Image::Color * Image::pixel(uint x, uint y)
{
    switch (format_)
    {
        case F_MONO_8:  return &data_[y * width_ + x];
        case F_RGB_24:  return &data_[(y * width_ + x) * 3];
        case F_RGBA_32: return &data_[(y * width_ + x) * 4];
    }
    MO_ASSERT(false, "unknown image format");
    return 0;
}

Image::Color Image::average(uint x, uint y) const
{
    switch (format_)
    {
        case F_MONO_8: return *pixel(x, y);
        case F_RGBA_32:
        case F_RGB_24:
        {
            auto pix = pixel(x, y);
            return ((int)pix[0] + (int)pix[1] + (int)pix[2])/3;
        }
    }
    return 0;
}




bool Image::loadImage(const QString &filename)
{
    QImage img(filename);
    if (img.isNull())
        return false;

    return createFrom(img);
}

bool Image::createFrom(const QImage & img)
{
    switch (img.format())
    {
        default: return false;

        case QImage::Format_Mono:
        {
            resize(img.width(), img.height(), F_MONO_8);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const Color * src =
                        img.constScanLine(img.height() - y - 1);
                int shift = 7;
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = ((*src >> shift) & 1) * 255;
                    --shift;
                    if (shift < 0)
                    {
                        shift = 7;
                        ++src;
                    }
                }
            }
        }
        break;

        case QImage::Format_MonoLSB:
        {
            resize(img.width(), img.height(), F_MONO_8);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const Color * src =
                        img.constScanLine(img.height() - y - 1);
                int shift = 0;
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = ((*src >> shift) & 1) * 255;
                    ++shift;
                    if (shift == 8)
                    {
                        shift = 0;
                        ++src;
                    }
                }
            }
        }
        break;

        case QImage::Format_RGB32:
        {
            resize(img.width(), img.height(), F_RGB_24);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const Color * src =
                        img.constScanLine(img.height() - y - 1);
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = src[2];
                    *dst++ = src[1];
                    *dst++ = src[0];
                    src += 4;
                }
            }
        }
        break;

        case QImage::Format_Indexed8:
        {
            resize(img.width(), img.height(), F_RGBA_32);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const Color * src = img.constScanLine(img.height() - y - 1);
                for (int x=0; x<img.width(); ++x)
                {
                    const QRgb rgb = img.color(*src);
                    *dst++ = qRed(rgb);
                    *dst++ = qGreen(rgb);
                    *dst++ = qBlue(rgb);
                    *dst++ = qAlpha(rgb);
                    ++src;
                }
            }
        }
        break;

        case QImage::Format_RGB16:
        {
            resize(img.width(), img.height(), F_RGB_24);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const u_int16_t * src = reinterpret_cast<const u_int16_t*>(
                        img.constScanLine(img.height() - y - 1));
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = ((*src >> 11) & ((1<<5)-1)) << 3;
                    *dst++ = ((*src >> 5) & ((1<<6)-1)) << 2;
                    *dst++ = (*src & ((1<<5)-1)) << 3;
                    ++src;
                }
            }
        }
        break;

        case QImage::Format_ARGB32_Premultiplied: // XXX not tested
        case QImage::Format_ARGB32:
        {
            resize(img.width(), img.height(), F_RGBA_32);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const Color * src =
                        img.constScanLine(img.height() - y - 1);
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = src[2];
                    *dst++ = src[1];
                    *dst++ = src[0];
                    *dst++ = src[3];
                    src += 4;
                }
            }
        }
        break;
    }

    return true;
}

} // namespace MO
