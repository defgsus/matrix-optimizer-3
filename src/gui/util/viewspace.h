/** @file viewspace.h

    @brief view space with offset and scale

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_VIEWSPACE_H
#define MOSRC_GUI_UTIL_VIEWSPACE_H

#include "types/float.h"

namespace MO {
class Sequence;
namespace IO { class DataStream; }
namespace GUI {
namespace UTIL {

class ViewSpace
{
public:

    ViewSpace();
    ViewSpace(Double scaleX, Double scaleY);
    ViewSpace(Double x, Double y, Double scaleX, Double scaleY);

    // -------------- io ----------------

    void serialize(IO::DataStream& stream) const;
    void deserialize(IO::DataStream& stream);

    // ------------ getter --------------

    Double x() const { return x_; }
    Double y() const { return y_; }
    Double scaleX() const { return sx_; }
    Double scaleY() const { return sy_; }

    Double minX() const { return minx_; }
    Double maxX() const { return maxx_; }
    Double minY() const { return miny_; }
    Double maxY() const { return maxy_; }

    // ----------- setter ---------------

    void setX(Double x);
    void setY(Double y);
    void setScaleX(Double sx);
    void setScaleY(Double sy);

    void addX(Double x);
    void addY(Double y);

    void setMinX(Double minx);
    void setMaxX(Double maxx);
    void setMinY(Double miny);
    void setMaxY(Double maxy);

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

    /** Adjusts the current ViewSpace to match the sequence settings */
    void mapToSequence(const Sequence *);

    void mapFromSequence(const Sequence *);

private:

    void applyLimits_();

    Double
        x_,
        y_,
        sx_,
        sy_,
        minx_,
        miny_,
        maxx_,
        maxy_;

    bool
        doLimitByChangingScale_,
        doMinx_,
        doMiny_,
        doMaxx_,
        doMaxy_;
};


} // namespace UTIL
} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_VIEWSPACE_H
