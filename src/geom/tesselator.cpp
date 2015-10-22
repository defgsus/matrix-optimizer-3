#include <QPolygon>

#include "tesselator.h"
#include "geom/geometry.h"
#include "math/vector.h"
#include "io/log.h"
#include "io/error.h"
#include "3rd/polypartition/src/polypartition.h"

namespace MO {
namespace GEOM {

// -- Private --

class TesselatorPrivate
{
public:

    QVector<DVec2>
    /** Basis triangulation mesh */
        basis;
    QVector<QVector<DVec2>>
    /** input data (polygon outline) */
        input,
    /** input after intesection with basis mesh */
        meshedInput;
    /** output */
    std::list<TPPLPoly> out_tri;

    void clear();
    void checkLoop();
    /** Intersect every triangle in tris with @ref input and
        store in @ref multiInput */
    void intersectBasis();
    /** Calls glut functions on data in @ref input */
    void tesselate(bool trianglesOnly);
    /** Converts glut output to triangles in @ref final.
        Callback to GLU_TESS_END */
    void convertBlock();
    /** Fixes triangle winding in @ref final */
    void fixWinding();

    bool isWindingCorrect(const DVec2& a, const DVec2& b, const DVec2& c) const;
};

// -- Tesselator --

Tesselator::Tesselator()
    : p_    (new TesselatorPrivate())
{
    p_->clear();

}

Tesselator::~Tesselator()
{
    p_->clear();
    delete p_;
}

void Tesselator::clear()
{
    p_->clear();
}

void TesselatorPrivate::clear()
{
    input.clear();
    meshedInput.clear();
}

void Tesselator::setTriangulationMesh(const QVector<DVec2> &triangles)
{
    p_->basis = triangles;
}

void Tesselator::getExtend(
        const QVector<DVec2> &polygon, DVec2 &minEx, DVec2 &maxEx)
{
    auto i = polygon.begin();
    if (i == polygon.end())
    {
        minEx = DVec2(0.);
        maxEx = DVec2(0.);
        return;
    }
    minEx = maxEx = *i; ++i;
    for (; i != polygon.end(); ++i)
    {
        minEx.x = std::min(minEx.x, i->x);
        minEx.y = std::min(minEx.y, i->y);
        maxEx.x = std::max(maxEx.x, i->x);
        maxEx.y = std::max(maxEx.y, i->y);
    }
}

void Tesselator::createTriangulationMesh(
        const DVec2 &minExt, const DVec2 &maxExt, const DVec2 &st)
{
    QVector<DVec2> mesh;
    for (double j=minExt.y; j<maxExt.y; j += st.x)
    for (double i=minExt.x; i<maxExt.x; i += st.y)
    {
        mesh << DVec2(i,        j)
             << DVec2(i+st.x,   j)
             << DVec2(i,        j+st.y)
             << DVec2(i+st.x,   j)
             << DVec2(i+st.x,   j+st.y)
             << DVec2(i,        j+st.y);
    }

    setTriangulationMesh(mesh);
}

void TesselatorPrivate::checkLoop()
{
    if (input.isEmpty())
        return;
    QVector<DVec2> & in = input.back();
    if (in.front() == in.back())
        in.pop_back();
}

void Tesselator::addPolygon(const QVector<DVec2> &points)
{
    p_->input << points;
    p_->checkLoop();
}


void Tesselator::addPolygon(const QVector<Vec2> &points)
{
    p_->input << QVector<DVec2>();
    auto & in = p_->input.back();
    for (auto p : points)
        in.append(DVec2(p.x, p.y));
    p_->checkLoop();
}

void Tesselator::addPolygon(const QVector<QPointF> &points)
{
    p_->input << QVector<DVec2>();
    auto & in = p_->input.back();
    for (auto p : points)
        in.append(DVec2(p.x(), p.y()));
    p_->checkLoop();
}

void Tesselator::addPolygon(const QPolygonF &points)
{
    p_->input << QVector<DVec2>();
    auto & in = p_->input.back();
    for (auto p : points)
        in.append(DVec2(p.x(), p.y()));
    p_->checkLoop();
}

void Tesselator::tesselate()
{
    p_->tesselate(true);
}

void Tesselator::tesselate(const QVector<DVec2> & poly, bool trianglesOnly)
{
    p_->clear();
    addPolygon(poly);
    p_->tesselate(trianglesOnly);
}

void Tesselator::tesselate(const QVector<Vec2> & poly, bool trianglesOnly)
{
    p_->clear();
    addPolygon(poly);
    p_->tesselate(trianglesOnly);
}

void Tesselator::tesselate(const QVector<QPointF> & poly, bool trianglesOnly)
{
    p_->clear();
    addPolygon(poly);
    p_->tesselate(trianglesOnly);
}

void TesselatorPrivate::tesselate(bool /*trianglesOnly*/)
{
    if (!basis.isEmpty())
        intersectBasis();
    else
        meshedInput = input;

/*    MO_PRINT("-- " << meshedInput.size() << " polygons --");
    for (auto & p : meshedInput)
        MO_PRINT(p.size() << " vertices");
*/

    TPPLPartition partition;
    std::list<TPPLPoly> in_poly;

    for (const auto & input : meshedInput)
    {
        if (input.size() < 3)
            continue;
        int si = input.size();
        if (input.front() == input.back())
            --si;

        in_poly.push_back(TPPLPoly());
        TPPLPoly& poly = in_poly.back();

        poly.Init(si);
        poly.SetHole(false);
        for (int i=0; i<si; ++i)
        {
            poly.GetPoints()[i] = TPPLPoint(input[i].x, input[i].y);
        }
    }

    partition.Triangulate_EC(&in_poly, &out_tri);
//    MO_PRINT(out_tri.size() << " triangles");

    fixWinding();
}

void TesselatorPrivate::intersectBasis()
{
    QPolygonF shape;
    for (const auto & i : input)
        for (const auto & v : i)
            shape.append(QPointF(v.x, v.y));

    for (int i = 0; i < basis.size()/3; ++i)
    {
        QPolygonF tri;
        tri.append(QPointF(basis[i*3].x, basis[i*3].y));
        tri.append(QPointF(basis[i*3+1].x, basis[i*3+1].y));
        tri.append(QPointF(basis[i*3+2].x, basis[i*3+2].y));

        tri = shape.intersected(tri);
        if (tri.isEmpty())
            continue;

        meshedInput.append(QVector<DVec2>());
        auto & dst = meshedInput.back();
        for (const auto & p : tri)
        {
            dst << DVec2(p.x(), p.y());
        }
    }
}

/*
void TesselatorPrivate::convertBlock()
{
    if (!(  (primitiveType == GL_TRIANGLES
          || primitiveType == GL_TRIANGLE_STRIP
          || primitiveType == GL_TRIANGLE_FAN)
         && output.count() >= 3))
    {
        MO_WARNING("invalid data on call to TesselatorPrivate::convertBlock()");
        return;
    }

    if (primitiveType == GL_TRIANGLES)
    {
        final += output;
        return;
    }


    if (primitiveType == GL_TRIANGLE_FAN)
    {
        const DVec2 origin = output.at(0);

        for (int i=1; i<output.count()-1; ++i)
        {
            final.append(origin);
            final.append(output.at(i));
            final.append(output.at(i+1));
        }

        return;
    }


    if (primitiveType == GL_TRIANGLE_STRIP)
    {
        // first triangle
        DVec2   p1 = output.at(0),
                p2 = output.at(1),
                p3 = output.at(2);

        final.append(p1);
        final.append(p2);
        final.append(p3);

        for (int i=1; i<output.count()-2; ++i)
        {
            p1 = p2;
            p2 = p3;
            p3 = output.at(i+2);

            final.append(p1);
            final.append(p2);
            final.append(p3);
        }
    }

}
*/

bool TesselatorPrivate::isWindingCorrect(
        const DVec2 &a, const DVec2 &b, const DVec2 &c) const
{
    // two edges
    const auto x = b - a, y = c - a;
    // z-part of cross product
    const auto dz = x.x * y.y - y.x * x.y;
    return dz > 0.;
}

void TesselatorPrivate::fixWinding()
{
    /*
    for (int i=0; i<final.count()/3; ++i)
    {
        if (!isWindingCorrect(final[i*3], final[i*3+1], final[i*3+2]))
        {
            std::swap(final[i*3+1], final[i*3+2]);
        }
    }*/
}


bool Tesselator::isValid() const
{
    return p_->out_tri.size() >= 3;
}

uint Tesselator::numVertices() const
{
    return p_->out_tri.size() * 3;
}

uint Tesselator::numTriangles() const
{
    return p_->out_tri.size();
}

Geometry * Tesselator::getGeometry(bool asTriangles) const
{
    auto g = new Geometry();
    getGeometry(*g, asTriangles);
    return g;
}

void Tesselator::getGeometry(Geometry & g, bool asTriangles) const
{
    for (const TPPLPoly& tri : p_->out_tri)
    {
        if (tri.GetNumPoints() < 3)
            continue;

        const Geometry::IndexType
                p1 = g.addVertex(tri.GetPoint(0).x, tri.GetPoint(0).y, 0.f),
                p2 = g.addVertex(tri.GetPoint(1).x, tri.GetPoint(1).y, 0.f),
                p3 = g.addVertex(tri.GetPoint(2).x, tri.GetPoint(2).y, 0.f);

        if (asTriangles)
            g.addTriangle(p1, p2, p3);
        else
        {
            g.addLine(p1, p2);
            g.addLine(p2, p3);
            g.addLine(p3, p1);
        }
    }
}


} // namespace GEOM
} // namespace MO

