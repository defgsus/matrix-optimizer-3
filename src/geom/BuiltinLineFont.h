/** @file builtinlinefont.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.12.2014</p>
*/

#ifndef MOSRC_GEOM_BUILTINLINEFONT_H
#define MOSRC_GEOM_BUILTINLINEFONT_H

#include <cinttypes>

namespace MO {

/** Mono-spaced font set in GL_LINES data style.
    Lines are in the coordinate range [0,1] */
class BuiltInLineFont
{
    BuiltInLineFont();
    ~BuiltInLineFont();
public:

    struct Font
    {
        /** The character */
        uint16_t utf16;

        /** Number of lines */
        unsigned int num;

        /** Pointer to 'num' lines, 'num' * 2 vertices or 'num' * 4 coordinates */
        float * data;
    };

    /** Returns the given character, or NULL */
    static const Font * getFont(uint16_t utf16);


private:
    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_GEOM_BUILTINLINEFONT_H
