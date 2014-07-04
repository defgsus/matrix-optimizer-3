/** @file trackview.h

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#ifndef MOSRC_GUI_TRACKVIEW_H
#define MOSRC_GUI_TRACKVIEW_H

#include <QWidget>
#include <QList>
#include <QHash>

#include "util/viewspace.h"

class QGraphicsScene;

namespace MO {
class Track;
class Scene;
namespace GUI {

class SequenceWidget;

class TrackView : public QWidget
{
    Q_OBJECT
public:
    explicit TrackView(QWidget *parent = 0);

    /** Returns height of particular track */
    int trackHeight(Track *) const;

signals:

    /** Emitted when the number or height of tracks changed */
    void tracksChanged();

public slots:

    void setViewSpace(const UTIL::ViewSpace&);

    void setScene(Scene *);

    /** Remove everything from this view. */
    void clearTracks();
    /** Insert the list of tracks and their sequences into the view.
        Previous content will be removed. */
    void setTracks(const QList<Track*>& tracks, bool send_signal = false);

private:

    void createSequenceWidgets_();
    void updateViewSpace_();



    UTIL::ViewSpace space_;

    Scene * scene_;
    QList<Track*> tracks_;
    QList<SequenceWidget*> sequenceWidgets_;
    QHash<QString, int> trackHeights_;

    // ---- config ----

    int defaultTrackHeight_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKVIEW_H
