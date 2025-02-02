/** @file projectionsystemsettings.h

    @brief Complete settings for projectors and dome

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/27/2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H
#define MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H

#include <QList>

#include "DomeSettings.h"
#include "ProjectorSettings.h"
#include "CameraSettings.h"

namespace MO {
namespace IO { class XmlStream; }

class ProjectionSystemSettings
{
public:
    ProjectionSystemSettings();

    // ------------ io ------------

    void serialize(IO::XmlStream&) const;
    void deserialize(IO::XmlStream&);

    void serialize(QByteArray&) const;
    void deserialize(const QByteArray&);

    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);

    // ---------- getter ----------

    bool operator == (const ProjectionSystemSettings&) const;
    bool operator != (const ProjectionSystemSettings& o) const { return !(*this == o); }

    uint numProjectors() const { return projectors_.size(); }

    int blendMethod() const { return blendMethod_; }
    Float blendMargin() const { return blendMargin_; }

    /** Returns the settings for the dome */
    const DomeSettings& domeSettings() const { return dome_; }
    /** Returns the settings for each projector */
    const ProjectorSettings& projectorSettings(int idx) const;
    /** Returns the settings for each camera of each projector */
    const CameraSettings& cameraSettings(int idx) const;

    /** Returns a projector id that is not currently used */
    int getUniqueId() const;

    /** Returns true if the projector id is currently used */
    bool hasId(int id) const;

    // --------- setter -----------

    void clear();

    /** -1 = none, 0 = sb, 1 = mh */
    void setBlendMethod(int method) { blendMethod_ = method; }
    void setBlendMargin(Float margin) { blendMargin_ = margin; }

    void setDomeSettings(const DomeSettings& s) { dome_ = s; }

    /** Replaces the projector.
        If it's name is empty it will be called projector x (x depending on the position).
        The projector id will be made unique if necessary. */
    void setProjectorSettings(int idx, const ProjectorSettings& s);
    void setCameraSettings(int idx, const CameraSettings& s);

    /** Appends the projector to the internal list.
        If it's name is empty it will be called "Projector x" (x depending on the new position).
        The projector id will be made unique if necessary. */
    void appendProjector(const ProjectorSettings& s);
    /** Inserts the projector into the internal list.
        If it's name is empty it will be called "Projector x" (x depending on the new position).
        The projector id will be made unique if necessary. */
    void insertProjector(int idx, const ProjectorSettings& s);
    void removeProjector(int idx);

    // --------- blending ---------

    /** Calculates the overlapping areas between each projector */
    void calculateOverlapAreas(Float min_spacing = 0.05, Float max_spacing = 1.0);

private:

    DomeSettings dome_;
    QList<ProjectorSettings> projectors_;
    QList<CameraSettings> cameras_;

    int blendMethod_;
    Float blendMargin_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H
