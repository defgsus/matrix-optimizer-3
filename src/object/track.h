/** @file track.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#ifndef MOSRC_OBJECT_TRACK_H
#define MOSRC_OBJECT_TRACK_H

#include <QStringList>

#include "object.h"

namespace MO {

class Track : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Track);

    bool isTrack() const { return true; }

    /** Returns the idName()s of the sequences that belong to this track. */
    const QStringList& sequenceIds() const { return sequenceIds_; }

    /** Pointers to all sequences on this track. */
    const QList<Sequence*>& sequences() const { return baseSequences_; }

    virtual void addSequence(Sequence *);
    virtual void removeSequence(Sequence *);

    virtual void collectModulators();

signals:

public slots:

private:

    /** Ids of all sequences on this track */
    QStringList sequenceIds_;

    /** sequences on this track. */
    QList<Sequence *> baseSequences_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRACK_H
