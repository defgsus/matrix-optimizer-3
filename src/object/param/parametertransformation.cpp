/** @file parametertransformation.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#if 1

#include "parametertransformation.h"
#include "modulatortransformation.h"
#include "object/scene.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

ParameterTransformation::ParameterTransformation(Object * object, const QString& id, const QString& name)
    :   Parameter(object, id, name),
        defaultValue_   (1.0),
        value_          (1.0)
{
}


void ParameterTransformation::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parm", 1);

    for (int i=0; i<4; ++i)
        for (int j=0; j<4; ++j)
            io << value_[i][j];
}

void ParameterTransformation::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parm", 1);

    for (int i=0; i<4; ++i)
        for (int j=0; j<4; ++j)
            io >> value_[i][j];

}

void ParameterTransformation::copyFrom(Parameter* other)
{
    auto p = dynamic_cast<ParameterTransformation*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    value_ = p->value_;
}

int ParameterTransformation::getModulatorTypes() const
{
    return //Object::TG_TRANSFORMATION
            Object::TG_REAL_OBJECT;
}

Mat4 ParameterTransformation::value(const RenderTime& time) const
{
    for (auto m : modulators())
    {
        Mat4 mat = static_cast<ModulatorTransformation*>(m)->value(time);
        return mat;
    }

    return defaultValue_;
}



Modulator * ParameterTransformation::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorTransformation(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}


/*
void ParameterTransformation::getValues(Double time, uint thread, Double timeIncrement, uint number, Double *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time, thread);
        time += timeIncrement;
    }
}
*/

} // namespace MO

#endif // 0
