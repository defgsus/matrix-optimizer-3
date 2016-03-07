/** @file parametertexture.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#include "parametertexture.h"
#include "modulatortexture.h"
#include "object/scene.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "io/filemanager.h"
#include "io/imagereader.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

//Q_DECLARE_METATYPE(MO::ParameterTexture*);
//namespace { static int register_param = qMetaTypeId<MO::ParameterTexture*>(); }


namespace MO {

struct ParameterTexture::Private
{
    Private(ParameterTexture*p)
        : p             (p)
        , createdTex    (0)
        , lastTex       (0)
        , constTex      (0)
        , lastScene     (0)
        , inputType     (IT_INPUT)
        , wrapModeX     (WM_CLAMP)
        , wrapModeY     (WM_CLAMP)
        , magMode       (MAG_LINEAR)
        , minMode       (MIN_LINEAR)
        , filename      (":/texture/mo_black.png")
        , needChange    (true)
    { }

    ~Private()
    {
    }

    void releaseCreated();
    const GL::Texture* getNoneTexture();
    const GL::Texture* getFileTexture();
    const GL::Texture* getMasterFrameTexture(bool depth);

    ParameterTexture* p;

    GL::Texture *createdTex;
    const GL::Texture *lastTex, *constTex;
    std::vector<int> lastHash;
    Scene* lastScene;

    InputType inputType;
    WrapMode wrapModeX, wrapModeY;
    MagMode magMode;
    MinMode minMode;

    QString filename;
    bool needChange;
};


ParameterTexture::ParameterTexture(Object * object, const QString& id, const QString& name)
    : Parameter         (object, id, name)
    , p_                (new Private(this))
{
}

ParameterTexture::~ParameterTexture()
{
    delete p_->createdTex;
    delete p_;
}

const QStringList ParameterTexture::inputTypeIds =
{ "none", "input", "file", "master", "master_depth" };
const QStringList ParameterTexture::inputTypeNames =
{ QObject::tr("none"), QObject::tr("input"), QObject::tr("file"),
  QObject::tr("master frame"), QObject::tr("master frame depth") };
const QList<ParameterTexture::InputType> ParameterTexture::inputTypeValues =
{ IT_NONE, IT_INPUT, IT_FILE, IT_MASTER_FRAME, IT_MASTER_FRAME_DEPTH };

const QStringList ParameterTexture::magModeIds =
{ "nearest", "linear" };
const QStringList ParameterTexture::magModeNames =
{ QObject::tr("nearest"), QObject::tr("linear") };
const QList<ParameterTexture::MagMode> ParameterTexture::magModeValues =
{ MAG_NEAREST, MAG_LINEAR };

const QStringList ParameterTexture::minModeIds =
{ "nearest", "linear", "nmn", "lmn", "nml", "lml" };
const QStringList ParameterTexture::minModeNames =
{ QObject::tr("nearest"), QObject::tr("linear"),
  QObject::tr("nearest mipmap nearest"), QObject::tr("linear mipmap nearest"),
  QObject::tr("nearest mipmap linear"), QObject::tr("linear mipmap linear") };
const QList<ParameterTexture::MinMode> ParameterTexture::minModeValues =
{ MIN_NEAREST, MIN_LINEAR,
  MIN_NEAREST_MIPMAP_NEAREST, MIN_LINEAR_MIPMAP_NEAREST,
  MIN_NEAREST_MIPMAP_LINEAR, MIN_LINEAR_MIPMAP_LINEAR };

const QStringList ParameterTexture::wrapModeIds =
{ "clamp", "repeat", "mirror" };
const QStringList ParameterTexture::wrapModeNames =
{ QObject::tr("clamp"), QObject::tr("repeat"), QObject::tr("mirror") };
const QList<ParameterTexture::WrapMode> ParameterTexture::wrapModeValues =
{ WM_CLAMP, WM_REPEAT, WM_MIRROR };

ParameterTexture::InputType ParameterTexture::inputType() const { return p_->inputType; }
ParameterTexture::WrapMode  ParameterTexture::wrapModeX() const { return p_->wrapModeX; }
ParameterTexture::WrapMode  ParameterTexture::wrapModeY() const { return p_->wrapModeY; }
ParameterTexture::MagMode   ParameterTexture::magMode() const { return p_->magMode; }
ParameterTexture::MinMode   ParameterTexture::minMode() const { return p_->minMode; }
                    QString ParameterTexture::filename() const { return p_->filename; }

void ParameterTexture::setInputType(InputType t)
{
    p_->inputType = t;
    p_->needChange = true;
}
void ParameterTexture::setWrapMode(WrapMode m) { setWrapModeX(m); setWrapModeY(m); }
void ParameterTexture::setWrapModeX(WrapMode m) { p_->wrapModeX = m; }
void ParameterTexture::setWrapModeY(WrapMode m) { p_->wrapModeY = m; }
void ParameterTexture::setMagMode(MagMode m) { p_->magMode = m; }
void ParameterTexture::setMinMode(MinMode m) { p_->minMode = m; }
void ParameterTexture::setFilename(const QString& fn) { p_->needChange = true; p_->filename = fn; }

void ParameterTexture::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partex", 3);

    // v2
    io << magModeIds[p_->magMode] << minModeIds[p_->minMode]
       << wrapModeIds[p_->wrapModeX] << wrapModeIds[p_->wrapModeY];
    // v3
    io << inputTypeIds[p_->inputType] << p_->filename;
}

void ParameterTexture::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    const int ver = io.readHeader("partex", 3);

    if (ver >= 2)
    {
        io.readEnum(p_->magMode, MAG_LINEAR, magModeIds, magModeValues);
        io.readEnum(p_->minMode, MIN_LINEAR, minModeIds, minModeValues);
        io.readEnum(p_->wrapModeX, WM_CLAMP, wrapModeIds, wrapModeValues);
        io.readEnum(p_->wrapModeY, WM_CLAMP, wrapModeIds, wrapModeValues);
    }
    else
    {
        p_->wrapModeX = p_->wrapModeY = WM_CLAMP;
        p_->magMode = MAG_LINEAR;
        p_->minMode = MIN_LINEAR;
    }
    if (ver >= 3)
    {
        io.readEnum(p_->inputType, IT_INPUT, inputTypeIds, inputTypeValues);
        io >> p_->filename;
    }
    else
        p_->inputType = IT_INPUT;

    p_->needChange = true;
}



int ParameterTexture::getModulatorTypes() const
{
    return Object::T_SHADER | Object::T_CAMERA | Object::T_TEXTURE;
}

void ParameterTexture::setTextureParam(const GL::Texture* tex) const
{
    if (tex->isMultiSample())
        return;

    using namespace gl;

    switch (p_->magMode)
    {
        case MAG_NEAREST:
            tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST)); break;
        case MAG_LINEAR:
            tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR)); break;
    }

    switch (p_->minMode)
    {
        case MIN_NEAREST:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_NEAREST)); break;
        case MIN_LINEAR:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR)); break;
        case MIN_NEAREST_MIPMAP_NEAREST:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_NEAREST_MIPMAP_NEAREST)); break;
        case MIN_NEAREST_MIPMAP_LINEAR:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_NEAREST_MIPMAP_LINEAR)); break;
        case MIN_LINEAR_MIPMAP_NEAREST:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR_MIPMAP_NEAREST)); break;
        case MIN_LINEAR_MIPMAP_LINEAR:
            tex->setTexParameter(GL_TEXTURE_MIN_FILTER, GLint(GL_LINEAR_MIPMAP_LINEAR)); break;
    }

    switch (p_->wrapModeX)
    {
        case WM_CLAMP:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_MIRRORED_REPEAT)); break;
    }

    switch (p_->wrapModeY)
    {
        case WM_CLAMP:
            tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_MIRRORED_REPEAT)); break;
    }

}

const GL::Texture * ParameterTexture::value(const RenderTime& time) const
{
    const GL::Texture* tex = 0;

    switch (p_->inputType)
    {
        case IT_NONE: tex = p_->getNoneTexture(); break;
        case IT_FILE: tex = p_->getFileTexture(); break;
        case IT_MASTER_FRAME: tex = p_->getMasterFrameTexture(false); break;
        case IT_MASTER_FRAME_DEPTH: tex = p_->getMasterFrameTexture(true); break;
        case IT_INPUT:
            // take the first valid
            for (auto m : modulators())
            if (auto t = static_cast<ModulatorTexture*>(m)->value(time))
            {
                tex = t;
                break;
            }
        break;
    }

    if (tex)
    {
//  std::cout << "get tex '" << t->name() << "', hash=" << t->hash() << std::endl;
        if (time.thread() >= p_->lastHash.size())
            p_->lastHash.resize(time.thread()+1, -1);
        p_->lastHash[time.thread()] = tex->hash();
    }
    return p_->lastTex = tex;
}

bool ParameterTexture::hasChanged(const RenderTime& time) const
{
    if (time.thread() >= p_->lastHash.size())
        return true;

    // get the connected texture
    const GL::Texture * t = 0;
    for (auto m : modulators())
        if ((t = static_cast<ModulatorTexture*>(m)->value(time)))
            break;

    // different pointer
    if (t != p_->lastTex)
        return true;
    // same pointer, but changed
    if (t)
    {
//        std::cout << t->name() << ":" << t->hash()
//                  << ", last " << lastHash_[time.thread()] << std::endl;
        return t->hash() != p_->lastHash[time.thread()];
    }
    return false;
}


Modulator * ParameterTexture::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorTexture(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}

void ParameterTexture::releaseGl()
{
    p_->releaseCreated();
    p_->constTex = 0;
    p_->needChange = true;
}

void ParameterTexture::Private::releaseCreated()
{
    if (createdTex && createdTex->isHandle())
        createdTex->release();
    delete createdTex;
    createdTex = 0;
}

const GL::Texture* ParameterTexture::Private::getNoneTexture()
{
    if (!needChange)
        return createdTex;
    releaseCreated();
    needChange = false;

    QImage img(4,4,QImage::Format_RGBA8888);
    img.fill(Qt::white);
    createdTex = GL::Texture::createFromImage(img, gl::GL_RGBA);
    lastTex = 0;
    return createdTex;
}

const GL::Texture* ParameterTexture::Private::getFileTexture()
{
    if (!needChange)
        return createdTex;
    releaseCreated();
    needChange = false;

    auto fn = IO::fileManager().localFilename(filename);

    ImageReader reader;
    reader.setFilename(fn);
    QImage img = reader.read();

    if (img.isNull())
    {
        // assign generic error texture
        if (!img.load(":/texture/error.png"))
            MO_IO_ERROR(READ, "Failed to even load internal error texture");

        MO_WARNING(QObject::tr("Loading image '%1' failed with '%2'\n")
                                        .arg(fn).arg(reader.errorString()));
        //MO_IO_ERROR(READ, tr("Loading image '%1' failed with '%2'\n")
        //                     .arg(fn).arg(reader.errorString()));
    }

    // upload to GPU
    createdTex = GL::Texture::createFromImage(img, gl::GL_RGBA);
    //if (isMipmap())
    //    texture_->createMipmaps(paramMipmaps_->baseValue(),
    //                            (GLenum)paramMinify_->baseValue());

    return createdTex;
}

const GL::Texture* ParameterTexture::Private::getMasterFrameTexture(bool depth)
{
    if (!needChange && lastScene)
        return constTex;
    constTex = 0;
    needChange = false;

    lastScene = p->object() ? p->object()->sceneObject() : 0;
    if (lastScene)
    {
        constTex = depth
                ? lastScene->fboMaster(MO_GFX_THREAD)->depthTexture()
                : lastScene->fboMaster(MO_GFX_THREAD)->colorTexture(0);
    }

    return constTex;
}


} // namespace MO
