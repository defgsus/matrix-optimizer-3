/** @file viewspace.cpp

    @brief view space with offset and scale

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/

#include "viewspace.h"
#include "io/error.h"
#include "io/datastream.h"

namespace MO {
namespace GUI {
namespace UTIL {

ViewSpace::ViewSpace()
    : x_(0),
      y_(0),
      sx_(1),
      sy_(1),
      minx_(0),
      miny_(0),
      maxx_(0),
      maxy_(0),
      doLimitByChangingScale_(false),
      doMinx_(false),
      doMiny_(false),
      doMaxx_(false),
      doMaxy_(false)
{
}


void ViewSpace::setX(Double x)
{
    x_ = x;

    applyLimits_();
}

void ViewSpace::setY(Double y)
{
    y_ = y;

    applyLimits_();
}

void ViewSpace::setScaleX(Double sx)
{
    sx_ = sx;

    applyLimits_();
}

void ViewSpace::setScaleY(Double sy)
{
    sy_ = sy;

    applyLimits_();
}

void ViewSpace::addX(Double x)
{
    x_ += x;

    applyLimits_();
}

void ViewSpace::addY(Double y)
{
    y_ += y;

    applyLimits_();
}

void ViewSpace::setMinX(Double minx)
{
    minx_ = minx; doMinx_ = true;

    applyLimits_();
}

void ViewSpace::setMaxX(Double maxx)
{
    maxx_ = maxx; doMaxx_ = true;

    applyLimits_();
}

void ViewSpace::setMinY(Double miny)
{
    miny_ = miny; doMiny_ = true;

    applyLimits_();
}

void ViewSpace::setMaxY(Double maxy)
{
    maxy_ = maxy; doMaxy_ = true;

    applyLimits_();
}

void ViewSpace::applyLimits_()
{
    // extend
    if (!doLimitByChangingScale_)
    {
        if (doMaxx_) x_ = std::min(x_, maxx_ - sx_);
        if (doMaxy_) y_ = std::min(y_, maxy_ - sy_);
    }

    // top/left
    if (doMinx_) x_ = std::max(x_, minx_);
    if (doMiny_) y_ = std::max(y_, miny_);
    // bottom/right
    if (doMaxx_) x_ = std::min(x_, maxx_);
    if (doMaxy_) y_ = std::min(y_, maxy_);

    // extend
    if (doLimitByChangingScale_)
    {
        if (doMaxx_ && x_ + sx_ > maxx_)
            sx_ = std::max((Double)0.0001, maxx_ - x_);

        if (doMaxy_ && y_ + sy_ > maxy_)
            sy_ = std::max((Double)0.0001, maxy_ - y_);
    }
}




Double ViewSpace::mapXTo(Double x) const
{
    return x * sx_ + x_;
}

Double ViewSpace::mapYTo(Double y) const
{
    return y * sy_ + y_;
}

Double ViewSpace::mapXFrom(Double x) const
{
    return (x - x_) / sx_;
}

Double ViewSpace::mapYFrom(Double y) const
{
    return (y - y_) / sy_;
}

Double ViewSpace::mapXDistanceTo(Double x) const
{
    return x * sx_;
}

Double ViewSpace::mapXDistanceFrom(Double x) const
{
    return x / sx_;
}

Double ViewSpace::mapYDistanceTo(Double y) const
{
    return y * sy_;
}

Double ViewSpace::mapYDistanceFrom(Double y) const
{
    return y / sy_;
}


void ViewSpace::zoomX(Double scale, Double t)
{
    const Double oldsx = sx_;
    sx_ *= scale;
    x_ += (oldsx - sx_) * t;

    applyLimits_();
}

void ViewSpace::zoomY(Double scale, Double t)
{
    const Double oldsy = sy_;
    sy_ *= scale;
    y_ += (oldsy - sy_) * t;

    applyLimits_();
}

void ViewSpace::zoom(Double scaleX, Double scaleY, Double tx, Double ty)
{
    zoomX(scaleX, tx);
    zoomY(scaleY, ty);
}


void ViewSpace::serialize(IO::DataStream &stream)
{
    stream.writeHeader("viewspace", 1);

    stream << x_ << y_ << sx_ << sy_
           << minx_ << miny_ << maxx_ << maxy_
           << doMinx_ << doMiny_ << doMaxx_ << doMaxy_
           << doLimitByChangingScale_;
}

void ViewSpace::deserialize(IO::DataStream &stream)
{
    stream.readHeader("viewspace", 1);

    stream >> x_ >> y_ >> sx_ >> sy_
            >> minx_ >> miny_ >> maxx_ >> maxy_
            >> doMinx_ >> doMiny_ >> doMaxx_ >> doMaxy_
            >> doLimitByChangingScale_;
}


} // namespace UTIL
} // namespace GUI
} // namespace MO

