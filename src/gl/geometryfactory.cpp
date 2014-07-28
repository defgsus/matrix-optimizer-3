/** @file geometryfactory.cpp

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#include <QObject>

#include "geometryfactory.h"
#include "geometry.h"
#include "math/vector.h"

namespace MO {
namespace GL {


void GeometryFactory::createCube(Geometry * g, float sl, bool asTriangles)
{
    return createBox(g, sl, sl, sl, asTriangles);
}

void GeometryFactory::createQuad(Geometry * g, float sx, float sy, bool asTriangles)
{
    sx *= 0.5f;
    sy *= 0.5f;

    g->setTexCoord(0,0);
    const int bl = g->addVertex(-sx, -sy, 0);
    g->setTexCoord(1,0);
    const int br = g->addVertex( sx, -sy, 0);
    g->setTexCoord(0,1);
    const int tl = g->addVertex(-sx,  sy, 0);
    g->setTexCoord(1,1);
    const int tr = g->addVertex( sx,  sy, 0);

    if (asTriangles)
    {
        g->addTriangle(tr, tl, bl);
        g->addTriangle(tr, bl, br);
    }
    else
    {
        g->addLine(bl, br);
        g->addLine(bl, tl);
        g->addLine(br, tr);
        g->addLine(tl, tr);
    }
}

void GeometryFactory::createBox(
        Geometry * g, float side_length_x, float side_length_y, float side_length_z,
        bool asTriangles)
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

    if (asTriangles)
    {
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

    else
    {
        // front
        g->addLine(fbl, fbr);
        g->addLine(fbl, ftl);
        g->addLine(fbr, ftr);
        g->addLine(ftl, ftr);
        // back
        g->addLine(bbl, bbr);
        g->addLine(bbl, btl);
        g->addLine(bbr, btr);
        g->addLine(btl, btr);
        // left
        g->addLine(fbl, bbl);
        g->addLine(ftl, btl);
        // right
        g->addLine(fbr, bbr);
        g->addLine(ftr, btr);
    }
}


void GeometryFactory::createGrid(Geometry * g, int sizeU, int sizeV, bool coords)
{
    for (int i=-sizeU; i<=sizeU; ++i)
    {
        auto p1 = g->addVertex(i,0,-sizeV),
             p2 = g->addVertex(i,0,sizeV);
        g->addLine(p1, p2);
    }
    for (int i=-sizeV; i<=sizeV; ++i)
    {
        auto p1 = g->addVertex(-sizeU,0,i),
             p2 = g->addVertex(sizeU,0,i);
        g->addLine(p1, p2);
    }

    if (!coords)
        return;

    g->setColor(1,0,0, 1);
    auto p1 = g->addVertex(0, 0.01, 0),
         p2 = g->addVertex(sizeU, 0.01, 0);
    g->addLine(p1, p2);

    g->setColor(0,1,0, 1);
    p1 = g->addVertex(0, 0.01, 0);
    p2 = g->addVertex(0, std::min(sizeU, sizeV), 0);
    g->addLine(p1, p2);

    g->setColor(0,0,1, 1);
    p1 = g->addVertex(0, 0.01, 0);
    p2 = g->addVertex(0, 0, sizeV);
    g->addLine(p1, p2);
}

void GeometryFactory::createUVSphere(
        Geometry * g, float rad, uint segu, uint segv, bool asTriangles)
{
    if (!asTriangles)
    {
        createUVSphereLines(g, rad, segu, segv);
        return;
    }

    // top point
    g->setTexCoord(0,1);
    g->addVertex(0, rad, 0);

    for (uint v = 1; v<segv; ++v)
    {
        // current vertex offset
        int rown = g->numVertices();

        // body vertices
        for (unsigned int u = 0; u<segu; ++u)
        {
            Vec3 p = MATH::pointOnSphere((float)u / segu, (float)v / segv);

            g->setTexCoord((float)u / segu, (float)v / segv);
            g->addVertex(p.x * rad, p.y * rad, p.z * rad);
        }

        // triangles on each 'row'
        for (unsigned int u = 0; u<segu; ++u)
        {
            // connect to top point
            if (v==1)
            {
                g->addTriangle(0, rown + u, rown + (u + 1) % segu);
            }
            else
            {
                // connect
                g->addTriangle(
                // previous row
                    rown - segu + u,
                // with this row
                    rown + u, rown + (u + 1) % segu);
                // .. and the other triangle of the quad
                g->addTriangle(rown - segu + (u + 1) % segu,
                               rown - segu + u,
                               rown + (u + 1) % segu);
            }
        }
    }

    int rown = g->numVertices() - segu;

    // bottom point
    g->setTexCoord(0,0);
    g->addVertex(0, -rad, 0);

    // connect to bottom point
    for (unsigned int u = 0; u<segu; ++u)
    {
        g->addTriangle(g->numVertices()-1,
                       rown + (u + 1) % segu, rown + u);
    }
}

void GeometryFactory::createUVSphereLines(
        Geometry * g, float rad, uint segu, uint segv)
{
    // top point
    g->setTexCoord(0,1);
    g->addVertex(0, rad, 0);

    for (uint v = 1; v<segv; ++v)
    {
        // current vertex offset
        int rown = g->numVertices();

        // body vertices
        for (unsigned int u = 0; u<segu; ++u)
        {
            Vec3 p = MATH::pointOnSphere((float)u / segu, (float)v / segv);
            g->addVertex(p.x * rad, p.y * rad, p.z * rad);
        }

        // lines on each 'row'
        for (unsigned int u = 0; u<segu; ++u)
        {
            if (v==1)
            {
                // connect to top point
                g->addLine(0, rown + u);
            }
            else
            {
                // connect previous row with this
                g->addLine(rown + u - segu, rown + u);
            }
            // connect with next point in row
            g->addLine(rown + u, rown + (u + 1) % segu);
        }
    }

    int rown = g->numVertices() - segu;

    // bottom point
    g->setTexCoord(0,0);
    g->addVertex(0, -rad, 0);

    // connect to bottom point
    for (unsigned int u = 0; u<segu; ++u)
    {
        g->addLine(g->numVertices() - 1, rown + u);
    }
}



void GeometryFactory::createFromSettings(Geometry * g, const GeometryFactorySettings * set)
{
    g->setColor(set->colorR, set->colorG, set->colorB, set->colorA);

    switch (set->type)
    {
    case GeometryFactorySettings::T_QUAD:
        createQuad(g, 1.f, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_BOX:
        createCube(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_GRID:
        createGrid(g, set->segmentsU, set->segmentsV, set->withCoords);
    break;

    case GeometryFactorySettings::T_UV_SPHERE:
        createUVSphere(g, 1.f, std::max((uint)3, set->segmentsU),
                               std::max((uint)2, set->segmentsV), set->asTriangles);
    break;
    }

    g->scale(set->scale * set->scaleX,
             set->scale * set->scaleY,
             set->scale * set->scaleZ);

    if (!set->sharedVertices)
        g->unGroupVertices();

    if (set->convertToLines)
        g->convertToLines();
    else
        g->calculateTriangleNormals();
}


// ------------------------ factorysettings -------------------------


const QStringList GeometryFactorySettings::typeIds =
{
    "quad", "box", "grid", "sphere"
};

const QStringList GeometryFactorySettings::typeNames =
{
    QObject::tr("quad"),
    QObject::tr("box"),
    QObject::tr("grid"),
    QObject::tr("uv-sphere")
};

GeometryFactorySettings::GeometryFactorySettings()
    : type          (T_BOX),
      asTriangles   (true),
      convertToLines(false),
      sharedVertices(true),
      colorR        (0.5f),
      colorG        (0.5f),
      colorB        (0.5f),
      colorA        (1.0f),
      scale         (1.f),
      scaleX        (1.f),
      scaleY        (1.f),
      scaleZ        (1.f),
      gridSize      (1),
      segmentsU     (10),
      segmentsV     (10),
      withCoords    (true)
{

}




} // namespace GL
} // namespace MO
