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

class QAction;

namespace MO {
class Track;
class Scene;
class Sequence;
namespace GUI {

class TrackHeader;
class SequenceWidget;

class TrackView : public QWidget
{
    Q_OBJECT
public:
    explicit TrackView(QWidget *parent = 0);

    /** Returns the header view associated with this view. */
    TrackHeader * trackHeader() const { return header_; }

    /** Returns the actual height of all tracks
        which can be different from the widget's height */
    int realHeight() const { return maxHeight_; }

    /** Returns height of particular track */
    int trackHeight(Track *) const;

    /** Returns the y position of the track */
    int trackY(Track *) const;

    /** Returns number of tracks displayed. */
    int numTracks() const { return tracks_.size(); }

signals:

    void viewSpaceChanged(const UTIL::ViewSpace&);

    /** Emitted when the number or height of tracks changed */
    void tracksChanged();

    /** Emitted when a track was selected */
    void trackSelected(Track *);

public slots:

    /** Sets the horizontal view space of the widget */
    void setViewSpace(const UTIL::ViewSpace&);

    /** Sets the vertical offset into the view */
    void setVerticalOffset(int);

    /** Remove everything from this view. */
    void clearTracks();
    /** Insert the list of tracks and their sequences into the view.
        Previous content will be removed. */
    void setTracks(const QList<Track*>& tracks, bool send_signal = false);

    /** Updates the view for the given Track */
    void updateTrack(Track *);

    /** update from sequences */
    void sequenceTimeChanged(MO::Sequence *);

protected:

    void mousePressEvent(QMouseEvent * );
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

private:

    //void deleteSequenceWidgets_(Track *);
    void createSequenceWidgets_(Track *);
    void updateWidgetsViewSpace_();
    void updateWidgetViewSpace_(SequenceWidget *);
    void calcTrackY_();
    void createEditActions_();

    SequenceWidget * widgetForSequence_(Sequence *) const;

    /** Returns track for screen y position, or NULL */
    Track * trackForY(int y) const;
    /** Whole track rectangle */
    QRect trackRect_(Track *) const;

    UTIL::ViewSpace space_;

    Scene * scene_;
    QList<Track*> tracks_;

    TrackHeader * header_;

    QList<SequenceWidget*> sequenceWidgets_;
    QHash<QString, int> trackHeights_;
    QHash<Track*, int> trackY_;
    int offsetY_, maxHeight_;

    Track * selTrack_;
    QList<QAction*> editActions_;
    /** createSequenceWidgets() will focus the widget containing this sequence */
    Sequence * nextFocusSequence_;
    Double currentTime_;

    QPoint dragStartPos_;
    Double dragStartTime_, dragStartSeqTime_;
    /** Sequence to be dragged around */
    Sequence * dragSequence_;
    Track * dragStartTrack_, * dragEndTrack_;

    // ---- config ----

    int defaultTrackHeight_,
        trackSpacing_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKVIEW_H
