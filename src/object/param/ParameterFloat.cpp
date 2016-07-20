/** @file parameterfloat.cpp

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/


#include "ParameterFloat.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/control/TrackFloat.h"
#include "object/Scene.h"
#include "ModulatorFloat.h"

// make ParameterFloat useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterFloat*);
namespace { static int register_param = qMetaTypeId<MO::ParameterFloat*>(); }


namespace MO {

Double ParameterFloat::infinity = 1e50;

ParameterFloat::ParameterFloat(Object * object, const QString& id, const QString& name)
    :   Parameter(object, id, name),
        defaultValue_   (0.0),
        minValue_       (-infinity),
        maxValue_       (+infinity),
        smallStep_      (1.0),
        value_          (0.0),
        isFractional_   (false),
        isDefaultFractional_(false)
{
}


void ParameterFloat::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parf", 2);

    if (isFractional())
        io << qint8(1) << frac_;
    else
        io << qint8(0) << value_;

}

void ParameterFloat::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    const int ver = io.readHeader("parf", 2);

    if (ver == 1)
    {
        io >> value_;
        isFractional_ = false;
    }
    else
    {
        qint8 isF;
        io >> isF;
        if (isF)
        {
            io >> frac_;
            value_ = frac_.value();
            isFractional_ = true;
        }
        else
        {
            io >> value_;
            isFractional_ = false;
        }
    }
    //MO_PRINT("READ " << name() << ": " << isFractional_ << " " << frac_.toString());
}

void ParameterFloat::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterFloat*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    defFrac_ = p->defFrac_;
    value_ = p->value_;
    frac_ = p->frac_;
    isFractional_ = p->isFractional_;
    isDefaultFractional_ = p->isDefaultFractional_;
    minValue_ = p->minValue_;
    maxValue_ = p->maxValue_;
    smallStep_ = p->smallStep_;
}


bool ParameterFloat::isMinLimit() const { return minValue()+10. > -infinity; }
bool ParameterFloat::isMaxLimit() const { return maxValue()-10. < infinity; }

QString ParameterFloat::baseValueString(bool ) const
{
    if (isFractional())
        return frac_.toString();
    else
        return QString::number(baseValue());
}

QString ParameterFloat::valueString(const RenderTime& t, bool ) const
{
    return QString::number(value(t));
}

QString ParameterFloat::getDocType() const
{
    QString str = typeName();

    // get range string
    bool limmin = isMinLimit(),
         limmax = isMaxLimit();
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

int ParameterFloat::getModulatorTypes() const
{
    return 0xffffffff;/*
    return Object::T_TRACK_FLOAT
         | Object::T_SEQUENCE_FLOAT
         | Object::T_MODULATOR_OBJECT_FLOAT
         | Object::T_AUDIO_OBJECT;*/
}

Double ParameterFloat::getModulationValue(const RenderTime& time) const
{
    Double mod = 0;

    for (auto m : modulators())
        mod += static_cast<ModulatorFloat*>(m)->value(time);

    return mod;
}



Modulator * ParameterFloat::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloat(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}



void ParameterFloat::getValues(
        const RenderTime & time, Double timeIncrement, uint number, Double *ptr) const
{
    RenderTime rtime(time);
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(rtime);
        rtime.setSecond( rtime.second() + timeIncrement );
    }
}


void ParameterFloat::getValues(
        const RenderTime & time, Double timeIncrement, uint number, F32 *ptr) const
{
    RenderTime rtime(time);
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(rtime);
        rtime.setSecond( rtime.second() + timeIncrement );
    }
}


} // namespace MO
