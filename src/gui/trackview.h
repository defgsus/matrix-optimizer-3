/** @file trackview.h

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#ifndef MOSRC_GUI_TRACKVIEW_H
#define MOSRC_GUI_TRACKVIEW_H

#include <QGraphicsView>
//#include <QWidget>

#include "util/viewspace.h"

class QGraphicsScene;

namespace MO {
namespace GUI {


class TrackView : public //QWidget
                        QGraphicsView
{
    Q_OBJECT
public:
    explicit TrackView(QWidget *parent = 0);

signals:

public slots:

    void setViewSpace(const UTIL::ViewSpace&);

private:

    void createItems_();

    QGraphicsScene * scene_;

    UTIL::ViewSpace space_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TRACKVIEW_H
