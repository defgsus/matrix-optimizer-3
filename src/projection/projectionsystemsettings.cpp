/** @file projectionsystemsettings.cpp

    @brief Complete settings for projectors and dome

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/27/2014</p>
*/

#include "projectionsystemsettings.h"
#include "io/error.h"
#include "io/xmlstream.h"

namespace MO {


ProjectionSystemSettings::ProjectionSystemSettings()
{
}

void ProjectionSystemSettings::clear()
{
    dome_ = DomeSettings();
    projectors_.clear();
}

void ProjectionSystemSettings::serialize(IO::XmlStream & io) const
{
    dome_.serialize(io);

    for (auto & s : projectors_)
        s.serialize(io);
}

void ProjectionSystemSettings::deserialize(IO::XmlStream & io)
{
    clear();

    while (io.nextSubSection())
    {
        //MO_IO_ERROR(VERSION_MISMATCH, "no section found in projection-system xml");

        if (io.isSection("dome"))
        {
            dome_.deserialize(io);
            io.leaveSection();
        }
        else if (io.isSection("projector"))
        {
            ProjectorSettings s;
            s.deserialize(io);
            appendProjector(s);
            io.leaveSection();
        }
    }
}

const ProjectorSettings& ProjectionSystemSettings::projectorSettings(int idx) const
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index out of range");
    return projectors_[idx];
}

void ProjectionSystemSettings::setProjectorSettings(int idx, const ProjectorSettings &s)
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index out of range");
    projectors_[idx] = s;
}

void ProjectionSystemSettings::appendProjector(const ProjectorSettings &s)
{
    insertProjector(projectors_.size(), s);
}

void ProjectionSystemSettings::insertProjector(int idx, const ProjectorSettings &s)
{
    projectors_.insert(idx, s);
}

void ProjectionSystemSettings::removeProjector(int idx)
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index out of range");
    projectors_.removeAt(idx);
}


} // namespace MO
