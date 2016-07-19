/** @file pointcloud.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#ifndef MOSRC_GEOM_POINTCLOUD_H
#define MOSRC_GEOM_POINTCLOUD_H

namespace MO {
namespace GEOM {

/** Container for pointclouds and triangulation */
class PointCloud
{
    class Private;
    Private * p_;
public:

    PointCloud();
    ~PointCloud();

};

} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_POINTCLOUD_H
