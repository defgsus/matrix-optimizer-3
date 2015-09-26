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
#include "script/angelscript_image.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/imagereader.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {

const QStringList TextureSetting::textureTypeNames =
{ tr("none"), tr("input"), tr("file"), tr("master frame"), tr("master frame depth"),
  tr("camera frame"), tr("camera frame depth") };


TextureSetting::TextureSetting(Object *parent)
    : QObject       (parent),
      object_       (parent),
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
        if (texture_->isHandle())
            MO_GL_WARNING("destruction of TextureSetting with allocated texture "
                          "- OpenGL resource leak");

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
    paramTex_->setVisibleGraph(true);

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

void TextureSetting::initGl()
{
    errorStr_.clear();

    if (paramType_->baseValue() == TEX_NONE)
        return;

    if (paramType_->baseValue() == TEX_PARAM)
    {
        constTexture_ = 0;
        // param will be modulated in realtime
        return;
    }

    if (paramType_->baseValue() == TEX_FILE)
    {
        const QString fn = IO::fileManager().localFilename(paramFilename_->value());

        setTextureFromImage_(fn);
        return;
    }
/*
    if (paramType_->baseValue() == TEX_ANGELSCRIPT)
    {
        if (setTextureFromAS_(paramAngelScript_->baseValue()))
            return;
    }
*/
    if (paramType_->baseValue() == TEX_MASTER_FRAME
     || paramType_->baseValue() == TEX_MASTER_FRAME_DEPTH)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
            MO_GL_ERROR("No Scene object for TextureSetting with type TT_MASTER_FRAME");

        connect(scene, SIGNAL(sceneFboChanged()),
                this, SLOT(updateSceneFbo_()));

        updateSceneFbo_();
        return;
    }


    if (paramType_->baseValue() == TEX_CAMERA_FRAME
     || paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH)
    {
        Scene * scene = object_->sceneObject();
        if (!scene)
            MO_GL_ERROR("No Scene object for TextureSetting with type TT_CAMERA_FRAME");

        connect(scene, SIGNAL(CameraFboChanged(Camera*)),
                this, SLOT(updateCameraFbo_()));

        updateCameraFbo_();
        return;
    }

    if (!constTexture_)
        MO_GL_ERROR("No texture assigned in TextureSetting::initGl()");
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

void TextureSetting::updateCameraFbo_()
{
    MO_DEBUG_GL("TextureSetting::updateCameraFbo_");

    Scene * scene = object_->sceneObject();
    if (!scene)
        MO_GL_ERROR("No Scene object for TextureSetting with type TT_CAMERA_FRAME");

    GL::FrameBufferObject * fbo = scene->fboCamera(MO_GFX_THREAD, paramCamera_->baseValue());

    // special:
    // when camera index out-of-range, don't throw error
    // but load an error image
    if (!fbo)
    {
        MO_GL_WARNING("No camera fbo received from scene");

        setTextureFromImage_(":/texture/error.png");
        return;
    }

    if (paramType_->baseValue() == TEX_CAMERA_FRAME_DEPTH )
    {
        constTexture_ = fbo->depthTexture();
        if (constTexture_ == 0)
            MO_GL_WARNING("No depth texture in TT_CAMERA_FRAME_DEPTH");
    }
    else
        constTexture_ = fbo->colorTexture();

}

void TextureSetting::updateSceneFbo_()
{
    MO_DEBUG_GL("TextureSetting::updateSceneFbo_");

    Scene * scene = object_->sceneObject();
    if (!scene)
        MO_GL_ERROR("No Scene object for TextureSetting with type TT_MASTER_FRAME");

    GL::FrameBufferObject * fbo = scene->fboMaster(MO_GFX_THREAD);
    if (fbo)
    {
        if (paramType_->baseValue() == TEX_MASTER_FRAME_DEPTH )
        {
            constTexture_ = fbo->depthTexture();
            if (constTexture_ == 0)
            {
                MO_GL_WARNING("No depth texture in TT_MASTER_FRAME_DEPTH");
                errorStr_ = tr("No depth texture in TT_MASTER_FRAME_DEPTH");
            }
        }
        else
            constTexture_ = fbo->colorTexture();
    }
}

void TextureSetting::setTextureFromImage_(const QString& fn)
{
    if (texture_ && texture_->isAllocated())
        texture_->release();
    delete texture_;
    texture_ = 0;
    constTexture_ = 0;

    ImageReader reader;
    reader.setFilename(fn);
    QImage img = reader.read();

    if (img.isNull())
    {
        errorStr_ = tr("Loading image '%1' failed with '%2'\n")
                .arg(fn).arg(reader.errorString());
        MO_DEBUG("loading image '" << fn << "' failed with '"
                 << reader.errorString() << "'");
        // assign generic error texture
        if (!img.load(":/texture/error.png"))
            return;
    }

    // upload to GPU
    texture_ = GL::Texture::createFromImage(img, GL_RGBA);
    constTexture_ = texture_;
}

void TextureSetting::setTextureFromAS_(const QString& script)
{
#ifdef MO_DISABLE_ANGELSCRIPT
    Q_UNUSED(script);
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

        texture_ = GL::Texture::createFromImage(img, GL_RGBA);
        constTexture_ = texture_;
    }
    catch (Exception& e)
    {
        errorStr_ = tr("AngelScript image failed with '%1'").arg(e.what());

        e << "\nin TextureFromAS of " << object_->name();
        throw;
    }
#endif
}

void TextureSetting::bind(Double time, uint thread, uint slot)
{
    if (paramType_->baseValue() == TEX_NONE)
        return;

    auto tex = constTexture_;

    if (paramType_->baseValue() == TEX_PARAM)
    {
        tex = paramTex_->value(time, thread);
        if (!tex)
            return;
    }

    if (!tex)
        MO_GL_ERROR("No texture defined for TextureSetting::bind()");

    // set active slot
    slot += (uint)GL_TEXTURE0;
    GLint act;
    MO_CHECK_GL_THROW( glGetIntegerv(GL_ACTIVE_TEXTURE, &act) );
    if ((GLint)slot != act)
        MO_CHECK_GL_THROW( glActiveTexture(GLenum(slot)) );

    tex->bind();

    if (!tex->isMultiSample())
    {
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
    }

    // set back
    if ((GLint)slot != act)
        MO_CHECK_GL_THROW( glActiveTexture(GLenum(act)) );
}


} // namespace MO
