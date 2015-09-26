/** @file tesselator.cpp

    @brief QPolygon to triangle tesselator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include <glbinding/Meta.h>

#include <QPolygon>

#include "tesselator.h"
#include "geom/geometry.h"
#include "math/vector.h"
#include "io/log.h"
#include "io/error.h"

// must be last to avoid conflict with glbinding
#if defined(__APPLE__)
#   include <OpenGL/glu.h>
#else
#   include <GL/glu.h>
#endif

#if (0) && !defined(NDEBUG)
#   define MO_DO_DEBUG_TESS
#   define MO_DEBUG_TESS(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_TESS(unused__)
#endif


// hack for now
#define MO_DISABLE_GLU


namespace MO {
namespace GEOM {

// -- callbacks --

#ifndef CALLBACK
#   define CALLBACK
#endif

void CALLBACK moTesselatorBegin(GLenum which, void * usr);
void CALLBACK moTesselatorEnd(void * usr);
void CALLBACK moTesselatorEdgeFlag(GLboolean, void * usr);
void CALLBACK moTesselatorVertex(DVec2 * data, void * usr);
void CALLBACK moTesselatorCombine(
        GLdouble coords[3], void * d[4], GLfloat w[4], DVec2 ** out, void * usr);


// -- Private --

class TesselatorPrivate
{
public:

    GLUtesselator * ctx;

    QVector<DVec2>
    /** input data (polygon outline) */
        input,
    /** output of one begin/end block */
        output,
    /** final triangles (3 verts each) */
        final,
    /** Basis triangulation mesh */
        basis;
    QVector<QVector<DVec2>>
        multiInput;
    /** temporary storage (for combine callback) */
    QVector<DVec2*> temp;

    /** type of current begin/end block */
    GLenum primitiveType;

    void clear();
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
#ifndef MO_DISABLE_GLU
    // create tesselation object
    p_->ctx = gluNewTess();

    // set callbacks
    gluTessCallback(p_->ctx, GLU_TESS_BEGIN_DATA, (void (CALLBACK *)())moTesselatorBegin);
    gluTessCallback(p_->ctx, GLU_TESS_END_DATA, (void (CALLBACK *)())moTesselatorEnd);
    gluTessCallback(p_->ctx, GLU_TESS_VERTEX_DATA, (void (CALLBACK *)())moTesselatorVertex);
    gluTessCallback(p_->ctx, GLU_TESS_COMBINE_DATA, (void (CALLBACK *)())moTesselatorCombine);
    //gluTessCallback(p_->ctx, GLU_TESS_EDGE_FLAG_DATA, (void (CALLBACK *)())moTesselatorEdgeFlag);
#endif
}

