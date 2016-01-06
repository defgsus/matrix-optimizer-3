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
    : Parameter       (object, id, name)
    , lastTex_          (0)

{
}


void ParameterTexture::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partex", 1);
}

void ParameterTexture::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("partex", 1);
}



int ParameterTexture::getModulatorTypes() const
{
    return Object::T_SHADER | Object::T_CAMERA | Object::T_TEXTURE;
}

const GL::Texture * ParameterTexture::value(const RenderTime& time) const
{
    // take the first valid
    for (auto m : modulators())
    if (auto t = static_cast<ModulatorTexture*>(m)->value(time))
    {
        if (t)
        {
            if (time.thread() >= lastHash_.size())
                lastHash_.resize(time.thread()+1);
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
        return t->hash() != lastHash_[time.thread()];
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
