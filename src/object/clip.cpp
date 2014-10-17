/** @file clip.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/
#include <QDebug>
#include "clip.h"
#include "io/datastream.h"
#include "io/error.h"
#include "sequence.h"
#include "clipcontainer.h"

namespace MO {

MO_REGISTER_OBJECT(Clip)

Clip::Clip(QObject *parent)
    : Object        (parent),
      clipContainer_(0),
      timeStarted_  (0),
      running_      (false),
      column_       (0),
      row_          (0),
      color_        (QColor(50,100,50))
{
    setName("Clip");
}

void Clip::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clip", 2);

    io << column_ << row_;

    // v2
    io << color_;
}

void Clip::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("clip", 2);

    io >> column_ >> row_;

    if (ver >= 2)
        io >> color_;
}

void Clip::createParameters()
{
    Object::createParameters();

    beginParameterGroup("clip", tr("clip"));

        p_speed_ = createFloatParameter("speed", tr("speed"),
                                        tr("The speed multiplier for all sequences in the clip"),
                                        1, 0.1, true, false);

    endParameterGroup();
}


void Clip::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void Clip::onParentChanged()
{
    Object::onParentChanged();

    clipContainer_ = qobject_cast<ClipContainer*>(parentObject());

    if (clipContainer_)
        clipContainer_->findNextFreeSlot(column_, row_, true);
}

void Clip::childrenChanged()
{
    // get all sequences and sub-sequences
    sequences_ = findChildObjects<Sequence>(QString(), true);
}

QList<Clip*> Clip::getAssociatedClips(Object *o)
{
    QSet<Clip*> clips;

    auto mod = o->getModulatingObjects();
    for (auto m : mod)
        if (m->parentObject() && m->parentObject()->isClip())
            clips.insert(static_cast<Clip*>(m->parentObject()));

    QList<Clip*> ret;
    for (auto c : clips)
        ret << c;
    return ret;
}

QList<Clip*> Clip::getAssociatedClips(Parameter * p, bool alsoObject)
{
    QSet<Clip*> clips;

    auto mod = p->getModulatingObjects();
    for (auto m : mod)
        if (m->parentObject() && m->parentObject()->isClip())
            clips.insert(static_cast<Clip*>(m->parentObject()));

    if (alsoObject && p->object())
    {
        auto mod = p->object()->getModulatingObjects();
        for (auto m : mod)
            if (m->parentObject() && m->parentObject()->isClip())
                clips.insert(static_cast<Clip*>(m->parentObject()));
    }

    QList<Clip*> ret;
    for (auto c : clips)
        ret << c;
    return ret;
}


void Clip::startClip(Double gtime)
{
    timeStarted_ = gtime;
    running_ = true;
}

void Clip::stopClip()
{
    running_ = false;
}


} // namespace MO
