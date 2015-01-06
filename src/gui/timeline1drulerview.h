/** @file timeline1drulerview.h

    @brief Timeline1DView with rulers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#ifndef TIMELINE1DRULERVIEW_H
#define TIMELINE1DRULERVIEW_H

#include <QWidget>

#include "gui/util/viewspace.h"

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

    MATH::Timeline1D * timeline() const;

    Timeline1DView * timelineView() { return timelineView_; }

    Ruler * rulerX() { return rulerX_; }
    Ruler * rulerY() { return rulerY_; }

    const UTIL::ViewSpace& viewSpace() const;

    // ----------- assignment ------

    /** Assigns a new (or no) Timeline1D */
    void setTimeline(MATH::Timeline1D * timeline = 0);

    /** Sets the options for the background grid as or-wise combination of PAINTER::Grid::Option */
    void setGridOptions(int options);

signals:

    /** Send when viewspace was changed by user */
    void viewSpaceChanged(const UTIL::ViewSpace&);

    /** Emitted after a change to the timeline data */
    void timelineChanged();

public slots:

    /** Sets option flags as defined in Timeline1DView::Option */
    void setOptions(int options);

    /** Sets a new viewspace for the timeline */
    void setViewSpace(const UTIL::ViewSpace &, bool send_signal = false);

    /** Fits the whole curve into view */
    void fitToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Fits the selected curve into view */
    void fitSelectionToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Clear any selections previously made.
        This should be called when a timeline was loaded. */
    void unselect();

protected:

    Timeline1DView * timelineView_;
    Ruler * rulerX_, * rulerY_;

    QGridLayout * layout_;
};

} // namespace GUI
} // namespace MO

#endif // TIMELINE1DRULERVIEW_H
