/** @file trackviewoverpaint.h

    @brief Overpainter for TrackView (for everything that needs to be on top of sequences)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

#ifndef MOSRC_GUI_TRACKVIEWOVERPAINT_H
#define MOSRC_GUI_TRACKVIEWOVERPAINT_H

#include <QWidget>

namespace MO {
namespace GUI {

class TrackView;

class TrackViewOverpaint : public QWidget
{
    Q_OBJECT
public:
    explicit TrackViewOverpaint(TrackView * trackView, QWidget *parent = 0);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);

private:
    TrackView * trackView_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKVIEWOVERPAINT_H
