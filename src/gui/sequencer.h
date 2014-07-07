/** @file sequencer.h

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCER_H
#define MOSRC_GUI_SEQUENCER_H

#include <QWidget>

#include "types/float.h"

class QGridLayout;
class QScrollBar;

namespace MO {
class Track;
class Object;
class Sequence;
namespace GUI {

class TrackHeader;
class TrackView;
class Ruler;
class TimeBar;

class Sequencer : public QWidget
{
    Q_OBJECT
public:
    explicit Sequencer(QWidget *parent = 0);

signals:

    /** Emitted when a sequence was double-clicked */
    void sequenceSelected(Sequence *);

    /** User dragged the time bar */
    void sceneTimeChanged(Double);

public slots:

    /** Tells me that the scene time has changed */
    void setSceneTime(Double);

    /** Remove everything from this view. */
    void clearTracks();
    /** Insert the list of tracks and their sequences into the view.
        Previous content will be removed. */
    void setTracks(const QList<Track*>& tracks);

    /** Convenience function to insert all tracks of the object. */
    void setTracks(Object *, bool recursive = true);

protected:

    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

protected slots:

    void updateVScroll_();

private:

    void createWidgets_();

    TrackHeader * trackHeader_;
    TrackView * trackView_;
    Ruler * rulerSec_, * rulerFps_;
    QScrollBar * vScroll_;
    TimeBar * playBar_;

    QGridLayout * gridLayout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCER_H
