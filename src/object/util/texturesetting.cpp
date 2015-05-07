/** @file texturesetting.cpp

    @brief Texture setting and allocator for Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include <QImage>

#include "texturesetting.h"
#include "object/scene.h"
#include "object/param/parameters.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/param/parametertexture.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "img/image.h"
#include "script/angelscript_image.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {

const QStringList TextureSetting::textureTypeNames =
{ tr("none"), tr("input"), tr("file"), tr("master frame"), tr("master frame depth"),
  tr("camera frame"), tr("camera frame depth") };


TextureSetting::TextureSetting(Object *parent, GL::ErrorReporting rep)
    : QObject       (parent),
      object_       (parent),
      rep_          (rep),
      texture_      (0),
      constTexture_ (0),
      paramType_    (0)
    , paramTex_     (0)
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
    auto params = object_->params();

    paramType_ = params->createSelectParameter(
            "_imgtype" + id_suffix, tr("image type"), tr("Type or source of the image data"),
            { "none", "param", "file", "master", "masterd", "camera", "camerad" },
            textureTypeNames,
            { tr("No texture will be used"),
              tr("The texture input is used"),
              tr("An image will be loaded from a file"),
              tr("The previous master frame is the source of the image"),
              tr("The depth information in the previous master frame is the source of the image"),
              tr("The previous frame of one of the cameras is the source of the image"),
              tr("The depth information in the previous frame of one of the cameras is the source of the image")},
            { TEX_NONE, TEX_PARAM, TEX_FILE,
              TEX_MASTER_FRAME, TEX_MASTER_FRAME_DEPTH,
              TEX_CAMERA_FRAME, TEX_CAMERA_FRAME_DEPTH },
            defaultType, true, false);
    if (!enableNone)
        paramType_->removeByValue(TEX_NONE);

    paramTex_ = params->createTextureParameter("_img_tex" + id_suffix,
                                               tr("texture input"),
                                               tr("Connects to a texture from somewhere else"));

    paramFilename_ = params->createFilenameParameter(
                "_imgfile" + id_suffix, tr("image file"), tr("Filename of the image"),
                normalMap? IO::FT_NORMAL_MAP : IO::FT_TEXTURE,
                normalMap? ":/normalmap/01.png" : ":/texture/mo_black.png");
/*
    paramAngelScript_ = params->createTextParameter(
                "_img_angelscript" + id_suffix, tr("angelscript"), tr("Script"),
                TT_ANGELSCRIPT,
                "\nvoid main()\n{\n\timage.fill(1,0,0);\n}\n");
*/
    paramCamera_ = params->createIntParameter(
                "_imgcamidx" + id_suffix, tr("camera frame"),
                tr("The index of the camera starting at 0"),
                0, true, false);
    paramCamera_->setMinValue(0);

    paramInterpol_ = params->createBooleanParameter(
                "_imginterpol" + id_suffix, tr("interpolation"),
                tr("The interpolation mode for pixel magnification"),
                tr("No interpolation"),
                tr("Linear interpolation"),
                true,
                true, false);

    paramWrapX_ = params->createSelectParameter(
            "_imgwrapx" + id_suffix, tr("on horiz. edges"),
            tr("Selects what happens on the horizontal edges of the texture"),
            { "clamp", "repeat", "repeatm" },
            { tr("clamp"), tr("repeat"), tr("mirror") },
            { tr("Colors stay the same"),
              tr("The texture repeats"),
              tr("The texture repeats mirrored") },
            { WM_CLAMP, WM_REPEAT, WM_MIRROR },
            WM_REPEAT, true, false);

    paramWrapY_ = params->createSelectParameter(
            "_imgwrapy" + id_suffix, tr("on vert. edges"),
            tr("Selects what happens on the vertical edges of the texture"),
            { "clamp", "repeat", "repeatm" },
            { tr("clamp"), tr("repeat"), tr("mirror") },
            { tr("Colors stay the same"),
              tr("The texture repeats"),
              tr("The texture repeats mirrored") },
            { WM_CLAMP, WM_REPEAT, WM_MIRROR },
            WM_REPEAT, true, false);

}

bool TextureSetting::needsReinit(Parameter *p) const
{
    return (p == paramType_
//        ||  p == paramAngelScript_
        || (p == paramFilename_ && paramType_->baseValue() == TEX_FILE)
        || (p == paramCamera_ && (   paramType_->baseValue() == TEX_CAMERA_FRAME
                                  || paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH)));
}

