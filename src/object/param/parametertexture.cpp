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
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

//Q_DECLARE_METATYPE(MO::ParameterTexture*);
//namespace { static int register_param = qMetaTypeId<MO::ParameterTexture*>(); }


namespace MO {

ParameterTexture::ParameterTexture(Object * object, const QString& id, const QString& name)
    : Parameter         (object, id, name)
    , lastTex_          (0)
    , wrapModeX_        (WM_CLAMP)
    , wrapModeY_        (WM_CLAMP)
    , magMode_          (MAG_LINEAR)
    , minMode_          (MIN_LINEAR)
{
}

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

void ParameterTexture::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partex", 2);

    // v2
    io << magModeIds[magMode_] << minModeIds[minMode_]
       << wrapModeIds[wrapModeX_] << wrapModeIds[wrapModeY_];
}

void ParameterTexture::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    const int ver = io.readHeader("partex", 2);

    if (ver >= 2)
    {
        io.readEnum(magMode_, MAG_LINEAR, magModeIds, magModeValues);
        io.readEnum(minMode_, MIN_LINEAR, minModeIds, minModeValues);
        io.readEnum(wrapModeX_, WM_CLAMP, wrapModeIds, wrapModeValues);
        io.readEnum(wrapModeY_, WM_CLAMP, wrapModeIds, wrapModeValues);
    }
    else
    {
        wrapModeX_ = wrapModeY_ = WM_CLAMP;
        magMode_ = MAG_LINEAR;
        minMode_ = MIN_LINEAR;
    }
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

    switch (magMode_)
    {
        case MAG_NEAREST:
            tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST)); break;
        case MAG_LINEAR:
            tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR)); break;
    }

    switch (minMode_)
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

    switch (wrapModeX_)
    {
        case WM_CLAMP:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE)); break;
        case WM_REPEAT:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_REPEAT)); break;
        case WM_MIRROR:
            tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_MIRRORED_REPEAT)); break;
    }

    switch (wrapModeY_)
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
    // take the first valid
    for (auto m : modulators())
    if (auto t = static_cast<ModulatorTexture*>(m)->value(time))
    {
        if (t)
        {
//            std::cout << "get tex '" << t->name() << "', hash=" << t->hash() << std::endl;
            if (time.thread() >= lastHash_.size())
                lastHash_.resize(time.thread()+1, -1);
            lastHash_[time.thread()] = t->hash();
        }
        return lastTex_ = t;
    }

    return lastTex_ = 0;
}

bool ParameterTexture::hasChanged(const RenderTime& time) const
{
    if (time.thread() >= lastHash_.size())
        return true;

    // get the connected texture
    const GL::Texture * t = 0;
    for (auto m : modulators())
        if ((t = static_cast<ModulatorTexture*>(m)->value(time)))
            break;

    // different pointer
    if (t != lastTex_)
        return true;
    // same pointer, but changed
    if (t)
    {
//        std::cout << t->name() << ":" << t->hash()
//                  << ", last " << lastHash_[time.thread()] << std::endl;
        return t->hash() != lastHash_[time.thread()];
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



} // namespace MO
