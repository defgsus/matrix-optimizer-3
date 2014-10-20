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
#include "math/interpol.h"
#include "math/polygon.h"
#include "io/log.h"
#include "geom/geometry.h"
#include "geom/tesselator.h"
#include "gl/screenquad.h"

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

    // ---- calc projection of projector ----

#ifndef MO_DISABLE_PROJECTOR_LENS_RADIUS
    // near-plane is the surface of the projector lens
    // we choose it so the lensradius will fit
    zNear_ = set_.lensRadius() * aspect_ / std::tan(MATH::deg_to_rad(set_.fov()) / 2);
#else
    zNear_ = 0.01;
#endif

    // make far-plane always farther than distance to other side of dome
    zFar_ = zNear_ + set_.distance() + domeSet_.radius() * 2.f + 0.1f;

    // projector's projection matrix
    frustum_ = MATH::perspective(set_.fov(), (Float)set_.width()/set_.height(), zNear_, zFar_);
    // inverse frustum for ray direction
    inverseFrustum_ = glm::inverse(frustum_);

    // get the shift to make zNear plane the actual lens surface
    // XXX this is a hack!
    Vec4 tmp = inverseFrustum_ * Vec4(0,0,-zNear_,1);
    Float zNearShift = tmp.z / tmp.w;

    // -- calc transformation matrix --

    trans_ = Mat4(1);
    trans_ = MATH::rotate(trans_, set_.latitude(), Vec3(0,1,0));
    trans_ = MATH::rotate(trans_, -set_.longitude(), Vec3(1,0,0));
    trans_ = glm::translate(trans_, Vec3(0,0,domeSet_.radius() + set_.distance()
                                         - zNearShift));
    // get actual position (lens centre)
    pos_ = Vec3( trans_ * Vec4(0,0,0,1) );

    // roll-pitch-yaw
    trans_ = MATH::rotate(trans_, set_.roll(),      Vec3(0,0,1));
    trans_ = MATH::rotate(trans_, set_.yaw(),       Vec3(0,1,0));
    trans_ = MATH::rotate(trans_, set_.pitch(),     Vec3(1,0,0));

    // inverse projection/view matrix for backward mapping
    inverseProjView_ = frustum_ * glm::inverse(trans_);

    valid_ = true;
}

Vec3 ProjectorMapper::getRayOrigin(Float s, Float t) const
{
#if (1)
    s = (s * 2 - 1);
    t = (t * 2 - 1);

    Vec4 pos = inverseFrustum_ * Vec4(s, t, -zNear_, 1);
    pos /= pos.w;

    return Vec3(trans_ * pos);
#else
    Vec3 org, dir;
    getRay(s, t, &org, &dir);
    return org;
#endif
}

