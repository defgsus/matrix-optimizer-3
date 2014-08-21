/** @file texturesetting.cpp

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include "texturesetting.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/scene.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "gl/texture.h"
#include "img/image.h"

namespace MO {

const QStringList TextureSetting::textureTypeNames =
{ tr("file"), tr("master frame"), tr("camera frame") };


TextureSetting::TextureSetting(Object *parent, GL::ErrorReporting rep)
    : QObject   (parent),
      object_   (parent),
      rep_      (rep),
      texture_  (0)
{
}


void TextureSetting::serialize(IO::DataStream &io) const
{
    io.writeHeader("texs", 1);
}

void TextureSetting::deserialize(IO::DataStream &io)
{
    io.readHeader("texs", 1);
}

void TextureSetting::createParameters(const QString &id_suffix)
{
    paramType_ = object_->createSelectParameter(
            "imgtype" + id_suffix, tr("image type"), tr("Type or source of the image data"),
            { "file", "master", "camera" },
            textureTypeNames,
            { tr("An image will be loaded from a file"),
              tr("The previous master frame is the source of the image"),
              tr("The previous camera frame is the source of the image") },
            { TT_FILENAME, TT_MASTER_FRAME, TT_CAMERA_FRAME },
            TT_FILENAME, true, false);

    paramFilename_ = object_->createFilenameParameter(
                "imgfile" + id_suffix, tr("image file"), tr("Filename of the image"),
                IO::FT_TEXTURE, ":/texture/mo_black.png");

}


// --------------- getter -------------------

uint TextureSetting::width() const
{
    return texture_? texture_->width() : 0;
}

uint TextureSetting::height() const
{
    return texture_? texture_->height() : 0;
}

// ------------- opengl ---------------------

bool TextureSetting::initGl()
{
    if (paramType_->baseValue() == TT_FILENAME)
    {
        Image img;
        img.loadImage(paramFilename_->value());

        texture_ = GL::Texture::createFromImage(img, GL_RGBA, rep_);
        if (!texture_)
            return false;
    }

    return true;
}

void TextureSetting::releaseGl()
{
    if (paramType_->baseValue() == TT_FILENAME)
    {
        texture_->release();
        delete texture_;
        texture_ = 0;
    }
}

bool TextureSetting::bind()
{
    if (!texture_)
    {
        MO_GL_ERROR_COND(rep_, "no texture defined for TextureSetting::bind()");
        return false;
    }

    return texture_->bind();
}

void TextureSetting::unbind()
{
    if (texture_)
        texture_->unbind();
}

} // namespace MO
