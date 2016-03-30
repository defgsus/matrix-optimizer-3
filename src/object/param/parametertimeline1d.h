/** @file parametertimeline1d.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERTIMELINE1D_H
#define MOSRC_OBJECT_PARAM_PARAMETERTIMELINE1D_H

#include "parameter.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace MATH { class Timeline1d; }

class ParameterTimeline1D : public Parameter
{
public:

    /** Use to set unbounded limits */
    static Double infinity;


    ParameterTimeline1D(Object * object, const QString& idName, const QString& name);
    ~ParameterTimeline1D();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("timeline1d"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TIMELINE1D; }

    virtual void copyFrom(Parameter* other) Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString&, const QString&) Q_DECL_OVERRIDE { return 0; }

    QString baseValueString(bool ) const override { return "XXX"; }
    QString valueString(const RenderTime& , bool ) const override { return "XXX"; }

    // ---------------- getter -----------------

    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double minTime() const { return minTime_; }
    Double maxTime() const { return maxTime_; }

    /** Returns the default timeline, or NULL if none is set */
    const MATH::Timeline1d * defaultTimeline() const { return default_; }

    /** Always returns the default timeline and creates one if neccessary. */
    const MATH::Timeline1d & getDefaultTimeline();

    const MATH::Timeline1d & baseValue() { return tl_ ? (const MATH::Timeline1d &)*tl_ : getDefaultTimeline(); }

    // ---------------- setter -----------------

    /** Read/Write access to timeline.
        This will create an internal MATH::Timeline1D if neccessary. */
    MATH::Timeline1d * timeline();

    /** Sets a new internal timeline, ownership is taken.
        If @p tl == NULL, the internal timeline is deleted. */
    void setTimeline(MATH::Timeline1d * tl);

    /** Sets new internal timeline data */
    void setValue(const MATH::Timeline1d &);

    /** Sets new default content for the internal timeline */
    void setDefaultTimeline(const MATH::Timeline1d &);

    void setMinValue(Double v) { minValue_ = v; }
    void setMaxValue(Double v) { maxValue_ = v; }
    void setMinTime(Double v) { minTime_ = v; }
    void setMaxTime(Double v) { maxTime_ = v; }

    void setNoMinValue() { minValue_ = infinity; }
    void setNoMaxValue() { maxValue_ = infinity; }
    void setNoMinTime() { minTime_ = infinity; }
    void setNoMaxTime() { maxTime_ = infinity; }

    /** Resets the internal timeline to default */
    void reset();

    // --------- gui -----------

    /** Opens a modal dialog to edit the timeline.
        Returns true, when a change was made.
        @note The scene MUST be present for this call!
        */
    bool openEditDialog(QWidget * parent = 0);

private:

    MATH::Timeline1d * tl_, * default_;

    Double
        minValue_,
        maxValue_,
        minTime_,
        maxTime_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERTIMELINE1D_H
