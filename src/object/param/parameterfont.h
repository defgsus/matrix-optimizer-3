/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/22/2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERFONT_H
#define MOSRC_OBJECT_PARAM_PARAMETERFONT_H

#include <QFont>
#include "parameter.h"

namespace MO {

/** A parameter that selects a font */
class ParameterFont : public Parameter
{
public:

    ParameterFont(Object * object, const QString& idName, const QString& name);
    ~ParameterFont();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const Q_DECL_OVERRIDE { static QString s("font"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_FONT; }

    QString baseValueString(bool ) const override { return baseValue().family(); }
    QString valueString(const RenderTime& , bool ) const override
            { return baseValue().family(); }

    // ---------------- getter -----------------

    const QFont& baseValue() const { return p_font_; }
    const QFont& defaultValue() const { return p_def_; }

    // ---------------- setter -----------------

    void setValue(const QFont& font) { p_font_ = font; }
    void setDefaultValue(const QFont& f) { p_def_ = f; }

    // --------- gui -----------

    // ------ modulation --------

    Modulator * getModulator(
            const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE
    { Q_UNUSED(modulatorId); Q_UNUSED(outputId); return 0; }

private:

    QFont p_font_, p_def_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERFONT_H
