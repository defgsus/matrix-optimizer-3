/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#include "keyboardstate.h"

namespace MO {

namespace {
    static KeyboardState * globalInstance_ = 0;
}

KeyboardState::KeyboardState()
{

}

void KeyboardState::clear()
{
    p_down.clear();
}

void KeyboardState::keyDown(int keycode)
{
    p_down.insert(keycode);
}

void KeyboardState::keyUp(int keycode)
{
    p_down.remove(keycode);
}

bool KeyboardState::isDown(int keycode) const
{
    return p_down.contains(keycode);
}

KeyboardState& KeyboardState::globalInstance()
{
    if (globalInstance_ == 0)
        globalInstance_ = new KeyboardState;
    return *globalInstance_;
}

} // namespace MO
