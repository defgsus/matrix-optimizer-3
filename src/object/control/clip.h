/** @file clip.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_OBJECT_CLIP_H
#define MOSRC_OBJECT_CLIP_H

#include <QColor>

#include "object/object.h"
#include "object/param/parameterfloat.h"

namespace MO {

class Clip : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Clip)

    bool isClip() const Q_DECL_OVERRIDE { return true; }

    Type type() const Q_DECL_OVERRIDE { return T_CLIP; }

    //virtual QString infoName() const;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    /** Collects all sequences */
    virtual void childrenChanged() Q_DECL_OVERRIDE;

    // -------------- tree -------------------

    /** The ClipContainer, this Clip assigned to, or NULL */
    ClipController * clipContainer() const { return p_clipContainer_; }

    /** Returns the list of contained sequences.
        Counts sub-objects as well */
    const QList<Sequence*> & sequences() const { return p_sequences_; }

    /** Returns a list of all Clips that object @p o is influenced by. */
    static QList<Clip*> getAssociatedClips(Object * o);

    /** Returns a list of all Clips that Parameter @p p is influenced by.
        @p parentMask can be an or-combination of Object::Type flags.
        They controll, how much the parameter's parents, if any, are also
        checked for beeing modulated by clips. */
    static QList<Clip*> getAssociatedClips(Parameter * p, int parentMask = 0);

    // -------------- getter -------------------

    uint column() const;
    uint row() const;

    Double speed() const { return paramSpeed_->baseValue(); }

    // -------------- setter -------------------

    /** Sets the container that manages this clip */
    void setClipContainer(ClipController * c) { p_clipContainer_ = c; }

    /** Sets the position in the ClipContainer.
        @note The ClipContainer does not get notified of this change!
              Call ClipContainer::updateClipPositions()! */
    void setPosition(uint col, uint row) { setRow(row); setColumn(col); }
    void setRow(uint row);
    void setColumn(uint col);

    // -------------- trigger ------------------

    /** Starts the clip (assuming global time @p gtime).
        Generally don't use this. Use ClipContainer::triggerClip() instead. */
    void startClip(Double gtime);

    /** Stops the clip */
    void stopClip();

    // ---------- values/timing ----------------

    bool isPlaying() const { return p_running_; }

    /** Returns the global scene time when the clip was started */
    Double timeStarted() const { return p_timeStarted_; }


signals:

public slots:

private:

    QList<Sequence*> p_sequences_;

    ClipController * p_clipContainer_;

    Double p_timeStarted_;
    bool p_running_;

    ParameterFloat
            * paramSpeed_;
};



} // namespace MO

#endif // MOSRC_OBJECT_CLIP_H
