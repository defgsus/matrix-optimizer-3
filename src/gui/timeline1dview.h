/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/24</p>
*/

#ifndef MOSRC_GUI_TIMELINE1DVIEW_H
#define MOSRC_GUI_TIMELINE1DVIEW_H

#include <functional>

#include <QWidget>
#include <QSet>
#include <QVector>

#include "math/timeline1d.h"
#include "gui/util/viewspace.h"

namespace MO {
namespace GUI {

namespace PAINTER { class Grid; class ValueCurve; class ValueCurveData; class SequenceOverpaint; }

class Timeline1DView : public QWidget
{
    Q_OBJECT

public:

    enum Option
    {
        O_MovePointsX = 1,
        O_MovePointsY = 1<<1,
        O_AddRemovePoints = 1<<2,
        O_ChangePointType = 1<<3,

        O_MoveViewX = 1<<8,
        O_MoveViewY = 1<<9,
        O_ZoomViewX = 1<<10,
        O_ZoomViewY = 1<<11,

        O_MoveView = O_MoveViewX | O_MoveViewY,
        O_ZoomView = O_ZoomViewX | O_ZoomViewY,
        O_ChangeViewX = O_MoveViewX | O_ZoomViewX,
        O_ChangeViewY = O_MoveViewY | O_ZoomViewY,
        O_ChangeViewAll = O_ChangeViewX | O_ChangeViewY,

        O_MovePoints = O_MovePointsX | O_MovePointsY,
        O_EditAll = O_MovePoints | O_AddRemovePoints | O_ChangePointType,

        O_EnableAll = 0xffffffff
    };

    enum ClipboardType
    {
        C_NONE,
        C_WHOLE,
        C_SELECTION
    };

    // ---------- ctor -------------

    explicit Timeline1DView(MATH::Timeline1d * timeline = 0, QWidget *parent = 0);
    ~Timeline1DView();

    // ---------- getter -----------

    /** Returns assigned timeline */
    MATH::Timeline1d * timeline() const { return tl_; }

    int options() const { return options_; }

    /** is anyone selected? */
    bool isSelected() const { return !selectHashSet_.empty(); }

    static ClipboardType isTimelineInClipboard();

    /** Returns the installed sequence overpainter, or NULL */
    PAINTER::SequenceOverpaint * sequenceOverpaint() const { return sequenceOverpaint_; }
    PAINTER::ValueCurve * sequenceCurvePainter() const { return sequenceCurvePainter_; }

    // ----------- assignment ------

    /** Assigns a new (or no) Timeline1D and updates the widget.
        No ownership change. */
    void setTimeline(MATH::Timeline1d * timeline = 0);

    /** Assigns a sequencer overpainter for timelines in sequences */
    void setSequenceOverpaint(PAINTER::SequenceOverpaint * s) { sequenceOverpaint_ = s; }
    void setSequenceCurvePainter(PAINTER::ValueCurve * v) { sequenceCurvePainter_ = v; }

    /** Sets the options for the background grid as or-wise combination of PAINTER::Grid::Option */
    void setGridOptions(int options);

    /** Assigns locking/unlocking functions to the editor.
        Any change to the timeline data will be locked here. */
    void setLockFunctions(std::function<void()> lock,
                          std::function<void()> unlock) { lock_ = lock; unlock_ = unlock; }

    // --------- conversion screen/time/value --------

    const UTIL::ViewSpace& viewSpace() const { return space_; }

    Double screen2time(Double x) const;
    Double screen2value(Double y) const;
    int time2screen(Double time) const;
    int value2screen(Double val) const;

signals:

    /** Send when viewspace was changed by user */
    void viewSpaceChanged(const UTIL::ViewSpace&);

    /** Emitted after a change to the timeline data */
    void timelineChanged();

    void statusTipChanged(const QString&);


public slots:

    void setOptions(int options);

    /** Sets a new viewspace for the timeline */
    void setViewSpace(const UTIL::ViewSpace &, bool send_signal = false);

    /** Sets new viewspaces for the timeline and the overpainter and sequence-curve. */
    void setViewSpace(const UTIL::ViewSpace &timelineSpace,
                      const UTIL::ViewSpace &overpainterSpace, bool send_signal = false);

