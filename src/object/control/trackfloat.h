/** @file trackfloat.h

    @brief Track for float f(x) values

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2014</p>
*/

#ifndef MOSRC_OBJECT_TRACKFLOAT_H
#define MOSRC_OBJECT_TRACKFLOAT_H


#include "track.h"
#include "object/interface/valuefloatinterface.h"


namespace MO {

class TrackFloat : public Track, public ValueFloatInterface
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(TrackFloat);

    virtual Type type() const { return T_TRACK_FLOAT; }

    /** Pointers to all float sequences on this track. */
    const QList<SequenceFloat*>& sequences() const { return sequences_; }

    virtual void collectModulators();

    virtual QList<Object*> getModulatingObjects() const;

    Double valueFloat(uint channel, Double time, uint thread) const Q_DECL_OVERRIDE;

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(Double time, uint thread, Double timeIncrement, uint number, Double * ptr) const;

    /** Writes @p number values starting at @p time into the pointer */
    void getValues(Double time, uint thread, Double timeIncrement, uint number, F32 * ptr) const;

signals:

public slots:

private:

    /** Float sequences on this track. */
    QList<SequenceFloat*> sequences_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRACKFLOAT_H