void TextureSetting::updateParameterVisibility()
{
    paramTex_->setVisible( paramType_->baseValue() == TEX_PARAM );
    paramFilename_->setVisible( paramType_->baseValue() == TEX_FILE );
    paramCamera_->setVisible(
                   paramType_->baseValue() == TEX_CAMERA_FRAME
                || paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH );

    //paramAngelScript_->setVisible(paramType_->baseValue() == TEX_ANGELSCRIPT);
}

void TextureSetting::getNeededFiles(IO::FileList &files, IO::FileType ft)
{
    if (paramType_->baseValue() == TEX_FILE)
        files.append(IO::FileListEntry(paramFilename_->value(), ft));
}

// --------------- getter -------------------

bool TextureSetting::isEnabled() const
{
    return paramType_? paramType_->baseValue() != TEX_NONE : false;
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
    if (paramType_->baseValue() == TEX_NONE)
        return true;

    if (paramType_->baseValue() == TEX_PARAM)
    {
        constTexture_ = 0;
        // param wiil be in realtime
        return true;
    }

    if (paramType_->baseValue() == TEX_FILE)
    {
        const QString fn = IO::fileManager().localFilename(paramFilename_->value());

        if (setTextureFromImage_(fn))
            return true;
    }
/*
    if (paramType_->baseValue() == TEX_ANGELSCRIPT)
    {
        if (setTextureFromAS_(paramAngelScript_->baseValue()))
            return true;
    }
*/
    if (paramType_->baseValue() == TEX_MASTER_FRAME
     || paramType_->baseValue() == TEX_MASTER_FRAME_DEPTH)
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


    if (paramType_->baseValue() == TEX_CAMERA_FRAME
     || paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH)
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

    if (paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH )
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
        if (paramType_->baseValue() == TEX_MASTER_FRAME_DEPTH )
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

bool TextureSetting::setTextureFromAS_(const QString& script)
{
#ifdef MO_DISABLE_ANGELSCRIPT
    Q_UNUSED(script);
    return false;

#else

    if (texture_ && texture_->isAllocated())
        texture_->release();
    delete texture_;
    texture_ = 0;
    constTexture_ = 0;

    QImage img;
    ImageEngineAS engine(&img);
    try
    {
        engine.execute(script);

        texture_ = GL::Texture::createFromImage(img, GL_RGBA, rep_);

        if (!texture_)
            return false;

        constTexture_ = texture_;
    }
    catch (Exception& e)
    {
        if (rep_ == GL::ER_THROW)
        {
            e << "\nin TextureFromAS of " << object_->name();
            throw;
        }
        else
        {
            MO_WARNING(e.what() << "\nin TextureFromAS of " << object_->name());
            return false;
        }
    }

    return true;
#endif
}

bool TextureSetting::bind(uint slot)
{
    if (paramType_->baseValue() == TEX_NONE)
        return true;

    auto tex = constTexture_;

    if (paramType_->baseValue() == TEX_PARAM)
    {
                               // XXX
        tex = paramTex_->value(0, MO_GFX_THREAD);
        if (!tex)
            return true;
    }

    if (!tex)
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

    bool r = tex->bind();

    // set interpolation mode
    if (paramInterpol_->baseValue())
        tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    else
        tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // wrapmode
    if (paramWrapX_->baseValue() == WM_CLAMP)
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE)) )
    else if (paramWrapX_->baseValue() == WM_MIRROR)
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_MIRRORED_REPEAT)) )
    else
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_REPEAT)) );

    if (paramWrapY_->baseValue() == WM_CLAMP)
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE)) )
    else if (paramWrapY_->baseValue() == WM_MIRROR)
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_MIRRORED_REPEAT)) )
    else
        MO_CHECK_GL( tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_REPEAT)) );

    // set back
    if ((GLint)slot != act)
        MO_CHECK_GL( glActiveTexture(GLenum(act)) );

    return r;
}

void TextureSetting::unbind(uint slot)
{
    if (paramType_->baseValue() == TEX_NONE)
        return;

    if (constTexture_)
    {
        // set active slot
        slot += uint(GL_TEXTURE0);
        GLint act;
        MO_CHECK_GL( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(GLenum(slot)) );

        // set back
        if ((GLint)slot != act)
            MO_CHECK_GL( glActiveTexture(GLenum(act)) );
    }
}

} // namespace MO
