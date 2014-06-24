/** @file viewspace.h

    @brief view space with offset and scale

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#ifndef MO_GUI_UTIL_VIEWSPACE_H
#define MO_GUI_UTIL_VIEWSPACE_H

#include "types/float.h"

namespace MO {
namespace GUI {
namespace UTIL {

template <typename Float>
class ViewSpace
{
public:

    ViewSpace() : x_(0), y_(0), sx_(1), sy_(1) { }

    // ------------ getter --------------

    Float x() const { return x_; }
    Float y() const { return y_; }
    Float scaleX() const { return sx_; }
    Float scaleY() const { return sy_; }

    // ----------- setter ---------------

    void setX(Float x) { x_ = x; }
    void setY(Float y) { y_ = y; }
    void setScaleX(Float sx) { sx_ = sx; }
    void setScaleY(Float sy) { sy_ = sy; }

    // ----------- mapping --------------

    Float mapXTo(Float x) const;
    Float mapYTo(Float y) const;

    Float mapXFrom(Float x) const;
    Float mapYFrom(Float y) const;

    // ----------- control --------------



private:

    Double x_,
           y_,
           sx_,
           sy_;
};

// --------------- implementation -------------------

template <typename Float>
Float ViewSpace<Float>::mapXTo(Float x) const
{
    return x * sx_ + x_;
}

template <typename Float>
Float ViewSpace<Float>::mapYTo(Float y) const
{
    return y * sy_ + y_;
}

template <typename Float>
Float ViewSpace<Float>::mapXFrom(Float x) const
{
    return (x - x_) / sx_;
}

template <typename Float>
Float ViewSpace<Float>::mapYFrom(Float y) const
{
    return (y - y_) / sy_;
}


} // namespace UTIL
} // namespace GUI
} // namespace MO

#endif // MO_GUI_UTIL_VIEWSPACE_H
