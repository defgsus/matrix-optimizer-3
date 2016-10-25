/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERFLOATMATRIX_H
#define MOSRC_OBJECT_PARAM_PARAMETERFLOATMATRIX_H


#include "Parameter.h"
#include "types/time.h"
#include "object/param/FloatMatrix.h"

namespace MO {
namespace GUI { class FloatMatrixDialog; }

class ParameterFloatMatrix : public Parameter
{
public:

    ParameterFloatMatrix(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("float-matrix"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_FLOAT_MATRIX; }

    virtual void copyFrom(Parameter* other) Q_DECL_OVERRIDE;

    virtual QString getDocType() const Q_DECL_OVERRIDE;

    QString baseValueString(bool ) const override {
        return QString::fromStdString(baseValue().layoutString()); }
    QString valueString(const RenderTime& t, bool ) const override
        { return QString::fromStdString(value(t).layoutString()); }

    // ---------------- getter -----------------

    FloatMatrix defaultValue() const { return defaultValue_; }

    FloatMatrix value(const RenderTime& time) const;

    const FloatMatrix& baseValue() const { return baseValue_; }

    bool isBaseValueEqual(const FloatMatrix& v) const { return baseValue_ == v; }

    bool hasChanged(const RenderTime&) const;

    // ---------------- setter -----------------

    void setDefaultValue(const FloatMatrix& v) { defaultValue_ = v; }
    void setValue(const FloatMatrix& v);

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    /** Receives modulation value at time */
    Double getModulationValue(const RenderTime& time) const;

    virtual Modulator * getModulator(
            const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

    // ---- GUI ----

    /** Opens a GUI::FloatMatrixDialog to edit the contents of this parameter.
        @note Don't let parent be the ParameterWidget as it would destroy the
        Dialog when selecting another object. */
    GUI::FloatMatrixDialog* openEditDialog(QWidget* parent = nullptr);

private:

    FloatMatrix
        defaultValue_,
        baseValue_;
    mutable std::vector<bool> hasChanged_;

    GUI::FloatMatrixDialog* diag_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERFLOATMATRIX_H
