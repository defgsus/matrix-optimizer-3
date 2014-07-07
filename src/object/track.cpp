/** @file track.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include <QDebug>

#include "track.h"
#include "io/datastream.h"
#include "io/error.h"
#include "sequence.h"
#include "io/log.h"


namespace MO {

//MO_REGISTER_OBJECT(Track)

Track::Track(QObject *parent) :
    Object(parent)
{
}

void Track::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("Track", 2);

    io << sequenceIds_;
}

void Track::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    int ver = io.readHeader("Track", 2);

    if (ver >= 2)
        io >> sequenceIds_;
}


void Track::addSequence(Sequence * s)
{
    MO_DEBUG_MOD("Track("<<idName()<<")::addSequence(" << s->idName() << ")");

    MO_ASSERT(s, "trying to add NULL Sequence to track '" << idName() << "'");
    MO_ASSERT(!sequenceIds_.contains(s->idName()), "Track::addSequence() duplicate sequence '"
              << s->idName() << "' on track '" << idName() << "'");

    sequenceIds_.append(s->idName());
}

void Track::removeSequence(Sequence * s)
{
    MO_DEBUG_MOD("Track("<<idName()<<")::removeSequence(" << s->idName() << ")");

    MO_ASSERT(s, "trying to remove NULL Sequence from track '" << idName() << "'");

    sequenceIds_.removeOne(s->idName());
    // tell sequence
    s->removeFromTrack(this);
}

void Track::collectModulators()
{
    MO_DEBUG_MOD("Track("<<idName()<<")::collectModulators()");

    baseSequences_.clear();

    Object * root = rootObject();

    for (auto const &id : sequenceIds_)
    {
        Object * o = root->findChildObject(id, true);

        if (auto s = qobject_cast<Sequence*>(o))
        {
            baseSequences_.append(s);
            // tell the sequence
            s->addToTrack(this);
        }
        else
            MO_WARNING("track '" << idName()
                       << "' could not find sequence '" << id << "'");
    }
}


} // namespace MO
