/** @file viewspace.h

    @brief view space with offset and scale

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_VIEWSPACE_H
#define MOSRC_GUI_UTIL_VIEWSPACE_H

#include "types/float.h"

namespace MO {
namespace GUI {
namespace UTIL {

class ViewSpace
{
public:

    ViewSpace();

    // -------------- io ----------------

    void serialize(QDataStream& stream);
    void deserialize(QDataStream& stream);

    // ------------ getter --------------

    Double x() const { return x_; }
    Double y() const { return y_; }
    Double scaleX() const { return sx_; }
    Double scaleY() const { return sy_; }

    // ----------- setter ---------------

    void setX(Double x) { x_ = x; }
    void setY(Double y) { y_ = y; }
    void setScaleX(Double sx) { sx_ = sx; }
    void setScaleY(Double sy) { sy_ = sy; }

    void addX(Double x) { x_ += x; }
    void addY(Double y) { y_ += y; }

    // ------ high-level changes --------

    void zoomX(Double scale, Double t = 0.5);
    void zoomY(Double scale, Double t = 0.5);
    void zoom(Double scaleX, Double scaleY, Double tx = 0.5, Double ty = 0.5);

    // ----------- mapping --------------

    Double mapXTo(Double x) const;
    Double mapYTo(Double y) const;

    Double mapXFrom(Double x) const;
    Double mapYFrom(Double y) const;

    Double mapXDistanceTo(Double x) const;
    Double mapXDistanceFrom(Double x) const;

    Double mapYDistanceTo(Double y) const;
    Double mapYDistanceFrom(Double y) const;

private:

    Double x_,
           y_,
           sx_,
           sy_;
};



} // namespace UTIL
} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_VIEWSPACE_H
