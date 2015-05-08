/** @file parametertexture.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#include "parametertexture.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "modulatortexture.h"

//Q_DECLARE_METATYPE(MO::ParameterTexture*);
//namespace { static int register_param = qMetaTypeId<MO::ParameterTexture*>(); }


namespace MO {

ParameterTexture::ParameterTexture(Object * object, const QString& id, const QString& name)
    :   Parameter       (object, id, name)
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

const GL::Texture * ParameterTexture::value(Double time, uint thread) const
{
    // take the first valid
    for (auto m : modulators())
        if (auto t = static_cast<ModulatorTexture*>(m)->value(time, thread))
            return t;

    return 0;
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
