/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/26/2015</p>
*/

#ifndef MOSRC_IO_MOUSESTATE_H
#define MOSRC_IO_MOUSESTATE_H

#include <QSet>
#include <QPoint>
#include <QPointF>
#include <QSize>

namespace MO {

/** A class to collect the state of a mouse */
class MouseState
{
public:
    MouseState();

    /** Sets the position in pixels and a normalized position,
        given the mouse position range */
    void setPos(const QPoint& pos, const QSize& size);
    /** Same as setPos() but when mouse is down. */
    void setDragPos(const QPoint& pos, const QSize& size);
    void keyDown(int key);
    void keyUp(int key);
    void clear();

    bool isDown(int key) const;
    const QPoint& pos() const { return p_pos; }
    QPointF posNorm() const { return p_posNorm; }
    QPointF posNormSigned() const { return p_posNorm * 2. - QPointF(1., 1.); }
    const QPoint& dragPos() const { return p_dragPos; }
    QPointF dragPosNorm() const { return p_dragPosNorm; }
    QPointF dragPosNormSigned() const { return p_dragPosNorm * 2. - QPointF(1., 1.); }

    /** Access to a global instance that is fed by the GL::Window */
    static MouseState& globalInstance();

private:
    QSet<int> p_down;
    QPoint p_pos, p_dragPos;
    QPointF p_posNorm, p_dragPosNorm;
};

} // namespace MO

#endif // MOSRC_IO_MOUSESTATE_H
