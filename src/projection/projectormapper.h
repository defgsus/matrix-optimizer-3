/** @file

    @brief Projection mapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORMAPPER_H
#define MOSRC_PROJECTION_PROJECTORMAPPER_H

#include "projectorsettings.h"
#include "domesettings.h"
#include "types/vector.h"

namespace MO {
namespace GEOM { class Geometry; }
class CameraSettings;

/** Calculator for ProjectorSettings */
class ProjectorMapper
{
public:

    ProjectorMapper();

    // ---------- getter ----------

    const ProjectorSettings& settings() const { return set_; }

    // -------- setter ------------

    void setSettings(const DomeSettings&, const ProjectorSettings&);

    // ----- getter for calculated values -----

    /** Are settings valid?
        If not, the calculated values may not make sense. */
    bool isValid() const { return valid_; }

    /** Aspect ratio of projector */
    Float aspect() const { return aspect_; }

    /** Position of the center of projector lens in 3d space.
        Use getRayOrigin() to find the actual position. */
    const Vec3& pos() const { return pos_; }

    /** Returns the matrix of projector view. */
    const Mat4& getTransformationMatrix() const { return trans_; }

    // --------------- transformation ---------------------

    /** Returns the origin of the ray for the given pixel in texture coordinates [0,1] */
    Vec3 getRayOrigin(Float s, Float t) const;

    /** Returns the ray for the given pixel in texture coordinates [0,1] */
    void getRay(Float s, Float t, Vec3 * ray_origin, Vec3 * ray_direction) const;

    /** Sphere coordinates for the given pixel in texture coordinates [0,1] */
    Vec2 mapToSphere(Float s, Float t) const;

    /** Gives the 3d coordinate on the dome for the given pixel in texture coordinates [0,1].
        The mapping is always done on the inside of the dome, regardless of the position of
        the projector. */
    Vec3 mapToDome(Float s, Float t) const;

    /** Gives the 3d coordinate on the dome for the given pixel in texture coordinates [0,1].
        The mapping is always done on the inside of the dome, regardless of the position of
        the projector. */
    Vec3 mapToDome(const Vec2& st) const { return mapToDome(st[0], st[1]); }

    /** Maps the Geometry to on the dome surface.
        The vertices should by 2d (xy-plane) and in the range [0,1] */
    void mapToDome(GEOM::Geometry *) const;

    /** Returns the coordinate within the slice [0,1] for the given position on the dome.
        If the dome position is not within the projected area, the returned point will
        be outside the range [0,1]. */
    Vec2 mapFromDome(const Vec3& dome_pos) const;

    /** Maps the Geometry supposed to lie on the dome surface to the slice coordiantes. */
    void mapFromDome(GEOM::Geometry *) const;

    // --------------- warping ----------------------------

    /* Find the warp necessary to project the image as seen from the camera. */
    //void getWarpImage(const CameraSettings&);

    /** Find the warp necessary to project the image as seen from the camera.
        The returned Geometry will be a grid of triangles on the z-plane [-1,1]
        with texture coordinates representing the warped slice. */
    void getWarpGeometry(const CameraSettings&, GEOM::Geometry *,
                         int num_segments_x = 32, int num_segments_y = 32) const;

    // -------------- blending -----------------------------

    /** Returns an anti-clockwise polygon outline of the overlapping area between
        this slice and the slice in @p other.
        The coordinates are in the range [0,1].
        If there is no overlap, an empty array is returned.
        @p min_spacing is the minimum distance between points of the polygon.
        @p max_spacing is the maximum distance. Points will be added between two consecutive
        points, if they are more than @p max_spacing apart. This is especially usefull
        for the edges of the projector slice.
        @note Needless to say that @p other should have the same DomeSettings. */
    QVector<Vec2> getOverlapArea(const ProjectorSettings& other, Float min_spacing = 0.05, Float max_spacing = 1.0) const;

    /** Truns the polygon into a triangle Geometry. */
    void getBlendGeometry(const QVector<Vec2>&, GEOM::Geometry *) const;

    //______________ PRIVATE AREA _________________
private:

    void recalc_();

    // ---------- config ----------

    DomeSettings domeSet_;
    ProjectorSettings set_;

    // -------- calculated --------

    bool valid_;
    Float aspect_, zNear_, zFar_;
    Vec3 pos_;
    Mat4 trans_, frustum_, inverseFrustum_, inverseProjView_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTOR_H
