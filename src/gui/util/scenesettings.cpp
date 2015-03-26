/** @file scenesettings.cpp

    @brief ViewSpaces and stuff for scene objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <QDebug>

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
#include "object/param/parameters.h"


namespace MO {
namespace GUI {


SceneSettings::SceneSettings(QObject *parent)
    : QObject               (parent),
      useCompression_       (true),
      defaultTrackHeight_   (30)

{
}

SceneSettings::SceneSettings(const SceneSettings &other)
    :   QObject         (other.parent())
{
    *this = other;
}

SceneSettings& SceneSettings::operator=(const SceneSettings& other)
{
    viewSpaces_ = other.viewSpaces_;
    trackHeights_ = other.trackHeights_;
    paramGroupExpanded_ = other.paramGroupExpanded_;
    treeExpanded_ = other.treeExpanded_;
    gridPos_ = other.gridPos_;

    return *this;
}

void SceneSettings::clear()
{
    viewSpaces_.clear();
    trackHeights_.clear();
    paramGroupExpanded_.clear();
    treeExpanded_.clear();
    gridPos_.clear();
    readVersion_ = -1;
}

void SceneSettings::serialize(IO::DataStream &io) const
{
    MO_DEBUG_IO("SceneSettings::serialize(" << &io << ")");

    io.writeHeader("scenesettings", 2);

    io << trackHeights_;

    // viewspaces
    io << (quint32)viewSpaces_.size();
    for (auto v = viewSpaces_.begin(); v != viewSpaces_.end(); ++v)
    {
        io << v.key();
        v->serialize(io);
    }

    // v2
    io << paramGroupExpanded_;
}

void SceneSettings::deserialize(IO::DataStream &io)
{
    MO_DEBUG_IO("SceneSettings::deserialize(" << &io << ")");

    clear();

    const int ver = readVersion_ = io.readHeader("scenesettings", 4);

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

    if (ver >= 2)
        io >> paramGroupExpanded_;

    if (ver >= 3 && ver <= 4)
        io >> treeExpanded_;

    if (ver == 4)
        io >> gridPos_;

//    qDebug() << treeExpanded_;
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
        + QDir::separator()
        + QFileInfo(sceneFilename).completeBaseName()
        + ".mo3-gui";
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
        space.setScaleX(std::min(10.0, seq->length()));
        space.setY(-1.2);
        space.setScaleY(2.4);
    }

    if (const SequenceFloat * seqf = qobject_cast<const SequenceFloat*>(obj))
    {
        Double minv, maxv;
        seqf->getMinMaxValue(0, 10/*seqf->length()*/, minv, maxv, MO_GUI_THREAD);
        space.setY(minv-0.2);
        space.setScaleY(maxv-minv+0.4);
    }

    return space;
}


void SceneSettings::setTrackHeight(const Track * track, int h)
{
    trackHeights_[track->idName()] = h;
}

int SceneSettings::getTrackHeight(const Track * track) const
{
    auto i = trackHeights_.find(track->idName());
    if (i == trackHeights_.end())
        return defaultTrackHeight_;

    return i.value();
}
/*
void SceneSettings::setParameterGroupExpanded(
        const Object * obj, const QString &groupId, bool expanded)
{
    const QString id = obj->idName() + "/" + groupId;
    if (!expanded)
        paramGroupExpanded_.remove(id);
    else
        paramGroupExpanded_.insert(id);
}

bool SceneSettings::getParameterGroupExpanded(const Object * obj, const QString &groupId) const
{
    const QString id = obj->idName() + "/" + groupId;
    return paramGroupExpanded_.contains(id);
}
*/

void SceneSettings::copySettings(const Object *dst, const Object *src)
{
    copySettings(dst->idName(), src->idName());
}

void SceneSettings::copySettings(const QString& dst, const QString& src)
{
    if (viewSpaces_.contains(src))
        viewSpaces_.insert(dst, viewSpaces_.value(src));

    if (trackHeights_.contains(src))
        trackHeights_.insert(dst, trackHeights_.value(src));

    for (auto & s : paramGroupExpanded_)
    if (s.startsWith(src + "/"))
    {
        const QString suff = s.mid(src.size());
        paramGroupExpanded_.insert(dst + suff);
    }

    for (auto & s : treeExpanded_)
    if (s.endsWith("/" + src))
    {
        const QString pref = s.left(s.size() - src.size());
        treeExpanded_.insert(pref + dst);
    }
}


void SceneSettings::updateTreeForCompatibility(Object * o)
{
    auto i = gridPos_.find(o->idName() + "/0");
    if (i != gridPos_.end())
        o->setAttachedData(i.value(), Object::DT_GRAPH_POS);

    auto j = treeExpanded_.find("0/" + o->idName());
    if (j != treeExpanded_.end())
        o->setAttachedData(true, Object::DT_GRAPH_EXPANDED);

    for (const QString & i : paramGroupExpanded_)
    if (i.startsWith(o->idName()))
    {
        QString group = i.mid(i.indexOf("/")+1);
        o->setAttachedData(true, Object::DT_PARAM_GROUP_EXPANDED, group);
    }

    // traverse childs
    for (auto c : o->childObjects())
        updateTreeForCompatibility(c);
}

} // namespace GUI
} // namespace MO
