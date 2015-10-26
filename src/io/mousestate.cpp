/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/26/2015</p>
*/

#include "mousestate.h"

namespace MO {

namespace {
    static MouseState * globalInstance_ = 0;
}

MouseState::MouseState()
{

}

void MouseState::clear()
{
    p_down.clear();
    p_pos = QPoint();
    p_posNorm = QPointF();
    p_dragPos = QPoint();
    p_dragPosNorm = QPointF();
}

void MouseState::keyDown(int keycode)
{
    p_down.insert(keycode);
}

void MouseState::keyUp(int keycode)
{
    p_down.remove(keycode);
}

void MouseState::setPos(const QPoint &pos, const QSize &size)
{
    p_pos = pos;
    p_posNorm.rx() = size.width() > 0 ? qreal(pos.x()) / size.width() : 0.;
    p_posNorm.ry() = size.height() > 0 ? qreal(pos.y()) / size.height() : 0.;
}

void MouseState::setDragPos(const QPoint &pos, const QSize &size)
{
    p_dragPos = pos;
    p_dragPosNorm.rx() = size.width() > 0 ? qreal(pos.x()) / size.width() : 0.;
    p_dragPosNorm.ry() = size.height() > 0 ? qreal(pos.y()) / size.height() : 0.;
}

bool MouseState::isDown(int keycode) const
{
    return p_down.contains(keycode);
}

MouseState& MouseState::globalInstance()
{
    if (globalInstance_ == 0)
        globalInstance_ = new MouseState;
    return *globalInstance_;
}

} // namespace MO
