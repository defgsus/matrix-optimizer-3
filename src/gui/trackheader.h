/** @file trackheader.h

    @brief Track/Names area working with TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_TRACKHEADER_H
#define MOSRC_GUI_TRACKHEADER_H

#include <QWidget>
#include <QList>

namespace MO {
class Track;
namespace GUI {

class TrackView;
class TrackHeaderWidget;

class TrackHeader : public QWidget
{
    Q_OBJECT
public:
    explicit TrackHeader(TrackView * trackView, QWidget *parent = 0);

signals:

public slots:

    /** Remove everything from this view. */
    void clearTracks();
    /** Insert the list of tracks into the view.
        Previous content will be removed. */
    void setTracks(const QList<Track*>& tracks);

private:

    void updateWidgetsViewSpace_();

    TrackView * trackView_;

    QList<Track*> tracks_;
    QList<TrackHeaderWidget*> widgets_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKHEADER_H
