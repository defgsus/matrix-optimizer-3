/** @file parametercallback.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H
#define MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H

#include <functional>

#include "parameter.h"

namespace MO {

/** A parameter that exposes a button that can be clicked */
class ParameterCallback : public Parameter
{
public:

    ParameterCallback(Object * object, const QString& idName, const QString& name);
    ~ParameterCallback();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const Q_DECL_OVERRIDE { static QString s("callback"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_CALLBACK; }

    //virtual void copyFrom(Parameter* ) Q_DECL_OVERRIDE { }

    QString baseValueString(bool ) const override { return QString(); }
    QString valueString(const RenderTime& , bool ) const override { return QString(); }

    // ---------------- getter -----------------

    std::function<void()> getCallback() const { return p_func_; }

    // ---------------- setter -----------------

    void setCallback(std::function<void()> func) { p_func_ = func; }

    // --------- gui -----------

    /** Execute the callback */
    void fire() { if (p_func_) p_func_(); }

    // ------ modulation --------

    int getModulatorTypes() const Q_DECL_OVERRIDE;
    Modulator * getModulator(
            const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;
    /** Calls fire() if input modulation > 0. */
    void fireIfInput(const RenderTime & time);
private:

    std::function<void()> p_func_;

};

} // namespace MO

#endif // MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H
