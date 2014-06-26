/** @file viewspace.cpp

    @brief view space with offset and scale

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#include <QDataStream>

#include "viewspace.h"
#include "io/error.h"

namespace MO {
namespace GUI {
namespace UTIL {

ViewSpace::ViewSpace() : x_(0), y_(0), sx_(1), sy_(1) { }


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
}

void ViewSpace::zoomY(Double scale, Double t)
{
    const Double oldsy = sy_;
    sy_ *= scale;
    y_ += (oldsy - sy_) * t;
}

void ViewSpace::zoom(Double scaleX, Double scaleY, Double tx, Double ty)
{
    zoomX(scaleX, tx);
    zoomY(scaleY, ty);
}


void ViewSpace::serialize(QDataStream &stream)
{
    // version
    stream << QString("viewspace");
    stream << (qint32)1;

    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream << x_ << y_ << sx_ << sy_;
}

void ViewSpace::deserialize(QDataStream &stream)
{
    // check version
    QString head;
    stream >> head;
    if (head != "timeline")
        MO_IO_ERROR(VERSION_MISMATCH, "expected 'viewspace' header, but got '" << head << "'");

    qint32 ver;
    stream >> ver;
    if (ver > 1)
        MO_IO_ERROR(VERSION_MISMATCH, "unknown viewspace version " << ver);

    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream >> x_ >> y_ >> sx_ >> sy_;
}


} // namespace UTIL
} // namespace GUI
} // namespace MO

