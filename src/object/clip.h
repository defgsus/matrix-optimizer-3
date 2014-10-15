/** @file clip.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_OBJECT_CLIP_H
#define MOSRC_OBJECT_CLIP_H

#include <QColor>

#include "object.h"

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

    virtual void onParentChanged() Q_DECL_OVERRIDE;

    // -------------- tree -------------------

    /** The ClipContainer, this Clip is on, or NULL */
    ClipContainer * clipContainer() const { return clipContainer_; }

    /** Returns the list of contained sequences.
        Counts sub-objects as well */
    const QList<Sequence*> & sequences() const { return sequences_; }

    // -------------- getter -------------------

    uint column() const { return column_; }
    uint row() const { return row_; }

    const QColor color() const { return color_; }

    // -------------- setter -------------------

    /** Sets the position in the ClipContainer.
        @note The ClipContainer does not get notified of this change!
              Call ClipContainer::updateClipPositions()! */
    void setPosition(uint col, uint row) { column_ = col; row_ = row; }
    void setRow(uint row) { setPosition(column(), row); }
    void setColumn(uint col) { setPosition(col, row()); }

    void setColor(const QColor& color) { color_ = color; }

    // -------------- trigger ------------------

    /** Starts the clip (assuming global time @p gtime).
        Generally don't use this. Use ClipContainer::triggerClip() instead. */
    void startClip(Double gtime);

    /** Stops the clip */
    void stopClip();

    // -------------- values -------------------

    bool isPlaying() const { return running_; }

    /** Returns the global scene time when the clip was started */
    Double timeStarted() const { return timeStarted_; }

    //Double value(Double time, uint thread) const;

signals:

public slots:

private:

    QList<Sequence*> sequences_;

    ClipContainer * clipContainer_;

    Double timeStarted_;
    bool running_;

    uint column_, row_;

    QColor color_;
};



} // namespace MO

#endif // MOSRC_OBJECT_CLIP_H
