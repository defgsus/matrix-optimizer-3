/** @file parameterint.cpp

    @brief Parameter of type Int

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/11/2014</p>
*/

#include "parameterint.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "modulatorfloat.h"

// make ParameterInt useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterInt*);
namespace { static int register_param = qMetaTypeId<MO::ParameterInt*>(); }


namespace MO {

Int ParameterInt::infinity = 1<<30;

ParameterInt::ParameterInt(Object * object, const QString& id, const QString& name)
    :   Parameter       (object, id, name),
        defaultValue_   (0.0),
        minValue_       (-infinity),
        maxValue_       (+infinity),
        smallStep_      (1.0),
        value_          (0.0)
{
}


void ParameterInt::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("pari", 1);

    // store future-save
    io << (qint64)value_;

}

void ParameterInt::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("pari", 1);

    qint64 v;
    io >> v;
    value_ = v;
}


QString ParameterInt::getDocType() const
{
    QString str = typeName();

    // get range string
    bool limmin = minValue() > -infinity,
         limmax = maxValue() < infinity;
    if (limmin || limmax)
    {
        if (limmin && limmax)
            str += " " + QObject::tr("range [%1, %2]").arg(minValue()).arg(maxValue());
        else if (limmin)
            str += " >= " + QString::number(minValue());
        else
            str += " <= " + QString::number(maxValue());
    }

    return str;
}

int ParameterInt::getModulatorTypes() const
{
    return Object::T_TRACK_FLOAT
        | Object::T_SEQUENCE_FLOAT
        | Object::T_MODULATOR_OBJECT_FLOAT;
}

Int ParameterInt::getModulationValue(const RenderTime& time) const
{
    Float mod = 0;

    for (auto m : modulators())
        mod += static_cast<ModulatorFloat*>(m)->value(time);

    return (Int)mod;
}



Modulator * ParameterInt::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloat(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}



} // namespace MO
