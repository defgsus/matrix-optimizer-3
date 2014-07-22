/** @file scenesettings.cpp

    @brief ViewSpaces and stuff for scene objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "scenesettings.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/track.h"
#include "object/sequence.h"
#include "object/sequencefloat.h"


namespace MO {
namespace GUI {


SceneSettings::SceneSettings(QObject *parent)
    :   QObject(parent),
      useCompression_         (true),
      defaultTrackHeight_     (30)

{
}

SceneSettings::SceneSettings(const SceneSettings &other)
    :   QObject         (other.parent()),
        viewSpaces_     (other.viewSpaces_),
        trackHeights_   (other.trackHeights_)
{
}

SceneSettings& SceneSettings::operator=(const SceneSettings& other)
{
    viewSpaces_ = other.viewSpaces_;
    trackHeights_ = other.trackHeights_;

    return *this;
}

void SceneSettings::serialize(IO::DataStream &io) const
{
    MO_DEBUG_IO("SceneSettings::serialize(" << &io << ")");

    io.writeHeader("scenesettings", 1);

    io << trackHeights_;

    // viewspaces
    io << (quint32)viewSpaces_.size();
    for (auto v = viewSpaces_.begin(); v != viewSpaces_.end(); ++v)
    {
        io << v.key();
        v->serialize(io);
    }
}

void SceneSettings::deserialize(IO::DataStream &io)
{
    MO_DEBUG_IO("SceneSettings::deserialize(" << &io << ")");

    io.readHeader("scenesettings", 1);

    io >> trackHeights_;

    // viewspaces
    quint32 n;
    io >> n;

    QHash<QString, UTIL::ViewSpace> temp;
    for (quint32 i=0; i<n; ++i)
    {
        QString key;
        UTIL::ViewSpace space;
        io >> key;
        space.deserialize(io);
        temp.insert(key, space);
    }

    viewSpaces_ = temp;
}

void SceneSettings::saveFile(const QString &filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not store scene-settings in '"
                    << filename << "'\n"
                    << file.errorString());

    IO::DataStream io(&file);

    io.writeHeader("scenesettingsfile", 1);

    io << (quint8)useCompression_;

    if (!useCompression_)
        serialize(io);
    else
    {
        QByteArray data;
        IO::DataStream ioc(&data, QIODevice::WriteOnly);
        serialize(ioc);
        io << qCompress(data, 9);
    }
}

void SceneSettings::loadFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not read scene-settings from '"
                    << filename << "'\n" << file.errorString());

    IO::DataStream io(&file);

    io.readHeader("scenesettingsfile", 1);

    quint8 compressed;
    io >> compressed;

    if (!compressed)
        deserialize(io);
    else
    {
        QByteArray data;
        io >> data;
        data = qUncompress(data);
        IO::DataStream iou(data);
        deserialize(iou);
    }
}

QString SceneSettings::getSettingsFileName(const QString &sceneFilename) const
{
    return
          QFileInfo(sceneFilename).absolutePath()
        + "/"
        + QFileInfo(sceneFilename).completeBaseName()
        + ".mo3-gui";
}


void SceneSettings::clear()
{
    viewSpaces_.clear();
    trackHeights_.clear();
}


void SceneSettings::setViewSpace(const Object *obj, const UTIL::ViewSpace &viewspace)
{
    viewSpaces_.insert(obj->idName(), viewspace);
}

UTIL::ViewSpace SceneSettings::getViewSpace(const Object *obj)
{
    // return saved viewspace
    auto i = viewSpaces_.find(obj->idName());
    if (i != viewSpaces_.end())
        return i.value();

    // create new space
    UTIL::ViewSpace space;

    // adjust/initialize to certain objects

    if (const Sequence * seq = qobject_cast<const Sequence*>(obj))
    {
        space.setScaleX(seq->length());
    }

    if (const SequenceFloat * seqf = qobject_cast<const SequenceFloat*>(obj))
    {
        Double minv, maxv;
        seqf->getMinMaxValue(0, seqf->length(), minv, maxv);
        space.setY(minv);
        space.setScaleY(maxv-minv);
    }

    return space;
}


void SceneSettings::setTrackHeight(const Track * track, int h)
{
    trackHeights_.insert(track->idName(), h);
}

int SceneSettings::getTrackHeight(const Track * track) const
{
    auto i = trackHeights_.find(track->idName());
    if (i == trackHeights_.end())
        return defaultTrackHeight_;

    return i.value();
}


} // namespace GUI
} // namespace MO
