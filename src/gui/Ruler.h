/** @file ruler.h

    @brief basic adjustable grid view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSRC_GUI_RULER_H
#define MOSRC_GUI_RULER_H

#include <QWidget>

#include "gui/util/ViewSpace.h"

namespace MO {
namespace GUI {
namespace PAINTER { class Grid; }

class Ruler : public QWidget
{
    Q_OBJECT
public:

    enum Option
    {
        O_DrawX = 1,
        O_DrawY = 2,
        O_DrawTextX = 4,
        O_DrawTextY = 8,
        O_DragX = 16,
        O_DragY = 32,
        O_ZoomX = 64,
        O_ZoomY = 128,

        O_DrawAll = O_DrawX | O_DrawY | O_DrawTextX | O_DrawTextY,

        O_DragAll = O_DragX | O_DragY,
        O_ZoomAll = O_ZoomX | O_ZoomY,
        O_ChangeViewAll = O_DragAll | O_ZoomAll,

        O_EnableAllX = Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_DragX | Ruler::O_ZoomX,
        O_EnableAllY = Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_DragY | Ruler::O_ZoomY,
        O_EnableAll = O_DrawAll | O_ChangeViewAll
    };

    // ------------- ctor ------------------

    explicit Ruler(QWidget *parent = 0);

    // ------------- getter ----------------

    const UTIL::ViewSpace& viewSpace() const { return space_; }

    int options() const { return options_; }

signals:

    /** Send when the viewspace has changed */
    void viewSpaceChanged(const UTIL::ViewSpace&);

    /** Send for a double-click.
        If any option regarding X is enabled, @p value will be the x value,
        else it will be the y value. */
    void doubleClicked(Double value);

    void fitRequest();

public slots:

    void setViewSpace(const UTIL::ViewSpace&, bool send_signal = false);

    void setOptions(int options);

protected:

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent*);

    QCursor defaultCursor_() const;

private:

    enum Action_
    {
        A_NOTHING,
        A_DRAG_SPACE
    };

    UTIL::ViewSpace space_;
    PAINTER::Grid * gridPainter_;

    int options_;

    Action_ action_;

    QPoint dragStart_, lastPos_;
    UTIL::ViewSpace dragStartSpace_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_RULER_H
