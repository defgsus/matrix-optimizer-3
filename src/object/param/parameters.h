/** @file parameters.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.11.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_PARAMETERS_H
#define MOSRC_OBJECT_UTIL_PARAMETERS_H

#include <QList>

#include "object/object_fwd.h"
#include "types/float.h"
#include "types/int.h"
#include "io/filetypes.h"

namespace MO {
namespace MATH { class Timeline1D; }

class Parameters
{
public:
    Parameters(Object * parent);
    ~Parameters();

    // ------------------- io --------------------

    /** Serializes all installed parameters */
    void serialize(IO::DataStream&) const;
    /** Deserializes the installed parameters.
        Unknown parameters are skipped! */
    void deserialize(IO::DataStream&);

    // ----------------- getter ------------------

    Object * object() const { return object_; }

    /** Returns the parent object's idName() or empty string */
    const QString& idName() const;

    /** Returns the list of parameters for this object */
    const QList<Parameter*>& parameters() const { return parameters_; }

    /** Returns a list of parameters that are visible in the ObjectGraphView */
    QList<Parameter*> getVisibleGraphParameters() const;

    /** Returns the parameter with the given id, or NULL. */
    Parameter * findParameter(const QString& id);

    // -------------- setter --------------------

    /** Starts a new group which will contain all Parameters created afterwards.
        @p id is the PERSITANT name, to keep the gui-settings between sessions. */
    void beginParameterGroup(const QString& id, const QString& name);

    /** Ends the current Parameter group */
    void endParameterGroup();


    /** Creates the desired parameter,
        or returns an already created parameter object.
        When the Parameter was present before, all it's settings are still overwritten.
        If @p statusTip is empty, a default string will be set in the edit views. */
    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, Double minValue, Double maxValue, Double smallStep,
                bool editable = true, bool modulateable = true);

    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, bool editable = true, bool modulateable = true);

    ParameterFloat * createFloatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Double defaultValue, Double smallStep,
                bool editable = true, bool modulateable = true);


    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, Int minValue, Int maxValue, Int smallStep,
                bool editable, bool modulateable);

    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, Int smallStep,
                bool editable, bool modulateable);

    ParameterInt * createIntParameter(
                const QString& id, const QString& name, const QString& statusTip,
                Int defaultValue, bool editable, bool modulateable);

    ParameterSelect * createBooleanParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QString& offStatusTip, const QString& onStatusTip,
                bool defaultValue, bool editable = true, bool modulateable = true);

    ParameterSelect * createSelectParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QStringList& valueIds, const QStringList& valueNames,
                const QList<int>& valueList,
                int defaultValue, bool editable = true, bool modulateable = true);

    ParameterSelect * createSelectParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QStringList& valueIds, const QStringList& valueNames,
                const QStringList& statusTips,
                const QList<int>& valueList,
                int defaultValue, bool editable = true, bool modulateable = true);

    ParameterText * createTextParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QString& defaultValue,
                bool editable = true, bool modulateable = true);

    ParameterText * createTextParameter(
                const QString& id, const QString& name, const QString& statusTip,
                TextType textType,
                const QString& defaultValue,
                bool editable = true, bool modulateable = true);

    ParameterFilename * createFilenameParameter(
                const QString& id, const QString& name, const QString& statusTip,
                IO::FileType fileType,
                const QString& defaultValue = QString(), bool editable = true);


    /** Creates a timeline parameter.
        Ownership of @p defaultValue stays with caller. */
    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue = 0, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue,
                Double minTime, Double maxTime, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1D * defaultValue,
                Double minTime, Double maxTime, Double minValue, Double maxValue,
                bool editable = true);

private:

    Object * object_;
    QList<Parameter*> parameters_;
    QString curGroupId_,
            curGroupName_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_PARAMETERS_H
