/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#ifndef MOSRC_TOOL_KEYBOARDSTATE_H
#define MOSRC_TOOL_KEYBOARDSTATE_H

#include <QSet>

namespace MO {


/** A class to collect the pressed-state of keys */
class KeyboardState
{
public:
    KeyboardState();

    void keyDown(int keycode);
    void keyUp(int keycode);
    void clear();

    bool isDown(int keycode) const;

    static KeyboardState& globalInstance();

private:
    QSet<int> p_down;
};

} // namespace MO


#endif // MOSRC_TOOL_KEYBOARDSTATE_H
