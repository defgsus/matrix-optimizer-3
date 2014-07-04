/** @file sequencer.h

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCER_H
#define MOSRC_GUI_SEQUENCER_H

#include <QWidget>

class QGridLayout;
class QScrollBar;

namespace MO {
class Track;
namespace GUI {

class TrackHeader;
class TrackView;
class Ruler;

class Sequencer : public QWidget
{
    Q_OBJECT
public:
    explicit Sequencer(QWidget *parent = 0);

signals:

public slots:

    /** Remove everything from this view. */
    void clearTracks();
    /** Insert the list of tracks and their sequences into the view.
        Previous content will be removed. */
    void setTracks(const QList<Track*>& tracks);

private:

    void createWidgets_();

    TrackHeader * trackHeader_;
    TrackView * trackView_;
    Ruler * rulerSec_, * rulerFps_;
    QScrollBar * vScroll_;

    QGridLayout * gridLayout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCER_H
