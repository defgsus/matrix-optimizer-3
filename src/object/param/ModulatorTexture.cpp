/** @file modulatortexture.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#include "ModulatorTexture.h"
#include "object/control/ModulatorObjectFloat.h"
#include "object/interface/ValueTextureInterface.h"
#include "io/DataStream.h"
#include "io/error.h"

namespace MO {


ModulatorTexture::ModulatorTexture(const QString &name, const QString &modulatorId, const QString& outputId,
                               Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, ST_TEXTURE, parent)
    , texFace_      (0)
{
}

void ModulatorTexture::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modtex", 1);

}

void ModulatorTexture::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modtex", 1);

}


void ModulatorTexture::copySettingsFrom(const Modulator *other)
{
    const ModulatorTexture * f = dynamic_cast<const ModulatorTexture*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorTexture::copySettingsFrom(" << other << ") "
                   "with non-tex type");
        return;
    }
}

bool ModulatorTexture::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorTexture::canBeModulator(NULL) called");

    return dynamic_cast<const ValueTextureInterface*>(o) != 0;
}


void ModulatorTexture::modulatorChanged_()
{
    texFace_ = 0;

    if (modulator() == 0)
        return;

    if (auto tex = dynamic_cast<ValueTextureInterface*>(modulator()))
        texFace_ = tex;

    else
    {
        MO_LOGIC_ERROR("illegal assignment of modulator '"
                       << modulator()->idName()
                       << "' to ModulatorTexture");
    }
}

const GL::Texture * ModulatorTexture::value(const RenderTime& time) const
{
    if (!modulator() || !modulator()->active(time))
        return 0;

    if (texFace_)
        return texFace_->valueTexture(outputChannel(), time);

    return 0;
}


} // namespace MO
