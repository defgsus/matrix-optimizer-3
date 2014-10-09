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
namespace MATH { class Timeline1D; }

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

    virtual Modulator * getModulator(const QString &) Q_DECL_OVERRIDE { return 0; }

    // ---------------- getter -----------------

    Double minValue() const { return minValue_; }
    Double maxValue() const { return maxValue_; }
    Double minTime() const { return minTime_; }
    Double maxTime() const { return maxTime_; }

    /** Returns the default value */
    const MATH::Timeline1D * defaultTimeline() const { return default_; }

    // ---------------- setter -----------------

    /** Read/Write access to timeline.
        This will create an internal MATH::Timeline1D if neccessary. */
    MATH::Timeline1D * timeline();

    /** Sets a new internal timeline, ownership is taken.
        If @p tl == NULL, the internal timeline is deleted. */
    void setTimeline(MATH::Timeline1D * tl);

    /** Sets new internal timeline data */
    void setTimeline(const MATH::Timeline1D &);

    /** Sets new default content for the internal timeline */
    void setDefaultTimeline(const MATH::Timeline1D &);

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

#ifndef MO_CLIENT
    /** Opens a modal dialog to edit the timeline.
        Returns true, when a change was made.
        @note The scene MUST be present for this call!
        */
    bool openEditDialog(QWidget * parent = 0);
#endif

private:

    MATH::Timeline1D * tl_, * default_;

    Double
        minValue_,
        maxValue_,
        minTime_,
        maxTime_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERTIMELINE1D_H
