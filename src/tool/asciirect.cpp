/** @file asciirect.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.02.2015</p>
*/

#include "asciirect.h"

namespace MO {

AsciiRect::AsciiRect() : AsciiRect(0, 0) { }

AsciiRect::AsciiRect(uint width, uint height)
    : p_rect_  (width * height)
    , p_w_      (width)
    , p_h_      (height)
{
    clear();
    p_table_ = ".,+*#";
}


QString AsciiRect::toString(const QChar newline) const
{
    QString r;

    for (uint y=0; y<p_h_; ++y)
    {
        for (uint x=0; x<p_w_; ++x)
            r += table(p_rect_[y*p_w_+x]);

        r += newline;
    }

    return r;
}

void AsciiRect::addPixelF(float x, float y, float value)
{
    int ix = x, iy = y;
    if (ix < 0 || ix >= int(p_w_) || iy < 0 || iy >= int(p_h_))
        return;

    float fx = x-ix, fy = y-iy,
          fx1 = 1.f - fx, fy1 = 1. - fy;

    pixel(ix, iy) += fx1 * fy1 * value;
    if (ix + 1 < int(p_w_))
    {
        pixel(ix+1, iy) += fx * fy1 * value;
        if (iy + 1 < int(p_h_))
            pixel(ix+1, iy+1) += fx * fy * value;
    }
    if (iy + 1 < int(p_h_))
        pixel(ix, iy+1) += fx1 * fy * value;
}


} // namespace MO