    /** Fits the whole curve into view */
    void fitToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Fits the selected curve into view */
    void fitSelectionToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Clear any selections previously made.
        This should be called when a timeline was loaded. */
    void unselect();

    void copyAll();
    void copySelection();
    void paste();

protected slots:

    void slotPointContextMenu_();
    void slotEmptyContextMenu_();

protected:

    // _________ PRIVATE TYPES _________

    enum Action_
    {
        A_NOTHING,
        A_START_DRAG_SPACE,
        A_DRAG_SPACE,
        A_DRAG_SELECTED,
        A_START_SELECT_FRAME,
        A_SELECT_FRAME
    };

    enum RectStyle_
    {
        RS_NORMAL,
        RS_HOVER,
        RS_SELECTED,
        RS_UPDATE
    };

    struct DragPoint_
    {
        bool valid;
        MATH::Timeline1d::Point
            oldp,
            newp;
        MATH::Timeline1d::TpList::iterator
            it;
        DragPoint_() { }
        DragPoint_(const MATH::Timeline1d::TpList::iterator& it) : valid(true), oldp(it->second), newp(it->second), it(it) { }
    };

    class Locker_
    {
        Timeline1DView * v;
    public:
        Locker_(Timeline1DView * v) : v(v) { if (v->lock_) v->lock_(); }
        ~Locker_() { if (v->unlock_) v->unlock_(); }
    };

    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

    /** Sets statusTip property and emits statusTipChanged() */
    void setStatusTip_(const QString&);

    void clearHover_();
    void setHover_(const MATH::Timeline1d::Point&);
    bool isHover_() const;
    MATH::Timeline1d::TpList::iterator hoverPoint_();

    void clearSelect_();
    void addSelect_(const MATH::Timeline1d::Point&, bool do_swap = false);
    void addSelect_(const QRect& rect, bool do_swap = false);
    void selectAll_();
    void selectDirection_(int dir);

    /** Returns the screen rect for a given point */
    QRect handleRect_(const MATH::Timeline1d::Point&, RectStyle_ rs);
    void updateAroundPoint_(const MATH::Timeline1d::Point&);
    void updateDerivatives_(MATH::Timeline1d::TpList::iterator it, int leftRight = 1);

    void changeScale_(int screenX, int screenY, Double factorX, Double factorY);
    void fitToView_(Double tmin, Double tmax, bool fitX, bool fitY, int marginInPixels);

    void changePointType_(MATH::Timeline1d::Point::Type t);
    void moveSelected_(Double dx, Double dy);
    void addPoint_(Double t, Double v);

    QCursor defaultCursor_() const;

    /** Limits the time to the screen when moving/zooming is disabled. */
    Double limitX_(Double time) const;
    /** Limits the value to the screen when moving/zooming is disabled. */
    Double limitY_(Double value) const;

    // ____________ MEMBER _____________

    MATH::Timeline1d * tl_;

    UTIL::ViewSpace space_, ospace_;

    PAINTER::Grid * gridPainter_;
    PAINTER::ValueCurve * valuePainter_;
    PAINTER::ValueCurveData * valuePainterData_;
    PAINTER::ValueCurve * sequenceCurvePainter_;
    PAINTER::SequenceOverpaint * sequenceOverpaint_;

    std::function<void()> lock_, unlock_;

    // ---- config ----

    int options_;

    int handleRadius_,
        handleRadiusHovered_,
        handleRadiusSelected_;
    Double
        zoomChange_;
    Qt::Modifier
        modifierSelectFrame_,
        modifierMultiSelect_,
        modifierMoveVert_;

    // ---- interaction ----

    MATH::Timeline1d::TpHash
        hoverHash_,
        hoverCurveHash_;

    QSet<MATH::Timeline1d::TpHash>
        selectHashSet_;

    QVector<DragPoint_>
        dragPoints_;

    UTIL::ViewSpace dragStartSpace_;

    Action_ action_;
    QPoint dragStart_,
           popupClick_;

    QRect selRect_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
