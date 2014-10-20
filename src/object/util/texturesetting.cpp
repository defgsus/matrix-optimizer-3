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
#include "io/filemanager.h"

using namespace gl;

namespace MO {

const QStringList TextureSetting::textureTypeNames =
{ tr("none"), tr("file"), tr("master frame"), tr("master frame depth"),
  tr("camera frame"), tr("camera frame depth") };


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

void TextureSetting::createParameters(const QString &id_suffix, TextureType defaultType,
                bool enableNone, bool normalMap)
{
    paramType_ = object_->createSelectParameter(
            "_imgtype" + id_suffix, tr("image type"), tr("Type or source of the image data"),
            { "none", "file", "master", "masterd", "camera", "camerad" },
            textureTypeNames,
            { tr("No texture will be used"),
              tr("An image will be loaded from a file"),
              tr("The previous master frame is the source of the image"),
              tr("The depth information in the previous master frame is the source of the image"),
              tr("The previous frame of one of the cameras is the source of the image"),
              tr("The depth information in the previous frame of one of the cameras is the source of the image")},
            { TT_NONE, TT_FILE,
              TT_MASTER_FRAME, TT_MASTER_FRAME_DEPTH,
              TT_CAMERA_FRAME, TT_CAMERA_FRAME_DEPTH },
            defaultType, true, false);
    if (!enableNone)
        paramType_->removeByValue(TT_NONE);


    paramFilename_ = object_->createFilenameParameter(
                "_imgfile" + id_suffix, tr("image file"), tr("Filename of the image"),
                normalMap? IO::FT_NORMAL_MAP : IO::FT_TEXTURE,
                normalMap? ":/normalmap/01.png" : ":/texture/mo_black.png");

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
        || (p == paramCamera_ && (   paramType_->baseValue() == TT_CAMERA_FRAME
                                  || paramType_->baseValue() == TT_CAMERA_FRAME_DEPTH)));
}

void TextureSetting::updateParameterVisibility()
{
    paramFilename_->setVisible( paramType_->baseValue() == TT_FILE );
    paramCamera_->setVisible(
                   paramType_->baseValue() == TT_CAMERA_FRAME
                || paramType_->baseValue() == TT_CAMERA_FRAME_DEPTH );
}

void TextureSetting::getNeededFiles(IO::FileList &files, IO::FileType ft)
{
    if (paramType_->baseValue() == TT_FILE)
        files.append(IO::FileListEntry(paramFilename_->value(), ft));
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
        const QString fn = IO::fileManager().localFilename(paramFilename_->value());

        if (setTextureFromImage_(fn))
            return true;
    }

    if (paramType_->baseValue() == TT_MASTER_FRAME
     || paramType_->baseValue() == TT_MASTER_FRAME_DEPTH)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
        {
            MO_GL_ERROR_COND(rep_, "no Scene object for TextureSetting with type TT_MASTER_FRAME");
            return 0;
        }

        connect(scene, SIGNAL(sceneFboChanged()),
                this, SLOT(updateSceneFbo_()));

        if (updateSceneFbo_())
            return true;
    }


    if (paramType_->baseValue() == TT_CAMERA_FRAME
     || paramType_->baseValue() == TT_CAMERA_FRAME_DEPTH)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
        {
            MO_GL_ERROR_COND(rep_, "no Scene object for TextureSetting with type TT_CAMERA_FRAME");
            return 0;
        }

        connect(scene, SIGNAL(CameraFboChanged(Camera*)),
                this, SLOT(updateCameraFbo_()));

        if (updateCameraFbo_())
            return true;
    }

    if (!constTexture_)
        MO_GL_ERROR_COND(rep_, "no texture assigned in TextureSetting::initGl()");

    return constTexture_ != 0;
}

void TextureSetting::releaseGl()
{
    Scene * scene = object_->sceneObject();
    if (scene)
    {
        connect(scene, SIGNAL(sceneFboChanged()),
                this, SLOT(updateSceneFbo_()));
        disconnect(scene, SIGNAL(CameraFboChanged(Camera*)),
                   this, SLOT(updateCameraFbo_()));
    }

    if (texture_)
    {
        texture_->release();
        delete texture_;
        texture_ = 0;
    }

    constTexture_ = 0;
}

bool TextureSetting::updateCameraFbo_()
{
    MO_DEBUG_GL("TextureSetting::updateCameraFbo_");

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
        MO_WARNING("no camera fbo received from scene");

        return setTextureFromImage_(":/texture/error.png");
    }

    if (paramType_->baseValue() == TT_CAMERA_FRAME_DEPTH )
    {
        constTexture_ = fbo->depthTexture();
        if (constTexture_ == 0)
            MO_GL_WARNING("no depth texture in TT_CAMERA_FRAME_DEPTH");
    }
    else
        constTexture_ = fbo->colorTexture();


    return constTexture_ != 0;
}

bool TextureSetting::updateSceneFbo_()
{
    MO_DEBUG_GL("TextureSetting::updateSceneFbo_");

    Scene * scene = object_->sceneObject();
    if (!scene)
    {
        MO_GL_ERROR_COND(rep_, "no Scene object for TextureSetting with type TT_MASTER_FRAME");
        return 0;
    }

    GL::FrameBufferObject * fbo = scene->fboMaster(MO_GFX_THREAD);
    if (fbo)
    {
        if (paramType_->baseValue() == TT_MASTER_FRAME_DEPTH )
        {
            constTexture_ = fbo->depthTexture();
            if (constTexture_ == 0)
                MO_GL_WARNING("no depth texture in TT_MASTER_FRAME_DEPTH");
        }
        else
            constTexture_ = fbo->colorTexture();
    }

    return constTexture_ != 0;
}

bool TextureSetting::setTextureFromImage_(const QString& fn)
{
    if (texture_ && texture_->isAllocated())
        texture_->release();
    delete texture_;
    texture_ = 0;
    constTexture_ = 0;

    Image img;
    if (!img.loadImage(fn) &&
        !img.loadImage(":/texture/error.png"))
        return false;

    texture_ = GL::Texture::createFromImage(img, GL_RGBA, rep_);

    if (!texture_)
        return false;

    constTexture_ = texture_;
    return true;

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
    slot += (uint)GL_TEXTURE0;
    GLint act;
    MO_CHECK_GL( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
    if ((GLint)slot != act)
        MO_CHECK_GL( glActiveTexture(GLenum(slot)) );

    bool r = constTexture_->bind();

    // set back
    if ((GLint)slot != act)
        MO_CHECK_GL( glActiveTexture(GLenum(act)) );

    return r;
}

void TextureSetting::unbind(uint slot)
{
    if (paramType_->baseValue() == TT_NONE)
        return;

    if (constTexture_)
    {
        // set active slot
        slot += uint(GL_TEXTURE0);
        GLint act;
        MO_CHECK_GL( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(GLenum(slot)) );

        constTexture_->unbind();

        // set back
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(GLenum(act)) );
    }
}

} // namespace MO
