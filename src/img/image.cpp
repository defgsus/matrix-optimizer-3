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

using namespace gl;

namespace MO {

const QStringList Image::formatNames =
{ "MONO-8", "RGB-24", "RGBA-32" };

Image::Image()
    : width_        (0),
      height_       (0),
      format_       (F_RGB_24),
      data_         (0)
{
    MO_DEBUG_IMG("Image::Image()");
}

Image::Image(uint width, uint height, Format format)
    : width_        (width),
      height_       (height),
      format_       (format),
      data_         (0)
{
    MO_DEBUG_IMG("Image::Image(" << width << ", " << height << ", "
                 << formatNames[format] << ")");

    resize_();
}

uint Image::channelSizeInBytes() const
{
    switch (format_)
    {
        case F_MONO_8: return 1;
        case F_RGB_24: return 1;
        case F_RGBA_32: return 1;
    }
    MO_ASSERT(false, "unhandled image format");
    return 0;
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

GLenum Image::glEnumForFormat() const
{
    switch (format_)
    {
        case F_MONO_8: return GL_LUMINANCE;
        case F_RGB_24: return GL_RGB;
        case F_RGBA_32: return GL_RGBA;
    }
    MO_ASSERT(false, "unhandled image format");
    return GL_NONE;
}

GLenum Image::glEnumForType() const
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
    MO_DEBUG_IMG("Image::resize_(" << width_ << ", " << height_ <<
                ", " << formatNames[format_] << ")");

    dataVec_.resize(sizeInBytes() + 16);
    size_t loc = (size_t)&dataVec_[0];
    data_ = (Color*)( (loc / 16 + 1) * 16 );
}

const Image::Color * Image::pixel(uint x, uint y) const
{
    return &data_[(y * width_ + x) * pixelSizeInBytes()];
}

Image::Color * Image::pixel(uint x, uint y)
{
    return &data_[(y * width_ + x) * pixelSizeInBytes()];
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
    MO_ASSERT(false, "unhandled image format");
    return 0;
}




bool Image::loadImage(const QString &filename)
{
    MO_DEBUG_IMG("Image::loadImage('" << filename << "')");

    QImage img(filename);
    if (img.isNull())
        return false;

    return createFrom(img);
}

bool Image::createFrom(const QImage & img)
{
    MO_DEBUG_IMG("Image::createFrom(img) size=" << img.width() << "x" << img.height());

    switch (img.format())
    {
        default: return false;

        case QImage::Format_Mono:
        {
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_Mono -> Image::F_MONO_8");

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
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_MonoLSB -> Image::F_MONO_8");

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
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_RGB32 -> Image::F_RGB_24");
            //MO_ASSERT(img.bytesPerLine() == img.width() * 4, "");
            resize(img.width(), img.height(), F_RGB_24);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const QRgb * src = reinterpret_cast<const QRgb*>(
                        img.constScanLine(img.height() - y - 1));
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = qRed(*src);
                    *dst++ = qGreen(*src);
                    *dst++ = qBlue(*src);
                    ++src;
                }
            }
        }
        break;

        case QImage::Format_Indexed8:
        {
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_Indexed8 -> Image::F_RGBA_32");

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
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_RGB16 -> Image::F_RGB_24");

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
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_ARGB32_Premultiplied -> Image::F_RGBA_32");
        case QImage::Format_ARGB32:
        {
            MO_DEBUG_IMG("Image::createFrom() QImage::Format_ARGB32 -> Image::F_RGBA_32");

            resize(img.width(), img.height(), F_RGBA_32);
            Color * dst = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                const QRgb * src = reinterpret_cast<const QRgb*>(
                        img.constScanLine(img.height() - y - 1));
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = qRed(*src);
                    *dst++ = qGreen(*src);
                    *dst++ = qBlue(*src);
                    *dst++ = qAlpha(*src);
                    ++src;
                }
            }
        }
        break;
    }

    return true;
}

QImage Image::toQImage() const
{
    MO_DEBUG_IMG("Image::toQImage()");

    // determine format for QImage
    switch (format_)
    {
        default: MO_ASSERT(false, "unknown Image format"); return QImage(); break;

        case F_RGB_24:
        {
            MO_DEBUG_IMG("Image::toQImage() F_RGB_24 -> QImage::Format_RGB32");

            QImage img(width_, height_, QImage::Format_RGB32);

            const Color * src = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                QRgb * dst = reinterpret_cast<QRgb*>(
                            img.scanLine(img.height() - y - 1));
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = qRgb(src[0], src[1], src[2]);
                    src += 3;
                }
            }

            return img;
        }

        case F_RGBA_32:
        {
            MO_DEBUG_IMG("Image::toQImage() F_RGBA_32 -> QImage::Format_ARGB32");

            QImage img(width_, height_, QImage::Format_ARGB32);

            const Color * src = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                QRgb * dst = reinterpret_cast<QRgb*>(
                            img.scanLine(img.height() - y - 1));
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = qRgba(src[0], src[1], src[2], src[3]);
                    src += 4;
                }
            }

            return img;
        }

        // convert 8bit to paletted data
        case F_MONO_8:
        {
            MO_DEBUG_IMG("Image::toQImage() F_MONO_8 -> QImage::Format_Indexed8");

            QImage img(width_, height_, QImage::Format_Indexed8);

            img.setColorCount(256);
            for (uint i=0; i<256; ++i)
                img.setColor(i, qRgb(i,i,i));

            const Color * src = &data_[0];
            for (int y=0; y<img.height(); ++y)
            {
                uchar * dst = img.scanLine(img.height() - y - 1);
                for (int x=0; x<img.width(); ++x)
                {
                    *dst++ = *src++;
                }
            }

            return img;
        }
    }
}

} // namespace MO
