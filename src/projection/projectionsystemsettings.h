/** @file projectionsystemsettings.h

    @brief Complete settings for projectors and dome

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/27/2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H
#define MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H

#include <QList>

#include "domesettings.h"
#include "projectorsettings.h"

namespace MO {
namespace IO { class XmlStream; }

class ProjectionSystemSettings
{
public:
    ProjectionSystemSettings();

    // ------------ io ------------

    void serialize(IO::XmlStream&) const;
    void deserialize(IO::XmlStream&);

    // ---------- getter ----------

    int numProjectors() const { return projectors_.size(); }

    const DomeSettings& domeSettings() const { return dome_; }

    const ProjectorSettings& projectorSettings(int idx) const;

    // --------- setter -----------

    void clear();

    void setDomeSettings(const DomeSettings& s) { dome_ = s; }

    void setProjectorSettings(int idx, const ProjectorSettings& s);

    void appendProjector(const ProjectorSettings& s);
    void insertProjector(int idx, const ProjectorSettings& s);
    void removeProjector(int idx);

private:

    DomeSettings dome_;
    QList<ProjectorSettings> projectors_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTIONSYSTEMSETTINGS_H
