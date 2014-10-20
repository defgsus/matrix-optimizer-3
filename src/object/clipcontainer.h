/** @file clipcontainer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_OBJECT_CLIPCONTAINER_H
#define MOSRC_OBJECT_CLIPCONTAINER_H

#include <QVector>
#include <QMap>

#include "object.h"

namespace MO {

class ClipContainer : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ClipContainer)

    bool isClipContainer() const Q_DECL_OVERRIDE { return true; }

    Type type() const Q_DECL_OVERRIDE { return T_CLIP_CONTAINER; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void childrenChanged() Q_DECL_OVERRIDE;

    // -------------- getter ------------------

    uint numberRows() const { return rows_; }
    uint numberColumns() const { return cols_; }

    /** Returns the name of the column */
    QString columnName(uint index) const;
    /** Returns the name of the row */
    QString rowName(uint index) const;

    /** Adjusts @p column and @p row to point at the next free position.
        Returns true when a free slot was found.
        Returns true in @p resized if the container had to be resized. */
    bool findNextFreeSlot(uint & column, uint & row, bool resizeIfNecessary = false, bool * resized = 0);

    // -------------- setter ------------------

    /** Sets the extent of the clip area.
        The area will always be as large as the contained clips need it */
    void setNumber(uint cols, uint rows);
    void setNumberRows(uint rows) { setNumber(numberColumns(), rows); }
    void setNumberColumns(uint cols) { setNumber(cols, numberRows()); }

    // -------------- clips -------------------

    /** Call this after changes to the positions of Clips.
        This might also change the size of the ClipContainer to fit in
        all the Clip positions. */
    void updateClipPositions();

    /** Returns the clip at the specific position, or NULL */
    Clip * clip(uint column, uint row) const;

    /** Returns the one clip that is playing on @p column, or NULL */
    Clip * playingClip(uint column) const;

    /** Queues a clip for playing at global scene time @p gtime.
        Another playing clip on the same column is triggered for stopping. */
    void triggerClip(Clip * clip, Double gtime);

    /** Queues a clip for stopping at global scene time @p gtime */
    void triggerStopClip(Clip * clip, Double gtime);

    /** Queues a whole row for playing at global scene time @p gtime.
        Any playing clip on each column is triggered for stopping. */
    void triggerRow(uint index, Double gtime);

    /** Queues a column for stopping at global scene time @p gtime.
        The currently playing clip on the column is triggered for stopping. */
    void triggerStopColumn(uint index, Double gtime);

signals:

    void clipTriggered(Clip *);
    void clipStopTriggered(Clip *);
    void clipStarted(Clip *);
    void clipStopped(Clip *);

public slots:

private:

    uint rows_, cols_,
        maxRow_, maxCol_;

    /** [y][x] */
    QVector<Clip*> clips_;

    QMap<SamplePos, Clip*> triggerStart_, triggerStop_;

    QMap<uint, QString>
        columnNames_,
        rowNames_;
};



} // namespace MO

#endif // MOSRC_OBJECT_CLIPCONTAINER_H
