/** @file geometryfactory.cpp

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#include "geometryfactory.h"
#include "geometry.h"

namespace MO {
namespace GL {


void GeometryFactory::createCube(Geometry * g, float sl)
{
    return createBox(g, sl, sl, sl);
}

void GeometryFactory::createBox(
        Geometry * g, float side_length_x, float side_length_y, float side_length_z)
{
    float
        sx = side_length_x * 0.5f,
        sy = side_length_y * 0.5f,
        sz = side_length_z * 0.5f;

    // --- vertices / corner-points ---

    // front-bottom-left
    g->setTexCoord(0,0);
    int fbl = g->addVertex(-sx, -sy,  sz);
    // front-bottom-right
    g->setTexCoord(1,0);
    int fbr = g->addVertex( sx, -sy,  sz);
    // front-top-right
    g->setTexCoord(1,1);
    int ftr = g->addVertex( sx,  sy,  sz);
    // front-top-left
    g->setTexCoord(0,1);
    int ftl = g->addVertex(-sx,  sy,  sz);

    // back-bottom-left
    g->setTexCoord(0,1);
    int bbl = g->addVertex(-sx, -sy, -sz);
    // aso..
    g->setTexCoord(1,1);
    int bbr = g->addVertex( sx, -sy, -sz);
    g->setTexCoord(1,0);
    int btr = g->addVertex( sx,  sy, -sz);
    g->setTexCoord(0,0);
    int btl = g->addVertex(-sx,  sy, -sz);

    // --- surfaces / triangles ---

    // front
    g->addTriangle(ftr, ftl, fbl);
    g->addTriangle(ftr, fbl, fbr);
    // right
    g->addTriangle(ftr, fbr, btr);
    g->addTriangle(fbr, bbr, btr);
    // back
    g->addTriangle(btr, bbr, btl);
    g->addTriangle(btl, bbr, bbl);
    // left
    g->addTriangle(ftl, btl, bbl);
    g->addTriangle(ftl, bbl, fbl);
    // top
    g->addTriangle(ftr, btr, btl);
    g->addTriangle(ftr, btl, ftl);
    // bottom
    g->addTriangle(fbr, fbl, bbl);
    g->addTriangle(fbr, bbl, bbr);

}


} // namespace GL
} // namespace MO
