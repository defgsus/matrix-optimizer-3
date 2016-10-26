/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORFLOATMATRIX_H
#define MOSRC_OBJECT_PARAM_MODULATORFLOATMATRIX_H


#include "Modulator.h"
#include "types/time.h"
#include "math/FloatMatrix.h"

namespace MO {

class ValueFloatMatrixInterface;

class ModulatorFloatMatrix : public Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent.
    */
    ModulatorFloatMatrix(const QString& name, const QString& modulatorId,
                         const QString &outputId,
                         Parameter * p, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    /** Returns the matrix at given time */
    FloatMatrix value(const RenderTime& time) const;

    bool hasChanged(const RenderTime& time) const;

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    ValueFloatMatrixInterface * interface_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATORFLOATMATRIX_H
