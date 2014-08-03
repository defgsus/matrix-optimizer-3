/** @file freecamera.h

    @brief Free floating camera transform

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/3/2014</p>
*/

#ifndef MOSRC_GEOM_FREECAMERA_H
#define MOSRC_GEOM_FREECAMERA_H

#include "types/vector.h"

namespace MO {
namespace GEOM {


class FreeCamera
{
public:
    FreeCamera();

    // -------------- getter -----------------

    const Mat4& getMatrix() const { return matrix_; }

    /** Returns forward vector */
    Vec3 forward() const;

    // -------------- setter -----------------

    void setMatrix(const Mat4& );

    // ------------- navigation --------------

    /** Set absolute position */
    void moveTo(const Vec3&);

    /** Move forward/backward */
    void moveZ(Float steps);
    /** Move right/left */
    void moveX(Float steps);
    /** Move up/down */
    void moveY(Float steps);

    void rotateX(Float degree);
    void rotateY(Float degree);
    void rotateZ(Float degree);
private:

    Mat4 matrix_;

    //Float moveX_, moveY_, moveZ_;
};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_FREECAMERA_H
