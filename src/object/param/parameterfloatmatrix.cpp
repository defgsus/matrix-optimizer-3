/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include "parameterfloatmatrix.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "modulatorfloatmatrix.h"


namespace MO {

ParameterFloatMatrix::ParameterFloatMatrix(
        Object * object, const QString& id, const QString& name)
    :   Parameter(object, id, name)
{
}


void ParameterFloatMatrix::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parfm", 1);

    io << baseValue_;

}

void ParameterFloatMatrix::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parfm", 1);

    io >> baseValue_;
}

void ParameterFloatMatrix::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterFloatMatrix*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    baseValue_ = p->baseValue_;
}

QString ParameterFloatMatrix::getDocType() const
{
    QString str = typeName();

    // XXX

    return str;
}

int ParameterFloatMatrix::getModulatorTypes() const
{
    return 0xffffffff;
}

FloatMatrix ParameterFloatMatrix::value(const RenderTime& time) const
{
    for (auto m : modulators())
        return static_cast<ModulatorFloatMatrix*>(m)->value(time);

    return baseValue_;
}



Modulator * ParameterFloatMatrix::getModulator(
        const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloatMatrix(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}




} // namespace MO
