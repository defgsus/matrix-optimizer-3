/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#ifndef MOSRC_GUI_TIMELINE1DVIEW_H
#define MOSRC_GUI_TIMELINE1DVIEW_H

#include <QWidget>

#include "types/float.h"

namespace MO {

class Timeline1D;

namespace GUI {

class Timeline1DView : public QWidget
{
    Q_OBJECT
public:
    explicit Timeline1DView(Timeline1D * timeline = 0, QWidget *parent = 0);

    /** Assigns a new (or no) Timeline1D */
    void setTimeline(Timeline1D * timeline = 0);

    Double screen2time(int x) const;

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

    Timeline1D * tl_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
