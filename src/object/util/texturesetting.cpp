/** @file texturesetting.cpp

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include "texturesetting.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/scene.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "img/image.h"

namespace MO {

const QStringList TextureSetting::textureTypeNames =
{ tr("none"), tr("file"), tr("master frame"), tr("camera frame") };


TextureSetting::TextureSetting(Object *parent, GL::ErrorReporting rep)
    : QObject       (parent),
      object_       (parent),
      rep_          (rep),
      texture_      (0),
      constTexture_ (0),
      paramType_    (0)
{
}

TextureSetting::~TextureSetting()
{
    if (texture_)
    {
        if (texture_->isCreated())
            MO_GL_WARNING("destruction of TextureSetting with allocated texture");

        delete texture_;
    }
}


void TextureSetting::serialize(IO::DataStream &io) const
{
    io.writeHeader("texs", 1);
}

void TextureSetting::deserialize(IO::DataStream &io)
{
    io.readHeader("texs", 1);
}

void TextureSetting::createParameters(
        const QString &id_suffix, TextureType defaultType, bool enableNone)
{
    paramType_ = object_->createSelectParameter(
            "_imgtype" + id_suffix, tr("image type"), tr("Type or source of the image data"),
            { "none", "file", "master", "camera" },
            textureTypeNames,
            { tr("No texture will be used - this is not supported by all objects"),
              tr("An image will be loaded from a file"),
              tr("The previous master frame is the source of the image"),
              tr("The previous camera frame is the source of the image") },
            { TT_NONE, TT_FILE, TT_MASTER_FRAME, TT_CAMERA_FRAME },
            defaultType, true, false);
    if (!enableNone)
        paramType_->removeByValue(TT_NONE);


    paramFilename_ = object_->createFilenameParameter(
                "_imgfile" + id_suffix, tr("image file"), tr("Filename of the image"),
                IO::FT_TEXTURE, ":/texture/mo_black.png");

    paramCamera_ = object_->createIntParameter(
                "_imgcamidx" + id_suffix, tr("camera frame"),
                tr("The index of the camera starting at 0"),
                0, true, false);
    paramCamera_->setMinValue(0);
}

bool TextureSetting::needsReinit(Parameter *p) const
{
    return (p == paramType_
        || (p == paramFilename_ && paramType_->baseValue() == TT_FILE)
        || (p == paramCamera_ && paramType_->baseValue() == TT_CAMERA_FRAME));
}

// --------------- getter -------------------

bool TextureSetting::isEnabled() const
{
    return paramType_? paramType_->baseValue() != TT_NONE : false;
}

bool TextureSetting::isCube() const
{
    return constTexture_ && constTexture_->isCube();
}

uint TextureSetting::width() const
{
    return constTexture_? constTexture_->width() : 0;
}

uint TextureSetting::height() const
{
    return constTexture_? constTexture_->height() : 0;
}

// ------------- opengl ---------------------

bool TextureSetting::initGl()
{
    if (paramType_->baseValue() == TT_NONE)
        return true;

    if (paramType_->baseValue() == TT_FILE)
    {
        Image img;
        img.loadImage(paramFilename_->value());

        texture_ = GL::Texture::createFromImage(img, GL_RGBA, rep_);
        if (!texture_)
            return false;
        constTexture_ = texture_;
    }

    if (paramType_->baseValue() == TT_MASTER_FRAME)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
        {
            MO_GL_ERROR_COND(rep_, "no Scene object for TextureSetting with type TT_MASTER_FRAME");
            return 0;
        }

        GL::FrameBufferObject * fbo = scene->fboMaster(MO_GFX_THREAD);
        if (fbo)
            constTexture_ = fbo->colorTexture();
    }


    if (paramType_->baseValue() == TT_CAMERA_FRAME)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
        {
            MO_GL_ERROR_COND(rep_, "no Scene object for TextureSetting with type TT_CAMERA_FRAME");
            return 0;
        }


        GL::FrameBufferObject * fbo = scene->fboCamera(MO_GFX_THREAD, paramCamera_->baseValue());

        // special:
        // when camera index out-of-range, don't throw error
        // but load an error image
        if (!fbo)
        {
            Image img;
            img.loadImage(":/texture/error.png");
            texture_ = GL::Texture::createFromImage(img, GL_RGB, rep_);
            if (!texture_)
                return false;
            constTexture_ = texture_;
            return true;
        }

        constTexture_ = fbo->colorTexture();

    }

    if (!constTexture_)
        MO_GL_ERROR_COND(rep_, "no texture assigned in TextureSetting::initGl()");

    return constTexture_ != 0;
}

void TextureSetting::releaseGl()
{
    if (texture_)
    {
        texture_->release();
        delete texture_;
        texture_ = 0;
    }

    constTexture_ = 0;
}

bool TextureSetting::bind(uint slot)
{
    if (paramType_->baseValue() == TT_NONE)
        return true;

    if (!constTexture_)
    {
        MO_GL_ERROR_COND(rep_, "no texture defined for TextureSetting::bind()");
        return false;
    }

    // set active slot
    slot += GL_TEXTURE0;
    GLint act;
    MO_CHECK_GL( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
    if ((GLint)slot != act)
        MO_CHECK_GL( glActiveTexture(slot) );

    bool r = constTexture_->bind();

    // set back
    if ((GLint)slot != act)
        MO_CHECK_GL( glActiveTexture(act) );

    return r;
}

void TextureSetting::unbind(uint slot)
{
    if (paramType_->baseValue() == TT_NONE)
        return;

    if (constTexture_)
    {
        // set active slot
        slot += GL_TEXTURE0;
        GLint act;
        MO_CHECK_GL( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(slot) );

        constTexture_->unbind();

        // set back
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(act) );
    }
}

} // namespace MO
