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
class ViewSpaceT
{
public:

    ViewSpaceT() : x_(0), y_(0), sx_(1), sy_(1) { }

    // ------------ assignment ----------

    template <typename OtherFloat>
    ViewSpaceT<Float>& operator = (const ViewSpaceT<OtherFloat>& o)
    {
        x_ = o.x; y_ = o.y; sx_ = o.sx_; sy_ = o.sy_;
        return *this;
    }

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

    void addX(Float x) { x_ += x; }
    void addY(Float y) { y_ += y; }

    // ------ high-level changes --------

    void zoomX(Float scale, Float t = 0.5);
    void zoomY(Float scale, Float t = 0.5);
    void zoom(Float scaleX, Float scaleY, Float tx = 0.5, Float ty = 0.5);

    // ----------- mapping --------------

    Float mapXTo(Float x) const;
    Float mapYTo(Float y) const;

    Float mapXFrom(Float x) const;
    Float mapYFrom(Float y) const;

    Float mapXDistanceTo(Float x) const;
    Float mapXDistanceFrom(Float x) const;

    Float mapYDistanceTo(Float y) const;
    Float mapYDistanceFrom(Float y) const;

private:

    Double x_,
           y_,
           sx_,
           sy_;
};

// --------------- implementation -------------------

template <typename Float>
inline Float ViewSpaceT<Float>::mapXTo(Float x) const
{
    return x * sx_ + x_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapYTo(Float y) const
{
    return y * sy_ + y_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapXFrom(Float x) const
{
    return (x - x_) / sx_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapYFrom(Float y) const
{
    return (y - y_) / sy_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapXDistanceTo(Float x) const
{
    return x * sx_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapXDistanceFrom(Float x) const
{
    return x / sx_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapYDistanceTo(Float y) const
{
    return y * sy_;
}

template <typename Float>
inline Float ViewSpaceT<Float>::mapYDistanceFrom(Float y) const
{
    return y / sy_;
}


template <typename Float>
inline void ViewSpaceT<Float>::zoomX(Float scale, Float t)
{
    const Float oldsx = sx_;
    sx_ *= scale;
    x_ += (oldsx - sx_) * t;
}

template <typename Float>
inline void ViewSpaceT<Float>::zoomY(Float scale, Float t)
{
    const Float oldsy = sy_;
    sy_ *= scale;
    y_ += (oldsy - sy_) * t;
}

template <typename Float>
inline void ViewSpaceT<Float>::zoom(Float scaleX, Float scaleY, Float tx, Float ty)
{
    zoomX(scaleX, tx);
    zoomY(scaleY, ty);
}

// ------------ default type ----------------

typedef ViewSpaceT<Double> ViewSpace;


} // namespace UTIL
} // namespace GUI
} // namespace MO

#endif // MO_GUI_UTIL_VIEWSPACE_H