Tesselator::~Tesselator()
{
#ifndef MO_DISABLE_GLU
    gluDeleteTess(p_->ctx);
#endif
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
    multiInput.clear();
    output.clear();
    final.clear();

    for (auto it : temp)
        delete it;
    temp.clear();

    primitiveType = 0;
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

void Tesselator::tesselate(const QVector<DVec2> & poly, bool trianglesOnly)
{
    // copy points
    p_->clear();
    p_->input = poly;

    p_->tesselate(trianglesOnly);
}

void Tesselator::tesselate(const QVector<Vec2> & poly, bool trianglesOnly)
{
    // copy points
    p_->clear();
    for (const Vec2& p : poly)
        p_->input.append(DVec2(p[0], p[1]));

    p_->tesselate(trianglesOnly);
}

void Tesselator::tesselate(const QVector<QPointF> & poly, bool trianglesOnly)
{
    // copy points
    p_->clear();
    for (const QPointF& p : poly)
        p_->input.append(DVec2(p.x(), p.y()));

    p_->tesselate(trianglesOnly);
}

void TesselatorPrivate::tesselate(bool trianglesOnly)
{
    if (!basis.isEmpty())
        intersectBasis();

#ifdef MO_DISABLE_GLU
    Q_UNUSED(trianglesOnly);
#else
    if (trianglesOnly)
        gluTessCallback(ctx, GLU_TESS_EDGE_FLAG_DATA, (void (CALLBACK *)())moTesselatorEdgeFlag);
    else
        gluTessCallback(ctx, GLU_TESS_EDGE_FLAG_DATA, 0);

    gluTessNormal(ctx, 0,0,1);

    if (multiInput.isEmpty())
    {
        gluTessBeginPolygon(ctx, (void*)this);
        gluTessBeginContour(ctx);

        for (const DVec2& p : input)
        {
            GLdouble v[] = { p[0], p[1], 0.0 };

            gluTessVertex(ctx, v, (void*)&p);
        }

        gluTessEndContour(ctx);
        gluTessEndPolygon(ctx);
    }
    else
    {
        for (const auto & poly : multiInput)
        {
            gluTessBeginPolygon(ctx, (void*)this);
            gluTessBeginContour(ctx);

            for (const DVec2& p : poly)
            {
                GLdouble v[] = { p[0], p[1], 0.0 };

                gluTessVertex(ctx, v, (void*)&p);
            }

            gluTessEndContour(ctx);
            gluTessEndPolygon(ctx);
        }
    }
#endif
    fixWinding();
}

void TesselatorPrivate::intersectBasis()
{
    QPolygonF shape;
    for (const auto & v : input)
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

        multiInput.append(QVector<DVec2>());
        auto & dst = multiInput.back();
        for (const auto & p : tri)
        {
            dst << DVec2(p.x(), p.y());
        }
    }
}


void moTesselatorBegin(GLenum which, void * usr)
{
    MO_DEBUG_TESS("begin " << glbinding::Meta::getString(gl::GLenum(which)));

    ((TesselatorPrivate*)usr)->primitiveType = which;
}

void moTesselatorEnd(void * usr)
{
    MO_DEBUG_TESS("end");

    ((TesselatorPrivate*)usr)->convertBlock();
    ((TesselatorPrivate*)usr)->output.clear();
}

void moTesselatorEdgeFlag(GLboolean
                        #ifdef MO_DO_DEBUG_TESS
                          flag
                        #endif
                          , void*)
{
    MO_DEBUG_TESS("edgeflag " << (flag ? "true" : "false"));
}

void moTesselatorVertex(DVec2 * p, void * usr)
{
    MO_DEBUG_TESS("vertex " << *p);

    ((TesselatorPrivate*)usr)->output.append(*p);
}

void moTesselatorCombine(
        GLdouble coords[3], void *[], GLfloat [], DVec2 **out, void *usr)
{
    MO_DEBUG_TESS("combine");

    // simply copy the new point (no other information needed)
    *out = new DVec2(coords[0], coords[1]);

    // append to list for memory managment
    ((TesselatorPrivate*)usr)->temp.append(*out);
}

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
    for (int i=0; i<final.count()/3; ++i)
    {
        if (!isWindingCorrect(final[i*3], final[i*3+1], final[i*3+2]))
        {
            std::swap(final[i*3+1], final[i*3+2]);
        }
    }
}


bool Tesselator::isValid() const
{
    return p_->final.count() >= 3;
}

uint Tesselator::numVertices() const
{
    return p_->final.count();
}

uint Tesselator::numTriangles() const
{
    return p_->final.count() / 3;
}

Geometry * Tesselator::getGeometry(bool asTriangles) const
{
    auto g = new Geometry();
    getGeometry(*g, asTriangles);
    return g;
}

void Tesselator::getGeometry(Geometry & g, bool asTriangles) const
{
    for (int i=0; i<p_->final.count()/3; ++i)
    {
        const Geometry::IndexType
                p1 = g.addVertex(p_->final.at(i*3  )[0], p_->final.at(i*3)[1], 0),
                p2 = g.addVertex(p_->final.at(i*3+1)[0], p_->final.at(i*3+1)[1], 0),
                p3 = g.addVertex(p_->final.at(i*3+2)[0], p_->final.at(i*3+2)[1], 0);

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
