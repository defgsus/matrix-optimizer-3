/** @file

    @brief Projection mapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORMAPPER_H
#define MOSRC_PROJECTION_PROJECTORMAPPER_H

#include "projectorsettings.h"
#include "types/vector.h"

namespace MO {

class DomeSettings;

/** Calculator for ProjectorSettings */
class ProjectorMapper
{
public:

    ProjectorMapper();

    // ---------- getter ----------

    const ProjectorSettings& settings() const { return set_; }

    // -------- setter ------------

    void setSettings(const ProjectorSettings&);

    // ----- getter for calculated values -----

    /** Are settings valid?
        If not, the calculated values may not make sense. */
    bool isValid() const { return valid_; }

    /** Aspect ratio of projector */
    Float aspect() const { return aspect_; }

    /** Position of projector lens in 3d space. */
    const Vec3& pos() const { return pos_; }

    /** Returns the matrix of projector view. */
    const Mat4& getTransformationMatrix() const { return trans_; }

    /** Returns the ray for the given pixel in texture coordinates [0,1] */
    void getRay(Float s, Float t, Vec3 * ray_origin, Vec3 * ray_direction) const;

    /** Gives the 3d coordinate for the given pixel in texture coordinates [0,1] */
    Vec3 mapToDome(Float s, Float t, const DomeSettings&) const;

    /** Sphere coordinates for the given pixel in texture coordinates [0,1] */
    Vec2 mapToSphere(Float s, Float t) const;

    //______________ PRIVATE AREA _________________
private:

    void recalc_();

    // ---------- config ----------

    ProjectorSettings set_;

    // -------- calculated --------

    bool valid_;
    Float aspect_;
    Vec3 pos_;
    Mat4 trans_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTOR_H
