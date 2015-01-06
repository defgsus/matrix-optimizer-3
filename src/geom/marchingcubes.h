/** @file marchingcubes.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.01.2015</p>
*/

#ifndef MOSRC_GEOM_MARCHINGCUBES_H
#define MOSRC_GEOM_MARCHINGCUBES_H

#include <vector>
#include <functional>
#include "types/vector.h"

namespace MO {
namespace GEOM {

class Geometry;

/** Container for 3d grid with Geometry creation */
class MarchingCubes
{
public:

    void renderGrid(Geometry& g,
                         const int8_t * data,
                         int w, int h, int d,
                         const Mat4& trans = Mat4(1),
                         float isolevel = 0.f) const;


    /** Creates the triangles from the marching cubes algorithm in @p g */
    void renderScalarField(Geometry& g,
                         const Vec3& minExtend, const Vec3& maxExtend,
                         const Vec3& numCubes,
                         float isolevel,
                         std::function<float(const Vec3& pos)> func) const;

private:

};

} // namespace GEOM
} // namespace MO

#endif // MARCHINGCUBES_H

