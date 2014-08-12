/** @file geometryfactory.cpp

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#include <glm/gtx/rotate_vector.hpp>

#include <QObject>
#include <QHash>
#include <QFile>

#include "geometryfactory.h"
#include "geometry.h"
#include "objloader.h"
#include "math/vector.h"
#include "math/hash.h"
#include "io/datastream.h"

namespace MO {
namespace GEOM {


void GeometryFactory::createCube(Geometry * g, Float sl, bool asTriangles)
{
    return createBox(g, sl, sl, sl, asTriangles);
}

void GeometryFactory::createQuad(Geometry * g, Float sx, Float sy, bool asTriangles)
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
        Geometry * g, Float side_length_x, Float side_length_y, Float side_length_z,
        bool asTriangles)
{
    Float
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


void GeometryFactory::createGridXZ(Geometry * g, int sizeX, int sizeY, bool coords)
{
    g->setColor(0.5, 0.5, 0.5, 0.5);
    for (int i=-sizeX; i<=sizeX; ++i)
    {
        auto p1 = g->addVertex(i,0,-sizeY),
             p2 = g->addVertex(i,0,sizeY);
        g->addLine(p1, p2);
    }
    for (int i=-sizeY; i<=sizeY; ++i)
    {
        auto p1 = g->addVertex(-sizeX,0,i),
             p2 = g->addVertex( sizeX,0,i);
        g->addLine(p1, p2);
    }

    if (!coords)
        return;

    // coordinate system

    const Float marker = 0.1;

    g->setColor(1,0,0, 1);
    auto p1 = g->addVertex(0, 0.01, 0),
         p2 = g->addVertex(sizeX, 0.01, 0);
    g->addLine(p1, p2);
    for (int i=1; i<sizeX; ++i)
    {
        const int p =
        g->addVertex(i, 0.01 - marker, 0);
        g->addVertex(i, 0.01 + marker, 0);
        g->addLine(p, p+1);
    }

    const int size = std::min(sizeX, sizeY);
    g->setColor(0,1,0, 1);
    p1 = g->addVertex(0, 0.01, 0);
    p2 = g->addVertex(0, size, 0);
    g->addLine(p1, p2);
    for (int i=1; i<size; ++i)
    {
        const int p =
        g->addVertex(-marker, i, 0);
        g->addVertex( marker, i, 0);
        g->addLine(p, p+1);
    }

    g->setColor(0,0,1, 1);
    p1 = g->addVertex(0, 0.01, 0);
    p2 = g->addVertex(0, 0.01, sizeY);
    g->addLine(p1, p2);
    for (int i=1; i<sizeY; ++i)
    {
        const int p =
        g->addVertex(0, 0.01 - marker, i);
        g->addVertex(0, 0.01 + marker, i);
        g->addLine(p, p+1);
    }

}

void GeometryFactory::createGrid(Geometry * g, int sizeX, int sizeY, int sizeZ)
{
    const Geometry::IndexType start = g->numVertices();

    sizeX = std::max(1, sizeX);
    sizeY = std::max(1, sizeY);
    sizeZ = std::max(1, sizeZ);

    const Float
            ox = (Float)sizeX / 2 - 0.5,
            oy = (Float)sizeY / 2 - 0.5,
            oz = (Float)sizeZ / 2 - 0.5;

    for (int z=0; z<sizeZ; ++z)
    for (int y=0; y<sizeY; ++y)
    for (int x=0; x<sizeX; ++x)
    {
        g->addVertex(x-ox, y-oy, z-oz);
    }

#define MO__INDEX(x__, y__, z__) \
    (start + ((z__) * sizeY + (y__)) * sizeX + (x__))

    for (int z=0; z<sizeZ; ++z)
    for (int y=0; y<sizeY; ++y)
    for (int x=0; x<sizeX; ++x)
    {
        if (x>0)
            g->addLine(MO__INDEX(x,y,z), MO__INDEX(x-1,y,z));
        if (y>0)
            g->addLine(MO__INDEX(x,y,z), MO__INDEX(x,y-1,z));
        if (z>0)
            g->addLine(MO__INDEX(x,y,z), MO__INDEX(x,y,z-1));
    }

#undef MO__INDEX
}

void GeometryFactory::createUVSphere(
        Geometry * g, Float rad, uint segu, uint segv, bool asTriangles)
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
            Vec3 p = MATH::pointOnSphere((Float)u / segu, (Float)v / segv);

            g->setTexCoord((Float)(u+1) / (segu+1), 1.f - (Float)(v+1) / (segv+1));
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
        Geometry * g, Float rad, uint segu, uint segv)
{
    // top point
    g->addVertex(0, rad, 0);

    for (uint v = 1; v<segv; ++v)
    {
        // current vertex offset
        int rown = g->numVertices();

        // body vertices
        for (unsigned int u = 0; u<segu; ++u)
        {
            Vec3 p = MATH::pointOnSphere((Float)u / segu, (Float)v / segv);
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
    g->addVertex(0, -rad, 0);

    // connect to bottom point
    for (unsigned int u = 0; u<segu; ++u)
    {
        g->addLine(g->numVertices() - 1, rown + u);
    }
}

void GeometryFactory::createCylinder(Geometry * g, Float rad, Float height,
                                     uint segu, uint segv, bool open, bool asTriangles)
{
    segu = std::max((uint)3, segu);
    segv = std::max((uint)2, segv);

    uint start = g->numVertices();

    // create cylinder vertices
    for (uint y=0; y<segv; ++y)
    {
        const Float ty = (Float)y / (segv-1);

        for (uint x=0; x<segu; ++x)
        {
            const Float tx = (Float)x / (segu-1);
            const Float a = (Float)x / segu * TWO_PI;
            g->setTexCoord(tx, ty);
            g->addVertex(rad * sin(a), (ty-0.5) * height, rad * cos(a));
        }
    }

    // create surface

    height /= 2;

    if (!asTriangles)
    {
        for (uint y=0; y<segv; ++y)
        {
            for (uint x=0; x<segu; ++x)
            {
                // connect to next column
                g->addLine(start + y*segu + x,
                           start + y*segu + (x+1) % segu);
                // connect to row above
                if (y>0)
                    g->addLine(start + (y-1)*segu + x,
                               start + y*segu + x);
            }
        }

        // caps
        if (!open)
        {
            // bottom cap
            uint cap = g->numVertices();
            g->setTexCoord(0,0);
            g->addVertex(0, -height, 0);
            for (uint x=0; x<segu; ++x)
                g->addLine(cap, start + x);
            // top cap
            cap = g->numVertices();
            g->setTexCoord(0,1);
            g->addVertex(0, height, 0);
            for (uint x=0; x<segu; ++x)
                g->addLine(cap, start + (segv-1)*segu + x);
        }
    }
    else
    {
        for (uint y=0; y<segv; ++y)
        {
            if (y<segv-1)
            for (uint x=0; x<segu; ++x)
            {
                g->addTriangle(
                            start + y*segu + x,
                            start + y*segu + (x+1) % segu,
                            start + (y+1)*segu + (x+1) % segu);
                g->addTriangle(
                            start + y*segu + x,
                            start + (y+1)*segu + (x+1) % segu,
                            start + (y+1)*segu + x);
            }
        }

        // caps
        if (!open)
        {
            // bottom cap
            uint cap = g->numVertices();
            g->setTexCoord(0,0);
            g->addVertex(0, -height, 0);
            for (uint x=0; x<segu; ++x)
                g->addTriangle(cap, start + (x+1) % segu, start + x);
            // top cap
            cap = g->numVertices();
            g->setTexCoord(0,1);
            g->addVertex(0, height, 0);
            for (uint x=0; x<segu; ++x)
                g->addTriangle(cap, start + (segv-1)*segu + x,
                                    start + (segv-1)*segu + (x+1) % segu);
        }
    }
}



void GeometryFactory::createTorus(Geometry * g, Float rad_out, Float rad_in,
                                     uint segu, uint segv, bool asTriangles)
{
    segu = std::max((uint)3, segu);
    segv = std::max((uint)3, segv);

    uint start = g->numVertices();

    // create torus vertices
    for (uint y=0; y<segv; ++y)
    {
        const Float ty = (Float)y / (segv-1);
        const Float ang = (Float)y / segv * 360.f;

        for (uint x=0; x<segu; ++x)
        {
            const Float tx = (Float)x / (segu-1);
            const Float a = (Float)x / segu * TWO_PI;

            const Vec3 v = glm::rotateY(
                        Vec3(rad_out + rad_in*sin(a), rad_in*cos(a), 0),
                        ang);

            g->setTexCoord(tx, ty);
            g->addVertex(v[0], v[1], v[2]);
        }
    }

    // create surface

    if (!asTriangles)
    {
        for (uint y=0; y<segv; ++y)
        {
            for (uint x=0; x<segu; ++x)
            {
                // connect to next column
                g->addLine(start + y*segu + x,
                           start + y*segu + (x+1) % segu);
                // connect to next row
                g->addLine(start + y*segu + x,
                           start + ((y+1)%segv)*segu + x);
            }
        }

    }
    else
    {
        for (uint y=0; y<segv; ++y)
        {
            for (uint x=0; x<segu; ++x)
            {
                g->addTriangle(
                            start + y*segu + x,
                            start + y*segu + (x+1) % segu,
                            start + ((y+1)%segv)*segu + (x+1) % segu);
                g->addTriangle(
                            start + y*segu + x,
                            start + ((y+1)%segv)*segu + (x+1) % segu,
                            start + ((y+1)%segv)*segu + x);
            }
        }

    }
}


void GeometryFactory::createOctahedron(Geometry * g, Float scale, bool asTriangles)
{
    const Float
        a = 0.5f * scale / std::sqrt(2.f * std::sqrt(2.f)),
        b = 0.5f * scale;

    const int p0 = g->addVertex( 0,  b,  0);
    const int p1 = g->addVertex(-a,  0,  a);
    const int p2 = g->addVertex( a,  0,  a);
    const int p3 = g->addVertex( a,  0, -a);
    const int p4 = g->addVertex(-a,  0, -a);
    const int p5 = g->addVertex( 0, -b,  0);

    if (asTriangles)
    {
        g->addTriangle( p0,p1,p2 );
        g->addTriangle( p0,p2,p3 );
        g->addTriangle( p0,p3,p4 );
        g->addTriangle( p0,p4,p1 );
        g->addTriangle( p5,p2,p1 );
        g->addTriangle( p5,p3,p2 );
        g->addTriangle( p5,p4,p3 );
        g->addTriangle( p5,p1,p4 );
    }
    else
    {
        g->addLine( p0,p1 );
        g->addLine( p0,p2 );
        g->addLine( p0,p3 );
        g->addLine( p0,p4 );
        g->addLine( p1,p2 );
        g->addLine( p1,p4 );
        g->addLine( p1,p5 );
        g->addLine( p2,p3 );
        g->addLine( p2,p5 );
        g->addLine( p3,p4 );
        g->addLine( p3,p5 );
        g->addLine( p4,p5 );
    }
}

void GeometryFactory::createIcosahedron(Geometry * g, Float scale, bool asTriangles)
{
    const Float
        a = 0.5f * scale,
        b = scale / (1.f + std::sqrt(5.f));

    const int p0  = g->addVertex(  0,  b, -a );
    const int p1  = g->addVertex(  b,  a,  0 );
    const int p2  = g->addVertex( -b,  a,  0 );
    const int p3  = g->addVertex(  0,  b,  a );
    const int p4  = g->addVertex(  0, -b,  a );
    const int p5  = g->addVertex( -a,  0,  b );
    const int p6  = g->addVertex(  0, -b, -a );
    const int p7  = g->addVertex(  a,  0, -b );
    const int p8  = g->addVertex(  a,  0,  b );
    const int p9  = g->addVertex( -a,  0, -b );
    const int p10 = g->addVertex(  b, -a,  0 );
    const int p11 = g->addVertex( -b, -a,  0 );

    if (asTriangles)
    {
        g->addTriangle( p0, p2, p1 );
        g->addTriangle( p3, p1, p2 );
        g->addTriangle( p3, p5, p4 );
        g->addTriangle( p3, p4, p8 );
        g->addTriangle( p0, p7, p6 );
        g->addTriangle( p0, p6, p9 );
        g->addTriangle( p4, p11, p10 );
        g->addTriangle( p6, p10, p11 );
        g->addTriangle( p2, p9, p5 );
        g->addTriangle( p11, p5, p9 );
        g->addTriangle( p1, p8, p7 );
        g->addTriangle( p10, p7, p8 );
        g->addTriangle( p3, p2, p5 );
        g->addTriangle( p3, p8, p1 );
        g->addTriangle( p0, p9, p2 );
        g->addTriangle( p0, p1, p7 );
        g->addTriangle( p6, p11, p9 );
        g->addTriangle( p6, p7, p10 );
        g->addTriangle( p4, p5, p11 );
        g->addTriangle( p4, p10, p8 );
    }
    else
    {
        g->addLine( p0, p1 );
        g->addLine( p0, p2 );
        g->addLine( p0, p6 );
        g->addLine( p0, p7 );
        g->addLine( p0, p9 );
        g->addLine( p1, p2 );
        g->addLine( p1, p3 );
        g->addLine( p1, p7 );
        g->addLine( p1, p8 );
        g->addLine( p2, p3 );
        g->addLine( p2, p5 );
        g->addLine( p2, p9 );
        g->addLine( p3, p4 );
        g->addLine( p3, p5 );
        g->addLine( p3, p8 );
        g->addLine( p4, p5 );
        g->addLine( p4, p8 );
        g->addLine( p4, p10 );
        g->addLine( p4, p11 );
        g->addLine( p5, p9 );
        g->addLine( p5, p11 );
        g->addLine( p6, p7 );
        g->addLine( p6, p9 );
        g->addLine( p6, p10 );
        g->addLine( p6, p11 );
        g->addLine( p7, p8 );
        g->addLine( p7, p10 );
        g->addLine( p8, p10 );
        g->addLine( p9, p11 );
        g->addLine( p10, p11 );
    }
}

void GeometryFactory::createTetrahedron(Geometry * g, Float scale, bool asTriangles)
{
    const Float a = 0.5f * scale;

    const int p0 = g->addVertex( -a,  a,  a );
    const int p1 = g->addVertex(  a,  a, -a );
    const int p2 = g->addVertex( -a, -a, -a );
    const int p3 = g->addVertex(  a, -a,  a );

    if (asTriangles)
    {
        g->addTriangle( p0,p1,p2 );
        g->addTriangle( p0,p3,p1 );
        g->addTriangle( p0,p2,p3 );
        g->addTriangle( p1,p3,p2 );
    }
    else
    {
        g->addLine( p0,p1 );
        g->addLine( p0,p2 );
        g->addLine( p0,p3 );
        g->addLine( p1,p2 );
        g->addLine( p1,p3 );
        g->addLine( p2,p3 );
    }
}

void GeometryFactory::createDodecahedron(Geometry * g, Float scale, bool asTriangles)
{
    // XXX Need to fix triangle winding order!

    const Float
        phi = (1.f + std::sqrt(5.f)) / 2.f,
        a = 0.5f * scale,
        b = 0.5f / phi,
        c = 0.5f * (2.f - phi);

    std::vector<Float> pos = { 0.f, a, -a, b, -b, c, -c };
    enum Pindex
    {
        p_0, p_a, p_ma, p_b, p_mb, p_c, p_mc
    };

    QHash<int, Geometry::IndexType> hash;
    std::vector<Geometry::IndexType> idx;

    #define MO__ADDPOINT(x, y, z)                                   \
    {                                                               \
        const int h = MATH::getHash<int>(x, y, z);                  \
        auto i = hash.find(h);                                      \
        if (i == hash.end())                                        \
        {                                                           \
            auto index = g->addVertex( pos[x], pos[y], pos[z] );    \
            idx.push_back(index);                                   \
            hash.insert(h, index);                                  \
        }                                                           \
        else                                                        \
            idx.push_back( i.value() );                             \
    }

    #define MO__PENT(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,x5,y5,z5) \
        MO__ADDPOINT( x1,y1,z1 ); \
        MO__ADDPOINT( x2,y2,z2 ); \
        MO__ADDPOINT( x3,y3,z3 ); \
        MO__ADDPOINT( x4,y4,z4 ); \
        MO__ADDPOINT( x5,y5,z5 );

    MO__PENT(
     p_c,p_0,p_a,
     p_mc,p_0,p_a,
     p_mb,p_b,p_b,
     p_0,p_a,p_c,
     p_b,p_b,p_b);
    MO__PENT(
     p_mc,p_0,p_a,
     p_c,p_0,p_a,
     p_b,p_mb,p_b,
     p_0,p_ma,p_c,
     p_mb,p_mb,p_b);
    MO__PENT(
     p_c,p_0,p_ma,
     p_mc,p_0,p_ma,
     p_mb,p_mb,p_mb,
     p_0,p_ma,p_mc,
     p_b,p_mb,p_mb);
    MO__PENT(
     p_mc,p_0,p_ma,
     p_c,p_0,p_ma,
     p_b,p_b,p_mb,
     p_0,p_a,p_mc,
     p_mb,p_b,p_mb);
    MO__PENT(
     p_0,p_a,p_mc,
     p_0,p_a,p_c,
     p_b,p_b,p_b,
     p_a,p_c,p_0,
     p_b,p_b,p_mb);
    MO__PENT(
     p_0,p_a,p_c,
     p_0,p_a,p_mc,
     p_mb,p_b,p_mb,
     p_ma,p_c,p_0,
     p_mb,p_b,p_b);
    MO__PENT(
     p_0,p_ma,p_mc,
     p_0,p_ma,p_c,
     p_mb,p_mb,p_b,
     p_ma,p_mc,p_0,
     p_mb,p_mb,p_mb);
    MO__PENT(
     p_0,p_ma,p_c,
     p_0,p_ma,p_mc,
     p_b,p_mb,p_mb,
     p_a,p_mc,p_0,
     p_b,p_mb,p_b);
    MO__PENT(
     p_a,p_c,p_0,
     p_a,p_mc,p_0,
     p_b,p_mb,p_b,
     p_c,p_0,p_a,
     p_b,p_b,p_b);
    MO__PENT(
     p_a,p_mc,p_0,
     p_a,p_c,p_0,
     p_b,p_b,p_mb,
     p_c,p_0,p_ma,
     p_b,p_mb,p_mb);
    MO__PENT(
     p_ma,p_c,p_0,
     p_ma,p_mc,p_0,
     p_mb,p_mb,p_mb,
     p_mc,p_0,p_ma,
     p_mb,p_b,p_mb);
    MO__PENT(
     p_ma,p_mc,p_0,
     p_ma,p_c,p_0,
     p_mb,p_b,p_b,
     p_mc,p_0,p_a,
     p_mb,p_mb,p_b);

    #undef MO__PENT
    #undef MO__ADDPOINT

    if (asTriangles)
        for (unsigned int i=0; i<idx.size(); i+=5)
        {
            g->addTriangle( idx[i+0], idx[i+1], idx[i+4]);
            g->addTriangle( idx[i+1], idx[i+2], idx[i+4]);
            g->addTriangle( idx[i+4], idx[i+2], idx[i+3]);
        }
    else
        for (unsigned int i=0; i<idx.size(); i+=5)
        {
            g->addLine( idx[i+0], idx[i+1] );
            g->addLine( idx[i+1], idx[i+2] );
            g->addLine( idx[i+2], idx[i+3] );
            g->addLine( idx[i+3], idx[i+4] );
            g->addLine( idx[i+4], idx[i+0] );
        }

}






void GeometryFactory::createFromSettings(Geometry * g,
                                         const GeometryFactorySettings * set,
                                         ObjLoader * loader_)
{
    // shared vertices?
    g->setSharedVertices(set->sharedVertices);

    // initial color
    g->setColor(set->colorR, set->colorG, set->colorB, set->colorA);

    // create mesh
    switch (set->type)
    {
    case GeometryFactorySettings::T_FILE:
        if (!set->filename.isEmpty() && loader_)
        {
            loader_->loadFile(set->filename);
            loader_->getGeometry(g);
        }
    break;

    case GeometryFactorySettings::T_QUAD:
        createQuad(g, 1.f, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_BOX:
        createCube(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_GRID_XZ:
        createGridXZ(g, set->segmentsX, set->segmentsY, set->withCoords);
    break;

    case GeometryFactorySettings::T_GRID:
        createGrid(g, set->segmentsX, set->segmentsY, set->segmentsZ);
    break;

    case GeometryFactorySettings::T_UV_SPHERE:
        createUVSphere(g, 1.f, std::max((uint)3, set->segmentsX),
                               std::max((uint)2, set->segmentsY), set->asTriangles);
    break;

    case GeometryFactorySettings::T_TETRAHEDRON:
        createTetrahedron(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_OCTAHEDRON:
        createOctahedron(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_ICOSAHEDRON:
        createIcosahedron(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_DODECAHEDRON:
        createDodecahedron(g, 1.f, set->asTriangles);
    break;

    case GeometryFactorySettings::T_CYLINDER_CLOSED:
        createCylinder(g, 1.f, 1.f, set->segmentsX, set->segmentsY, false, set->asTriangles);
    break;

    case GeometryFactorySettings::T_CYLINDER_OPEN:
        createCylinder(g, 1.f, 1.f, set->segmentsX, set->segmentsY, true, set->asTriangles);
    break;

    case GeometryFactorySettings::T_TORUS:
        createTorus(g, 1.f, set->smallRadius, set->segmentsX, set->segmentsY, set->asTriangles);
    break;

    }

    // --- modify ---

    if (set->tesselate)
        g->tesselate(set->tessLevel);

    if (set->normalizeVertices)
        g->normalizeSphere(1, set->normalization);

    if (!set->sharedVertices
            //&& set->type != GeometryFactorySettings::T_FILE
            )
        g->unGroupVertices();

    if (set->convertToLines)
        g->convertToLines();

    if (set->removeRandomly)
        g->removePrimitivesRandomly(set->removeProb, set->removeSeed);

    if (set->transformWithEquation)
        g->transformWithEquation(set->equationX, set->equationY, set->equationZ);

    if (g->numTriangles() && set->calcNormalsBeforePrimitiveEquation)
        g->calculateTriangleNormals();

    if (set->transformPrimitivesWithEquation)
        g->transformPrimitivesWithEquation(set->pEquationX, set->pEquationY, set->pEquationZ);

    g->scale(set->scale * set->scaleX,
             set->scale * set->scaleY,
             set->scale * set->scaleZ);

    if (g->numTriangles() && set->calcNormals)
        g->calculateTriangleNormals();

    if (set->invertNormals)
        g->invertNormals();

}











// ------------------------ factorysettings -------------------------


const QStringList GeometryFactorySettings::typeIds =
{
    "file", "quad",
    "tetra", "hexa", "octa", "icosa", "dodeca",
    "cyl", "cylo", "torus",
    "gridxz", "grid", "uvsphere"
};

const QStringList GeometryFactorySettings::typeNames =
{
    QObject::tr("file"),
    QObject::tr("quad"),
    QObject::tr("tetrahedron"),
    QObject::tr("hexahedron (cube)"),
    QObject::tr("octahedron"),
    QObject::tr("icosahedron"),
    QObject::tr("dodecahedron"),
    QObject::tr("cylinder (closed)"),
    QObject::tr("cylinder (open)"),
    QObject::tr("torus"),
    QObject::tr("grid xz"),
    QObject::tr("grid"),
    QObject::tr("uv-sphere")
};

GeometryFactorySettings::GeometryFactorySettings()
    : type          (T_BOX),
      equationX     ("x"),
      equationY     ("y"),
      equationZ     ("z"),
      pEquationX    ("x"),
      pEquationY    ("y"),
      pEquationZ    ("z"),
      calcNormals   (true),
      invertNormals (false),
      asTriangles   (true),
      convertToLines(false),
      sharedVertices(true),
      tesselate     (false),
      normalizeVertices(false),
      removeRandomly(false),
      transformWithEquation(false),
      transformPrimitivesWithEquation(false),
      calcNormalsBeforePrimitiveEquation(false),
      colorR        (0.5f),
      colorG        (0.5f),
      colorB        (0.5f),
      colorA        (1.0f),
      scale         (1.f),
      scaleX        (1.f),
      scaleY        (1.f),
      scaleZ        (1.f),
      removeProb    (0.1f),
      normalization (1.f),
      smallRadius   (0.2f),
      gridSize      (1),
      segmentsX     (10),
      segmentsY     (10),
      segmentsZ     (1),
      tessLevel     (1),
      removeSeed    (1),
      withCoords    (true)
{

}

void GeometryFactorySettings::serialize(IO::DataStream & io) const
{
    io.writeHeader("geomfacset", 6);

    io << typeIds[type];

    io << calcNormals << asTriangles << convertToLines << sharedVertices
        << tesselate << normalizeVertices << removeRandomly
        << colorR << colorG << colorB << colorA
        << scale << scaleX << scaleY << scaleZ
        << removeProb << gridSize
        << segmentsX << segmentsY << segmentsZ
        << tessLevel << removeSeed << withCoords;

    // v6
    io << invertNormals;

    // v2
    io << normalization;

    // v3
    io << transformWithEquation << equationX << equationY << equationZ;

    // v4
    io << smallRadius;

    // v5
    io << transformPrimitivesWithEquation << calcNormalsBeforePrimitiveEquation
       << pEquationX << pEquationY << pEquationZ;
}

void GeometryFactorySettings::deserialize(IO::DataStream & io)
{
    int ver = io.readHeader("geomfacset", 6);

    io.readEnum(type, T_BOX, typeIds);

    io >> calcNormals >> asTriangles >> convertToLines >> sharedVertices
        >> tesselate >> normalizeVertices >> removeRandomly
        >> colorR >> colorG >> colorB >> colorA
        >> scale >> scaleX >> scaleY >> scaleZ
        >> removeProb >> gridSize
        >> segmentsX >> segmentsY >> segmentsZ
        >> tessLevel >> removeSeed >> withCoords;

    if (ver >= 6)
        io >> invertNormals;

    if (ver >= 2)
        io >> normalization;

    if (ver >= 3)
        io >> transformWithEquation >> equationX >> equationY >> equationZ;

    if (ver >= 4)
        io >> smallRadius;

    if (ver >= 5)
        io >> transformPrimitivesWithEquation >> calcNormalsBeforePrimitiveEquation
           >> pEquationX >> pEquationY >> pEquationZ;

}

void GeometryFactorySettings::saveFile(const QString &filename) const
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not create geometry-settings file '" << filename
                    << "'\n" << f.errorString());

    IO::DataStream stream(&f);

    MO_EXTEND_EXCEPTION(
        serialize(stream),
        "on writing geometry-settings file '" << filename << "'"
        );
}


void GeometryFactorySettings::loadFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(WRITE, "Could not open geometry-settings file '" << filename
                    << "'\n" << f.errorString());

    IO::DataStream stream(&f);
    MO_EXTEND_EXCEPTION(
        deserialize(stream),
        "on reading geometry-settings file '" << filename << "'"
        );
}

} // namespace GEOM
} // namespace MO
