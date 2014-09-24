/** @file

    @brief Projection mapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <QSet>

#include "projectormapper.h"
#include "camerasettings.h"
#include "math/intersection.h"
#include "math/constants.h"
#include "math/vector.h"
#include "io/log.h"
#include "geom/geometry.h"

namespace MO {


ProjectorMapper::ProjectorMapper()
    :   valid_      (false),
        aspect_     (0)
{
    recalc_();
}

void ProjectorMapper::setSettings(const DomeSettings & ds, const ProjectorSettings & s)
{
    domeSet_ = ds;
    set_ = s;
    recalc_();
}


void ProjectorMapper::recalc_()
{
    valid_ = false;

    if (set_.width() <= 0 || set_.height() <= 0)
        return;

    // aspect ratio
    aspect_ = (Float)set_.width()/set_.height();

    // -- calc transformation matrix --

    trans_ = Mat4(1);
    trans_ = MATH::rotate(trans_, set_.latitude(), Vec3(0,1,0));
    trans_ = MATH::rotate(trans_, -set_.longitude(), Vec3(1,0,0));
    trans_ = glm::translate(trans_, Vec3(0,0,domeSet_.radius() + set_.distance()));

    // get actual position
    pos_ = Vec3( trans_ * Vec4(0,0,0,1) );

    // roll-pitch-yaw
    trans_ = MATH::rotate(trans_, set_.roll(),      Vec3(0,0,1));
    trans_ = MATH::rotate(trans_, set_.yaw(),       Vec3(0,1,0));
    trans_ = MATH::rotate(trans_, set_.pitch(),     Vec3(1,0,0));

    // projection matrix of this projector
    frustum_ = MATH::perspective(set_.fov(), (Float)set_.width()/set_.height(),
                                 0.0001f, glm::length(pos_) + domeSet_.radius() + 0.1f);
    // inverse projection/view matrix (for backward mapping)
    inverseProjView_ = frustum_ * glm::inverse(trans_);

    valid_ = true;
}

Vec3 ProjectorMapper::getRayOrigin(Float s, Float t) const
{
    s = (s * 2 - 1) * aspect_;
    t = (t * 2 - 1);

    Vec3 pos = set_.lensRadius() * std::sqrt((Float)2) * Vec3(s, t, 0);
    return Vec3(trans_ * Vec4(pos, (Float)1));
}

void ProjectorMapper::getRay(Float s, Float t, Vec3 *ray_origin, Vec3 *ray_direction) const
{
    s = (s * 2 - 1) * aspect_;
    t = (t * 2 - 1);

    Float lensfac = set_.lensRadius() * std::sqrt((Float)2);
    Vec3 pos = lensfac * Vec3(s, t, 0);


    // size of projection - one unit away
    //    c² = 2a²(1-cos(gamma))
    // -> c = sqrt(2*(1-cos(gamma))
    // and hc = b * sin(alpha)
    // and gamma = 180 - 2*alpha
    // ->  alpha = (180 - gamma) / 2
    const Float gamma = MATH::deg_to_rad(set_.fov());
    const Float c = std::sqrt((Float)2*((Float)1-std::cos(gamma)))
               // XXX lensradius is not calculated properly here
                - (Float)0.135 * lensfac;
    const Float alpha = (Float)0.5 * ((Float)180 - set_.fov());
    const Float hc = std::sin(MATH::deg_to_rad(alpha));
    Vec3 dir = glm::normalize(Vec3(s*c*(Float)0.5, t*c*(Float)0.5, -hc));

    //MO_DEBUG("dir " << dir << ", len " << glm::length(dir));

    *ray_origin =    Vec3(trans_ * Vec4(pos, (Float)1));
    *ray_direction = Vec3(trans_ * Vec4(dir, (Float)0));
}

Vec3 ProjectorMapper::mapToDome(Float s, Float t) const
{
    Vec3 pos, dir;
    getRay(s, t, &pos, &dir);

    Float depth1, depth2;
    if (!MATH::intersect_ray_sphere(pos, dir, Vec3(0,0,0), domeSet_.radius(), &depth1, &depth2))
    {
        // outside dome...
        return pos + dir * (Float)0.01;
    }

    return pos + dir * depth1;
}

Vec2 ProjectorMapper::mapToSphere(Float, Float) const
{
    const Vec3 ray_origin = Vec3( trans_ * Vec4(0,0,0,1) );
    const Vec3 ray_dir = Vec3( trans_ * Vec4(0,0,-1,0) );

    Float depth1, depth2;

    if (!MATH::intersect_ray_sphere(
                ray_origin, ray_dir, Vec3(0,0,0), domeSet_.radius(), &depth1, &depth2))
    {
        return Vec2(0,0);
    }

    return Vec2(0,0);
}

Vec2 ProjectorMapper::mapFromDome(const Vec3 & pdome) const
{
    // project into projector's view space
    Vec4 pscr = inverseProjView_ * Vec4(pdome, 1);
    pscr /= pscr[3];

    // receive [0,1] point
    return Vec2(pscr) * (Float)0.5 + (Float)0.5;
}


void ProjectorMapper::getWarpImage(const CameraSettings & cam)
{
    const Mat4
            projview = cam.getProjectionMatrix() * cam.getViewMatrix();

    {
        // find point on dome of projected image
        const Vec3 pdome = mapToDome(0,0);

        // project into camera space
        Vec4 pscr = projview * Vec4(pdome, 1);
        pscr /= pscr[3];
        pscr = pscr * (Float)0.5 + (Float)0.5;

        MO_DEBUG("pscr = " << pscr);
    }
}

void ProjectorMapper::getWarpGeometry(const CameraSettings & cam, GEOM::Geometry * geo,
                                      int numx, int numy)
{
    numx = std::max(numx, 2);
    numy = std::max(numy, 2);

    geo->clear();
    geo->setSharedVertices(false);
    geo->setColor(1,1,1,1);

    const Mat4
            projview = cam.getProjectionMatrix() * cam.getViewMatrix();

    // create vertex grid with warped tex-coords
    for (int y = 0; y < numy; ++y)
    for (int x = 0; x < numx; ++x)
    {
        const Float
                tx = (Float)x / (numx-1),
                ty = (Float)y / (numy-1);

        // find point on dome of projected image
        const Vec3 pdome = mapToDome(tx, ty);

        // project into camera space
        Vec4 pscr = projview * Vec4(pdome, 1);
        pscr /= pscr[3];
        // receive [0,1] warped point
        pscr = pscr * (Float)0.5 + (Float)0.5;

        geo->setTexCoord(pscr[0], pscr[1]);
        geo->addVertexAlways(tx * 2 - 1, ty * 2 - 1, 0);
    }

    // create triangles
    for (int y = 1; y < numy; ++y)
    for (int x = 1; x < numx; ++x)
    {
        geo->addTriangle((y-1)*numx+x-1, (y-1)*numx+x, y*numx+x);
        geo->addTriangle((y-1)*numx+x-1, y*numx+x, y*numx+x-1);
    }
}


QVector<Vec2> ProjectorMapper::getOverlapArea(const ProjectorSettings &otherP, Float spacing) const
{
    spacing = std::max(spacing, (Float)0.0001);

    // number of points per edge
    const int numPoints = std::max(1, (int)((Float)1 / spacing));

    // list of already set points within given spacing
    QSet<qint64> hash;

    // points to work on
    struct Point
    {
        Vec2 st;
        // corners should always be added
        // regardless of their spacing to other points
        bool add_always;
        Point() { }
        Point(const Vec2& st, bool add_always = false) : st(st), add_always(add_always) { }
    };
    QVector<Point> points;

    // return list
    QVector<Vec2> poly;

    // mapper for other projector
    ProjectorMapper other;
    other.setSettings(domeSet_, otherP);

    // -- get other's edge points to process --

    // bottom edge
    for (int i=0; i<numPoints; ++i)
        // other's slice coordinate [0,1]
        points.append(Point(Vec2((Float)i/(numPoints), 0), i==0));
    // right edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2(1, (Float)i/(numPoints)), i==0));
    // top edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2((Float)1 - (Float)i/(numPoints), 1), i==0));
    // left edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2(0, (Float)1 - (Float)i/(numPoints)), i==0));

    // -- map those points into the current slice --

    for (const Point& p : points)
    {
        // other's projection on dome
        const Vec3 other_dome = other.mapToDome(p.st);
        const Vec2
                // slice coordinate [0,1] of other's projection
                // (which could be outside)
                st = mapFromDome(other_dome);

        const bool outside = st[0] < 0 || st[0] > 1 || st[1] < 0 || st[1] > 1;

        // always add corners that are inside our slice
        if (p.add_always && !outside)
        {
            poly.append(st);
            continue;
        }

        // otherwise clamp to slice area ...
        const Vec2 stc = glm::clamp(st, (Float)0, (Float)1);

        // ... and check spacing
        qint64 h = qint64(stc[0] / spacing) | (qint64(stc[1] / spacing) << 32);

        if (!hash.contains(h))
        {
            poly.append(stc);
            hash.insert(h);
        }
    }

    return poly;
}



} // namespace MO
