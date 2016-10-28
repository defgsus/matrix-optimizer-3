/** @file parametertexture.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#include "ParameterTexture.h"
#include "ParameterFilename.h"
#include "ModulatorTexture.h"
#include "object/Scene.h"
#include "tool/GeneralImage.h"
#include "gl/Texture.h"
#include "gl/FrameBufferObject.h"
#include "io/FileManager.h"
#include "io/ImageReader.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"

//Q_DECLARE_METATYPE(MO::ParameterTexture*);
//namespace { static int register_param = qMetaTypeId<MO::ParameterTexture*>(); }


namespace MO {

struct ParameterTexture::Private
{
    Private(ParameterTexture*p)
        : p             (p)
        , paramFilename (0)
        , createdTex    (0)
        , errorTex      (0)
        , lastTex       (0)
        , constTex      (0)
        , lastScene     (0)
        , isInputSelectable(true)
        , inputType     (IT_INPUT)
        , defaultInputType(IT_INPUT)
        , wrapModeX     (WM_CLAMP)
        , wrapModeY     (WM_CLAMP)
        , wrapModeZ     (WM_CLAMP)
        , magMode       (MAG_LINEAR)
        , minMode       (MIN_LINEAR)
        , mipmaps       (0)
        , needChange    (true)
    { }

    ~Private()
    {
    }

    void releaseCreated(GL::Texture** tex);
    const GL::Texture* value(const RenderTime& time);
    const GL::Texture* getNoneTexture(bool white);
    const GL::Texture* getFileTexture();
    const GL::Texture* getMasterFrameTexture(bool depth);
    const GL::Texture* getErrorTexture();

    ParameterTexture* p;

    ParameterFilename* paramFilename;
    GL::Texture *createdTex, *errorTex;
    const GL::Texture *lastTex, *constTex;
    std::vector<int> lastHash;
    Scene* lastScene;

    bool isInputSelectable;
    InputType inputType, defaultInputType;
    WrapMode wrapModeX, wrapModeY, wrapModeZ;
    MagMode magMode;
    MinMode minMode;
    uint mipmaps;

    bool needChange;
};


ParameterTexture::ParameterTexture(Object * object, const QString& id, const QString& name)
    : Parameter         (object, id, name)
    , p_                (new Private(this))
{
}

ParameterTexture::~ParameterTexture()
{
    delete p_->errorTex;
    delete p_->createdTex;
    delete p_;
}

const QStringList ParameterTexture::inputTypeIds =
{ "none", "black", "white", "input", "file", "master", "master_depth" };
const QStringList ParameterTexture::inputTypeNames =
{ QObject::tr("off"), QObject::tr("black"), QObject::tr("white"),
  QObject::tr("input"), QObject::tr("file"),
  QObject::tr("master frame"), QObject::tr("master frame depth") };
const QList<ParameterTexture::InputType> ParameterTexture::inputTypeValues =
{ IT_NONE, IT_BLACK, IT_WHITE, IT_INPUT, IT_FILE,
  IT_MASTER_FRAME, IT_MASTER_FRAME_DEPTH };

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
const QStringList ParameterTexture::minModeNamesShort =
{ QObject::tr("nearest"), QObject::tr("linear"),
  QObject::tr("near/mm/near"), QObject::tr("lin/mm/near"),
  QObject::tr("near/mm/linr"), QObject::tr("lin/mm/lin") };
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

bool ParameterTexture::isInputTypeSelectable() const { return p_->isInputSelectable; }
ParameterTexture::InputType ParameterTexture::inputType() const { return p_->inputType; }
ParameterTexture::InputType ParameterTexture::defaultInputType() const { return p_->defaultInputType; }
ParameterTexture::WrapMode  ParameterTexture::wrapModeX() const { return p_->wrapModeX; }
ParameterTexture::WrapMode  ParameterTexture::wrapModeY() const { return p_->wrapModeY; }
ParameterTexture::WrapMode  ParameterTexture::wrapModeZ() const { return p_->wrapModeZ; }
ParameterTexture::MagMode   ParameterTexture::magMode() const { return p_->magMode; }
ParameterTexture::MinMode   ParameterTexture::minMode() const { return p_->minMode; }
                       uint ParameterTexture::mipmaps() const { return p_->mipmaps; }
                    QString ParameterTexture::filename() const { return p_->paramFilename ? p_->paramFilename->value() : ""; }
ParameterFilename* ParameterTexture::filenameParameter() const { return p_->paramFilename; }

bool ParameterTexture::isMipmap() const
{
    return  minMode() != MIN_NEAREST
         && minMode() != MIN_LINEAR
         //&& paramMipmaps_->baseValue() > 0
            ;

}

void ParameterTexture::setInputTypeSelectable(bool e)
    { p_->isInputSelectable = e; }
void ParameterTexture::setInputType(InputType t)
    { p_->inputType = t; p_->needChange = true; }
void ParameterTexture::setDefaultInputType(InputType t)
    { p_->defaultInputType = t; }
void ParameterTexture::setWrapMode(WrapMode m)
    { setWrapModeX(m); setWrapModeY(m); setWrapModeZ(m); }
void ParameterTexture::setWrapModeX(WrapMode m) { p_->wrapModeX = m; }
void ParameterTexture::setWrapModeY(WrapMode m) { p_->wrapModeY = m; }
void ParameterTexture::setWrapModeZ(WrapMode m) { p_->wrapModeZ = m; }
void ParameterTexture::setMagMode(MagMode m) { p_->magMode = m; }
void ParameterTexture::setMinMode(MinMode m) { p_->minMode = m; }
void ParameterTexture::setMipmaps(uint level)
    { p_->needChange = p_->mipmaps != level && p_->inputType == IT_FILE;
      p_->mipmaps = level; }
void ParameterTexture::setFilenameParameter(ParameterFilename* p)
    { p_->paramFilename = p; p_->needChange = true; }

void ParameterTexture::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partex", 7);

    // v2
    io << magModeIds[p_->magMode] << minModeIds[p_->minMode]
       << wrapModeIds[p_->wrapModeX] << wrapModeIds[p_->wrapModeY];
    // v7
    io << wrapModeIds[p_->wrapModeZ];
    // v6
    if (p_->inputType == IT_INTERNAL)
        io << (qint8)0;
    else
    {
        // v6
        io << (qint8)1;
        // v3
        io << inputTypeIds[p_->inputType];
    }
    // v4 removed filename QString
    // v5
    io << (qint64)p_->mipmaps;
}

void ParameterTexture::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    const int ver = io.readHeader("partex", 7);

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
    if (ver >= 7)
        io.readEnum(p_->wrapModeZ, WM_CLAMP, wrapModeIds, wrapModeValues);
    else
        p_->wrapModeZ = WM_CLAMP;

    if (ver >= 3)
    {
        qint8 hasType = 1;
        if (ver >= 6)
            io >> hasType;
        if (hasType)
            io.readEnum(p_->inputType, IT_INPUT, inputTypeIds, inputTypeValues);
        else
            p_->inputType = IT_INTERNAL;
        if (ver == 3) { QString dummy; io >> dummy; }
    }
    else
        p_->inputType = p_->defaultInputType;

    if (ver >= 5)
    {
        qint64 mm;
        io >> mm; p_->mipmaps = mm;
    }

    p_->needChange = true;
}

void ParameterTexture::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterTexture*>(other);
    if (!p)
        return;

    if (p_->paramFilename && p->p_->paramFilename)
        p_->paramFilename->copyFrom(p->p_->paramFilename);

    p_->inputType = p->p_->inputType;
    p_->defaultInputType = p->p_->defaultInputType;
    p_->wrapModeX = p->p_->wrapModeX;
    p_->wrapModeY = p->p_->wrapModeY;
    p_->wrapModeZ = p->p_->wrapModeZ;
    p_->magMode = p->p_->magMode;
    p_->minMode = p->p_->minMode;
    p_->mipmaps = p->p_->mipmaps;
    p_->needChange = true;
}


int ParameterTexture::getModulatorTypes() const
{
    return Object::T_SHADER | Object::T_CAMERA | Object::T_TEXTURE;
}

void ParameterTexture::applyTextureParam(const GL::Texture* tex) const
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
            tex->setTexParameter(GL_TEXTURE_WRAP_S,
                                 GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_S,
                                 GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_S,
                                 GLint(GL_MIRRORED_REPEAT)); break;
    }

    switch (p_->wrapModeY)
    {
        case WM_CLAMP:
            tex->setTexParameter(GL_TEXTURE_WRAP_T,
                                 GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_T,
                                 GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_T,
                                 GLint(GL_MIRRORED_REPEAT)); break;
    }

    switch (p_->wrapModeZ)
    {
        case WM_CLAMP:
            tex->setTexParameter(GL_TEXTURE_WRAP_R,
                                 GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_R,
                                 GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_R,
                                 GLint(GL_MIRRORED_REPEAT)); break;
    }

}

const GL::Texture* ParameterTexture::Private::value(const RenderTime& time)
{
    const GL::Texture* tex = 0;

    switch (inputType)
    {
        case IT_INTERNAL:
        case IT_NONE: return nullptr;
        case IT_BLACK:
        case IT_WHITE: tex = getNoneTexture(inputType == IT_WHITE); break;
        case IT_FILE: tex = getFileTexture(); break;
        case IT_MASTER_FRAME: tex = getMasterFrameTexture(false); break;
        case IT_MASTER_FRAME_DEPTH: tex = getMasterFrameTexture(true); break;
        case IT_INPUT:
            // take the first valid
            for (auto m : p->modulators())
            if (auto t = static_cast<ModulatorTexture*>(m)->value(time))
            {
                tex = t;
                break;
            }
        break;
    }

    if (!tex)
        tex = getErrorTexture();

    return tex;
}

const GL::Texture * ParameterTexture::value(const RenderTime& time) const
{
    const GL::Texture* tex = p_->value(time);

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
    const GL::Texture * t = p_->value(time);

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


Modulator * ParameterTexture::getModulator(
        const QString& id, const QString& outputId)
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
    p_->releaseCreated(&p_->createdTex);
    p_->releaseCreated(&p_->errorTex);
    p_->constTex = 0;
    p_->needChange = true;
}

void ParameterTexture::Private::releaseCreated(GL::Texture** tex)
{
    if (*tex && (*tex)->isHandle())
        (*tex)->release();
    delete *tex;
    *tex = 0;
}

const GL::Texture* ParameterTexture::Private::getNoneTexture(bool white)
{
    if (!needChange)
        return createdTex;
    releaseCreated(&createdTex);
    needChange = false;

    QImage img(4,4,QImage::Format_RGBA8888);
    img.fill(white ? Qt::white : Qt::black);
    createdTex = GL::Texture::createFromImage(img, gl::GL_RGBA);
    lastTex = 0;
    return createdTex;
}

const GL::Texture* ParameterTexture::Private::getFileTexture()
{
    MO_ASSERT(paramFilename,
              "ParameterFilename not assigned to ParameterTexture");

    if (!needChange)
        return createdTex;
    releaseCreated(&createdTex);
    needChange = false;

    auto fn = IO::fileManager().localFilename(paramFilename->value());

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
    if (mipmaps > 0)
        createdTex->createMipmaps(mipmaps);

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

const GL::Texture* ParameterTexture::Private::getErrorTexture()
{
    if (!errorTex)
    {
        QImage img = GeneralImage::getErrorImage(
                    QObject::tr("NULL"),
                    QSize(128,128), QImage::Format_RGBA8888_Premultiplied);
        errorTex = GL::Texture::createFromImage(img, gl::GL_RGBA);
    }
    return errorTex;
}


} // namespace MO
