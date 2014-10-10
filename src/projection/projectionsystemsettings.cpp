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
#include "projectormapper.h"

namespace MO {


ProjectionSystemSettings::ProjectionSystemSettings()
{
}

void ProjectionSystemSettings::clear()
{
    dome_ = DomeSettings();
    projectors_.clear();
    cameras_.clear();
}

bool ProjectionSystemSettings::operator == (const ProjectionSystemSettings& o) const
{
    if (numProjectors() != o.numProjectors())
        return false;

    if (!(dome_ == o.dome_))
        return false;

    for (uint i=0; i<numProjectors(); ++i)
        if (   !(projectors_[i] == o.projectors_[i])
            || !(cameras_[i] == o.cameras_[i]))
            return false;

    return true;
}

void ProjectionSystemSettings::serialize(IO::XmlStream & io) const
{
    MO_DEBUG_IO("ProjectionSystemSettings::serialize()");

    dome_.serialize(io);

    for (auto & s : projectors_)
        s.serialize(io);
    for (auto & c : cameras_)
        c.serialize(io);
}

void ProjectionSystemSettings::deserialize(IO::XmlStream & io)
{
    MO_DEBUG_IO("ProjectionSystemSettings::deserialize()");

    clear();

    int count = 0;
    while (io.nextSubSection())
    {
        //MO_DEBUG_IO("ProjectionSystemSettings::deserialize() section='" << io.section() << "'");

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
        else if (io.isSection("camera"))
        {
            CameraSettings s;
            s.deserialize(io);
            if (count < cameras_.size())
                setCameraSettings(count, s);
            else
                // more cameras than projectors?
                MO_IO_WARNING(READ, "Camera settings for " << count << "th camera out of range ("
                              << cameras_.size());
            ++count;
            io.leaveSection();
        }
    }
}

void ProjectionSystemSettings::serialize(QByteArray & a) const
{
    MO_DEBUG_IO("ProjectionSystemSettings::serialize(QByteArray)");

    IO::XmlStream io;
    io.startWriting("mo-projection-system");
    serialize(io);
    io.stopWriting();
    a = io.data().toUtf8();
}

void ProjectionSystemSettings::deserialize(const QByteArray & a)
{
    MO_DEBUG_IO("ProjectionSystemSettings::deserialize(QByteArray)");

    IO::XmlStream io;
    io.setData(a);
    io.startReading("mo-projection-system");
    deserialize(io);
    io.stopReading();
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

void ProjectionSystemSettings::loadFile(const QString &filename)
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
//    if (idx >= projectors_.size())
//        abort();
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index " << idx << " out of range "
              "(" << projectors_.size() << ")");
    return projectors_[idx];
}

const CameraSettings& ProjectionSystemSettings::cameraSettings(int idx) const
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index " << idx << " out of range "
              "(" << projectors_.size() << ")");
    return cameras_[idx];
}

void ProjectionSystemSettings::setProjectorSettings(int idx, const ProjectorSettings &s)
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index " << idx << " out of range "
              "(" << projectors_.size() << ")");

    // check for unique id
    // but ignore the idx'th projector
    bool hasid = false;
    for (int i = 0; i<projectors_.size(); ++i)
        if (i != idx && projectors_[i].id() == s.id())
        {
            hasid = true;
            break;
        }

    projectors_[idx] = s;
    if (projectors_[idx].name().isEmpty())
        projectors_[idx].setName(QString("Projector %1").arg(idx+1));
    if (hasid)
        projectors_[idx].setId(getUniqueId());

}

void ProjectionSystemSettings::setCameraSettings(int idx, const CameraSettings &s)
{
    MO_ASSERT(idx >=0 && idx < cameras_.size(), "index " << idx << " out of range "
              "(" << cameras_.size() << ")");
    cameras_[idx] = s;
}

void ProjectionSystemSettings::appendProjector(const ProjectorSettings &set)
{
    insertProjector(projectors_.size(), set);
}

void ProjectionSystemSettings::insertProjector(int idx, const ProjectorSettings &set)
{
    ProjectorSettings s(set);
    if (s.name().isEmpty())
        s.setName(QString("Projector %1").arg(idx+1));
    if (hasId(s.id()))
    {
        //MO_DEBUG("Setting new projector id " << s.id() << " -> " << getUniqueId());
        s.setId(getUniqueId());
    }

    projectors_.insert(idx, s);
    cameras_.insert(idx, CameraSettings());
}

void ProjectionSystemSettings::removeProjector(int idx)
{
    MO_ASSERT(idx >=0 && idx < projectors_.size(), "index " << idx << " out of range "
              "(" << projectors_.size() << ")");
    projectors_.removeAt(idx);
    cameras_.removeAt(idx);
}

void ProjectionSystemSettings::calculateOverlapAreas(Float min_spacing, Float max_spacing)
{
    ProjectorMapper mapper;

    for (uint i=0; i<numProjectors(); ++i)
    {
        mapper.setSettings(domeSettings(), projectorSettings(i));
        projectors_[i].clearOverlapAreas();

        for (uint j=0; j<numProjectors(); ++j)
        if (i != j)
        {
            const QVector<Vec2>
                    area = mapper.getOverlapArea(projectorSettings(j), min_spacing, max_spacing);
            if (!area.isEmpty())
                projectors_[i].appendOverlapArea(area);
        }
    }
}

int ProjectionSystemSettings::getUniqueId() const
{
    int id = 0;
    for (auto & p : projectors_)
        id = std::max(id, p.id()+1);

    return id;
}

bool ProjectionSystemSettings::hasId(int id) const
{
    for (auto & p : projectors_)
        if (p.id() == id)
            return true;

    return false;
}

} // namespace MO