void ProjectorMapper::getRay(Float s, Float t, Vec3 *ray_origin, Vec3 *ray_direction) const
{
#if (0)

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

#else

    s = (s * 2 - 1);
    t = (t * 2 - 1);

    Vec4 pos = inverseFrustum_ * Vec4(s, t, -zNear_, 1);
    pos /= pos.w;

    Vec4 dirf = inverseFrustum_ * Vec4(s, t, -zFar_, 1);
    dirf /= dirf.w;

    Vec3 dir = glm::normalize(Vec3(dirf));

    *ray_origin =    Vec3(trans_ * pos);
    *ray_direction = Vec3(trans_ * Vec4(dir, (Float)0));

#endif

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

void ProjectorMapper::mapToDome(GEOM::Geometry * g) const
{
    for (uint i=0; i<g->numVertices(); ++i)
    {
        auto v = &g->vertices()[i*g->numVertexComponents()];
        Vec3 d = mapToDome(v[0], v[1]);
        v[0] = d[0];
        v[1] = d[1];
        v[2] = d[2];
    }
}

void ProjectorMapper::mapToDome(const QVector<Vec2> &slice_coords, QVector<Vec3> &dome_coords) const
{
    for (auto & c : slice_coords)
        dome_coords.append(mapToDome(c));
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

void ProjectorMapper::mapFromDome(GEOM::Geometry * g) const
{
    for (uint i=0; i<g->numVertices(); ++i)
    {
        Vec2 slice = mapFromDome(g->getVertex(i));
        auto v = &g->vertices()[i*g->numVertexComponents()];
        v[0] = slice[0];
        v[1] = slice[1];
        v[2] = 0.f;
    }
}

void ProjectorMapper::mapFromDome(const QVector<Vec3> &dome_coords, QVector<Vec2> &slice_coords) const
{
    for (auto & c : dome_coords)
        slice_coords.append(mapFromDome(c));
}

/*
XXX this is difficult!
    Currently we let the user handle this herself
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
*/

void ProjectorMapper::getWarpGeometry(const CameraSettings & cam, GEOM::Geometry * geo,
                                      int numx, int numy) const
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

void ProjectorMapper::getWarpDrawable(const CameraSettings & camera, GL::ScreenQuad * quad,
                                      int num_segments_x, int num_segments_y) const
{
    GEOM::Geometry * warp = new GEOM::Geometry();

    getWarpGeometry(camera, warp, num_segments_x, num_segments_y);

    // screen-quad
    quad->create(
                ":/shader/framebufferdraw.vert",
                ":/shader/framebufferdraw.frag",
                "",
                warp);
}


QVector<Vec2> ProjectorMapper::createOutline(Float space, Float sm) const
{
    QVector<Vec2> p;

    for (Float t = 0; t < 1; t += space)
        p.append(Vec2(t, 0));

    for (Float t = 0; t < 1; t += space)
        p.append(Vec2(1, t));

    for (Float t = 0; t < 1; t += space)
        p.append(Vec2(1.f-t, 1.f));

    for (Float t = 0; t < 1; t += space)
        p.append(Vec2(0, 1.f-t));

    if (sm != 0)
    {
        const Vec2
                ofs = Vec2(sm, sm * aspect_) * 0.5f,
                mul = 1.f - 2.f * ofs;
        for (auto & v : p)
        {
            v = ofs + mul * v;
        }
    }

    return p;
}


QVector<Vec2> ProjectorMapper::getOverlapArea(const ProjectorSettings &otherP, Float min_spacing, Float max_spacing) const
{
    min_spacing = std::max(min_spacing, (Float)0.0001);
    max_spacing = std::max(max_spacing, (Float)0.0001);

    // number of points per edge
    const int numPoints = std::max(1, (int)((Float)1 / min_spacing));

    // list of already set points within given spacing
    QSet<qint64> hash;

    // points to work on
    struct Point
    {
        Vec2 p;
        // corners should always be added
        // regardless of their spacing to other points
        bool is_corner,
             is_inside;
        Point() { }
        Point(const Vec2& point, const ProjectorMapper& me, const ProjectorMapper& other, bool corner = false)
            : is_corner(corner)
        {
            // project other's projected point into our slice
            p = me.mapFromDome(other.mapToDome(point));
            // remember if inside
            is_inside = MATH::inside_range(p, 0, 1);
        }
    };
    QVector<Point> points;

    // mapper for other projector
    ProjectorMapper other;
    other.setSettings(domeSet_, otherP);

    // find if corners of this slice map into other slice
    const bool
            c_bl = MATH::inside_range(other.mapFromDome(mapToDome(Vec2(0,0))), 0, 1),
            c_br = MATH::inside_range(other.mapFromDome(mapToDome(Vec2(1,0))), 0, 1),
            c_tl = MATH::inside_range(other.mapFromDome(mapToDome(Vec2(0,1))), 0, 1),
            c_tr = MATH::inside_range(other.mapFromDome(mapToDome(Vec2(1,1))), 0, 1);

    // -- get other's edge points projected into our slice --
    //    (and flag corners)

    // bottom edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2((Float)i/(numPoints), 0), *this, other, i==0));
    // right edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2(1, (Float)i/(numPoints)), *this, other, i==0));
    // top edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2((Float)1 - (Float)i/(numPoints), 1), *this, other, i==0));
    // left edge
    for (int i=0; i<numPoints; ++i)
        points.append(Point(Vec2(0, (Float)1 - (Float)i/(numPoints)), *this, other, i==0));

    // -- create intersecion with our slice --

    QVector<Vec2> tmp_poly;

    for (int i=0; i<points.count(); ++i)
    {
        const Point& p = points.at(i);

        // always add corners that are inside our slice
        if (p.is_corner && p.is_inside)
        {
            tmp_poly.append(p.p);
            continue;
        }

        // if outside?
        if (!p.is_inside)
        {
            // get previous/next point
            const Point&
                    prev = points.at((i+points.count()-1)%points.count()),
                    next = points.at((i+1)%points.count());

            // see if prev or next are inside
            Point p1, p2;
            if (prev.is_inside)
            {
                p1 = prev;
                p2 = p;
            }
            else if (next.is_inside)
            {
                p1 = p;
                p2 = next;
            }
            // neither inside
            else
            // check if candidate for a slice corner
            if ((p.p[0] < 0 || p.p[0] > 1) && (p.p[1] < 0 || p.p[1] > 1))
            {
                if (!(   (p.p[0] < 0 && p.p[1] < 0 && c_bl)
                      || (p.p[0] > 1 && p.p[1] < 0 && c_br)
                      || (p.p[0] < 0 && p.p[1] > 1 && c_tl)
                      || (p.p[0] > 1 && p.p[1] > 1 && c_tr)))
                    continue;

                // add unique corner of our slice
                Vec2 c = glm::clamp(p.p, (Float)0, (Float)1);
                qint64 h = qint64(c[0] / min_spacing) | (qint64(c[1] / min_spacing) << 32);

                if (!hash.contains(h))
                {
                    tmp_poly.append(c);
                    hash.insert(h);
                }

                continue;
            }

            // find intersection with slice edge
            Float t;
            if (MATH::intersect_line_line(Vec2(0,0), Vec2(0,1), p1.p, p2.p, &t))
                tmp_poly.append(Vec2(0,t));
            else if (MATH::intersect_line_line(Vec2(0,0), Vec2(1,0), p1.p, p2.p, &t))
                tmp_poly.append(Vec2(t,0));
            else if (MATH::intersect_line_line(Vec2(1,0), Vec2(1,1), p1.p, p2.p, &t))
                tmp_poly.append(Vec2(1,t));
            else if (MATH::intersect_line_line(Vec2(0,1), Vec2(1,1), p1.p, p2.p, &t))
                tmp_poly.append(Vec2(t,1));

            continue;
        }

        // if inside, simply add, but check for spacing
        qint64 h = qint64(p.p[0] / min_spacing) | (qint64(p.p[1] / min_spacing) << 32);

        if (!hash.contains(h))
        {
            tmp_poly.append(p.p);
            hash.insert(h);
        }
    }

    if (tmp_poly.isEmpty())
        return tmp_poly;

    // -- finally create points in-between when spacing is to large --
    //    (like on our slice's edges)

    QVector<Vec2> poly;

    for (int i=1; i<tmp_poly.count(); ++i)
    {
        const Vec2 &
                p1 = tmp_poly.at(i-1),
                p2 = tmp_poly.at(i);
        const Float d = glm::distance(p1, p2);
        if (d > max_spacing*1.1)
        {
            Float t = 0;
            while (t < d)
            {
                poly.append(p1 + t * (p2 - p1));
                t += max_spacing;
            }
        }
        else poly.append(p1);
    }
    poly.append(tmp_poly.back());

    return poly;
}


void ProjectorMapper::getBlendGeometry(const QVector<Vec2> & poly, GEOM::Geometry * geom) const
{
    GEOM::Tesselator tess;

    tess.tesselate(poly);
    tess.getGeometry(*geom);

    // set blend colors for each vertex
    for (uint i=0; i<geom->numVertices(); ++i)
    {
        Vec3 v = geom->getVertex(i);

        // distance to edge
        Float dist = std::min(
                            std::min(v[0], std::abs(1.f-v[0])),
                            std::min(v[1], std::abs(1.f-v[1])));

        Float col = MATH::smoothstep(0.0f, 0.1f, dist);

//        MO_DEBUG("vec " << v << " dist " << dist << " col " << col);

        geom->colors()[i*4] = col;
        geom->colors()[i*4+1] = col;
        geom->colors()[i*4+2] = col;
        geom->colors()[i*4+3] = 1.f;
    }
}


bool ProjectorMapper::getBlendGeometry(const ProjectorMapper &other, GEOM::Geometry *g)
{
    const Float
            minspace = 0.05,
            blendedge = 0.3,
            outrange = 1.55;

    // map other's slice projection into our screen space
    QVector<Vec3> domec;
    QVector<Vec2> pout, pin;
    other.mapToDome( other.createOutline(minspace), domec);
    mapFromDome(domec, pout);
    //MATH::limit(pout, Vec2(-4,-4), Vec2(4,4));

    // now the inner edge
    domec.clear();
    other.mapToDome( other.createOutline(minspace, blendedge), domec);
    mapFromDome(domec, pin);

    MO_ASSERT(pin.size() == pout.size(), "rounding error in createOutline?");

    // connect blend area by triangles
    for (int i=0; i<pout.size(); ++i)
    {
        const int i1 = (i+1) % pout.size();

        // only create the triangles inside
        if (!(MATH::inside_range(pout[i], -outrange, outrange)
              && MATH::inside_range(pout[i1], -outrange, outrange)
              && MATH::inside_range(pin[i], -outrange, outrange)
              && MATH::inside_range(pin[i1], -outrange, outrange)))
            continue;

        g->setColor(0,0,0,0);
        auto v1 = g->addVertex(pout[i][0], pout[i][1], 0);
        auto v2 = g->addVertex(pout[i1][0], pout[i1][1], 0);
        g->setColor(0,0,0,1);
        auto v3 = g->addVertex(pin[i][0], pin[i][1], 0);
        auto v4 = g->addVertex(pin[i1][0], pin[i1][1], 0);

        g->addTriangle(v1, v2, v3);
        g->addTriangle(v2, v4, v3);
    }

    // -- get complete blackout area --

    MATH::limit(pin, Vec2(-outrange,-outrange), Vec2(outrange, outrange));
    QVector<Vec2> black = MATH::get_intersection_poly_rect(pin, Vec2(0,0), Vec2(1,1));
    GEOM::Tesselator tess;
    tess.tesselate(black);
    g->setColor(0,0,0,1);
    tess.getGeometry(*g);

    return true;
}



bool ProjectorMapper::getIntersectionGeometry(const ProjectorMapper &other, GEOM::Geometry *g)
{
    const Float
            minspace = 0.05,
            outrange = 1.55,
            edge = 0.5;

    // map other's slice projection into our screen space
    QVector<Vec3> odome;
    QVector<Vec2> osliceorg, oslice;
    osliceorg = other.createOutline(minspace);
    other.mapToDome(osliceorg, odome);
    mapFromDome(odome, oslice);
    // clip to some extent (some back-projections are terribly wrong outside our frustum)
    MATH::limit(oslice, Vec2(-outrange,-outrange), Vec2(outrange, outrange));

    // intersect with own rectangle
    QVector<Vec2> overlap
            = MATH::get_intersection_poly_rect(oslice, Vec2(0,0), Vec2(1,1));

    if (overlap.isEmpty())
        return false;

    // triangulate
    GEOM::Tesselator tess;
    tess.tesselate(overlap, true);

    // create geometry
    GEOM::Geometry geom;
    tess.getGeometry(geom);

    // make finer resolution
    geom.tesselate(6);

#define MO__EDGE_DIST(vec__)                                    \
    std::min(std::min(MATH::smoothstep(0.f,edge,vec__[0]),      \
                      MATH::smoothstep(1.f,1.f-edge,vec__[0])), \
             std::min(MATH::smoothstep(0.f,edge,vec__[1]),      \
                      MATH::smoothstep(1.f,1.f-edge,vec__[1])))

    // set color
    for (uint i=0; i<geom.numVertices(); ++i)
    {
        const Vec2 pos = Vec2(geom.vertices()[i * geom.numVertexComponents()],
                              geom.vertices()[i * geom.numVertexComponents()+1]),
                   opos = other.mapFromDome(mapToDome(pos));
        auto color = &geom.colors()[i * geom.numColorComponents()];

        const Float edged = MO__EDGE_DIST(pos),
                    oedged = MO__EDGE_DIST(opos),

                    centd = std::min(1.f, 2.f * glm::length(pos-Vec2(.5f,.5f))),
                    ocentd = std::min(1.f, 2.f * glm::length(opos-Vec2(.5f,.5f)));

        /*const Float white = std::max(0.f,
                    1.f - oedged
                    - (1.f - edged));*/
        // Version Nr. 1:
        const Float white = 0.65 * (float(edged > 0.05) * float(oedged > 0.05))
                          + 1.0 * (std::min(1.f, 50.f * edged * float(edged >  0.05)) * float(oedged <= 0.05))
                          + 1.0 * (std::min(1.f, 0.65f * float(edged <= 0.05) * float(oedged <= 0.05)));
                            //std::min(1.f-oedged, edged);
        //const Float white = std::max(0.f, ocentd -
        //                    ((1-ocentd) * (1-centd)));

        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        color[3] = glm::clamp(1.f - white, 0.f, 1.f);
    }

#undef MO__EDGE_DIST

    g->addGeometry(geom);

    return true;
}



} // namespace MO
