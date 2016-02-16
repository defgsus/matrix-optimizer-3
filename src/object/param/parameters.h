/** @file parameters.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.11.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_PARAMETERS_H
#define MOSRC_OBJECT_UTIL_PARAMETERS_H

#include <functional>

#include <QList>
#include <QMap>
#include <QSet>

#include "object/object_fwd.h"
#include "types/vector.h"
#include "types/time.h"
#include "io/filetypes.h"

namespace MO {
namespace MATH { class Timeline1d; }

/** Container for all Parameters (modulatable or not) of an MO::Object */
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

    /** Returns the parameter with the given name, or NULL. */
    Parameter * findParameterName(const QString& name);

    /** Returns an automatic html from all parameters. */
    QString getParameterDoc() const;

    /** Returns the parameters per group name */
    QMap<QString, QList<Parameter*>> getParameterGroups() const;

    /** Returns a list of all objects that modulate any of the parameters.
        If @p recursive is true, the whole modulation chain will be resolved.
        This functions only works after modulator objects are resolved. */
    QList<Object*> getModulatingObjectsList(bool recursive) const;

    /** Adds all objects connections, that modulate any of the parameters.
        If @p recursive is true, the whole modulation chain will be resolved.
        This functions only works after modulator objects are resolved. */
    void getModulatingObjects(ObjectConnectionGraph&, bool recursive) const;

    /** Returns a list of all ids that modulate any of the parameters.
        This functions works also before modulator objects are resolved. */
    QList<QString> getModulatorIds() const;

    /** Adds all ids to the set, that modulate any of the parameters.
        This functions works also before modulator objects are resolved. */
    void getModulatorIds(QSet<QString>&) const;

    /** Pulling change request for Parameters.
        XXX Only for ParameterTexture right now. */
    bool haveInputsChanged(const RenderTime & time) const;

    // -------------- setter --------------------

    /** Removes all modulators with the given id.
        @note low-level function, not really part of interface. */
    void removeModulators(const QList<QString>& ids);

    // ------------ parameter creation ----------

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

    ParameterImageList * createImageListParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const QStringList& defaultValue = QStringList(), bool editable = true);

    ParameterCallback * createCallbackParameter(const QString& id, const QString& name, const QString& statusTip,
                std::function<void()> callback, bool modulateable = true);

    ParameterTexture * createTextureParameter(
                const QString& id, const QString& name, const QString& statusTip);

    ParameterGeometry * createGeometryParameter(
                const QString& id, const QString& name, const QString& statusTip);

    /** Creates a timeline parameter.
        Ownership of @p defaultValue stays with caller.
        The @p defaultValue timeline is copied, if specified and can be deleted
        when this call returns. */
    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1d * defaultValue = 0, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1d * defaultValue,
                Double minTime, Double maxTime, bool editable = true);

    ParameterTimeline1D * createTimeline1DParameter(
                const QString& id, const QString& name, const QString& statusTip,
                const MATH::Timeline1d * defaultValue,
                Double minTime, Double maxTime, Double minValue, Double maxValue,
                bool editable = true);


    ParameterSelect * createTextureFormatParameter(
                const QString& id, const QString& name, const QString& statusTip,
                int minChan = 1, int maxChan = 4);
    ParameterSelect * createTextureTypeParameter(
                const QString& id, const QString& name, const QString& statusTip,
                int defaultBitsize = 8);
    /** Combines the two values from texture format and type parameters and
        returns the correct format GLenum. */
    static int getTexFormat(int format, int type);


    ParameterTransformation * createTransformationParameter(
            const QString& id, const QString& name, const QString& statusTip, const Mat4& defaultValue = Mat4(1.));

    ParameterFont * createFontParameter(
            const QString& id, const QString& name, const QString& statusTip);
private:

    Object * object_;
    QList<Parameter*> parameters_;
    QString curGroupId_,
            curGroupName_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_PARAMETERS_H
