/** @file projectionsystemsettings.cpp

    @brief Complete settings for projectors and dome

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/27/2014</p>
*/

#include "projectionsystemsettings.h"
#include "io/error.h"
#include "io/log.h"
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
    MO_DEBUG_IO("ProjectionSystemSettings::serialize()");

    dome_.serialize(io);

    for (auto & s : projectors_)
        s.serialize(io);
}

void ProjectionSystemSettings::deserialize(IO::XmlStream & io)
{
    MO_DEBUG_IO("ProjectionSystemSettings::deserialize()");

    clear();

    while (io.nextSubSection())
    {
        //MO_DEBUG_IO("ProjectionSystemSettings::deserialize() section='" << io.section() << "'");
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

void ProjectionSystemSettings::saveFile(const QString &filename) const
{
    MO_DEBUG_IO("ProjectionSystemSettings::saveFile('" << filename << "')");

    IO::XmlStream io;
    io.startWriting("mo-projection-system");
    serialize(io);
    io.stopWriting();
    io.save(filename);
}

void ProjectionSystemSettings::laodFile(const QString &filename)
{
    MO_DEBUG_IO("ProjectionSystemSettings::loadFile('" << filename << "')");

    IO::XmlStream io;
    io.load(filename);
    io.startReading("mo-projection-system");
    deserialize(io);
    io.stopReading();
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

void ProjectionSystemSettings::appendProjector(const ProjectorSettings &set)
{
    ProjectorSettings s(set);
    if (s.name().isEmpty())
        s.setName(QString("projector %1").arg(projectors_.size()+1));
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
