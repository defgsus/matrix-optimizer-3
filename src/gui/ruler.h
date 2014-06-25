/** @file ruler.h

    @brief basic adjustable grid view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/25/2014</p>
*/

#ifndef RULER_H
#define RULER_H

#include <QWidget>

#include "gui/util/viewspace.h"

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

        O_DrawAll = O_DrawX | O_DrawY | O_DrawTextX | O_DrawTextY,
        O_DragAll = O_DragX | O_DragY,
        O_EnableAll = 0xffff
    };

    // ------------- ctor ------------------

    explicit Ruler(QWidget *parent = 0);

    // ------------- getter ----------------

    const UTIL::ViewSpace& viewSpace() const { return space_; }

    int options() const { return options_; }

signals:

    /** Send when the viewspace has changed */
    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    void setViewSpace(const UTIL::ViewSpace&, bool send_signal = false);

    void setOptions(int options);

protected:

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

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

    QPoint dragStart_;
    UTIL::ViewSpace dragStartSpace_;
};


} // namespace GUI
} // namespace MO

#endif // RULER_H
