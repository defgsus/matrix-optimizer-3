/** @file asciirect.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.02.2015</p>
*/

#ifndef MOSRC_TOOL_ASCIIRECT_H
#define MOSRC_TOOL_ASCIIRECT_H

#include <cmath>
#include <vector>
#include <QString>

namespace MO {

class AsciiRect
{
public:
    /** Creates empty rect */
    AsciiRect();
    AsciiRect(uint width, uint height);

    // --------- conversion ------------

    /** Returns the table entry at index, or 0 */
    QChar table(uint index) const
        { return index < (uint)p_table_.size() ? p_table_.at(index) : 0; }

    /** Returns the character for a value in range [0,1] */
    QChar table(float x) const
        { return x < 0 ? table(0u) : table(uint(x * p_table_.size() + .5f)); }

    /** Convert the current image to ascii */
    QString toString(const QChar newline = '\n') const;

    // --------- access ---------------

    void clear() { for (float & v : p_rect_) v = 0.f; }
    void clip(float mi = 0.f, float ma = 1.f)
        { for (float & v : p_rect_) v = std::max(mi, std::min(ma, v )); }

    float  pixel(uint x, uint y) const { return p_rect_[y * p_w_ + x]; }
    float& pixel(uint x, uint y) { return p_rect_[y * p_w_ + x]; }

    /** Adds smoothly, x and y are in [0,1] */
    void addPixelF(float x, float y, float value);

private:

    std::vector<float> p_rect_;
    QString p_table_;
    uint p_w_, p_h_;
};

} // namespace MO

#endif // MOSRC_TOOL_ASCIIRECT_H
