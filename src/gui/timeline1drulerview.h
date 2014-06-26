/** @file timeline1drulerview.h

    @brief Timeline1DView with rulers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#ifndef TIMELINE1DRULERVIEW_H
#define TIMELINE1DRULERVIEW_H

#include <QWidget>

class QGridLayout;

namespace MO {
namespace MATH { class Timeline1D; }
namespace GUI {

class Timeline1DView;
class Ruler;

class Timeline1DRulerView : public QWidget
{
    Q_OBJECT
public:
    explicit Timeline1DRulerView(MATH::Timeline1D * timeline = 0, QWidget *parent = 0);

    // -------------- components ------------------

    Timeline1DView * timelineView() { return timelineView_; }
    Ruler * rulerX() { return rulerX_; }
    Ruler * rulerY() { return rulerY_; }

signals:

public slots:

protected:

    Timeline1DView * timelineView_;
    Ruler * rulerX_, * rulerY_;

    QGridLayout * layout_;
};

} // namespace GUI
} // namespace MO

#endif // TIMELINE1DRULERVIEW_H
