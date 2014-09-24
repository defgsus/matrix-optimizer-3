/** @file tesselator.cpp

    @brief QPolygon to triangle tesselator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include <glbinding/Meta.h>

#include <QPolygon>

#include "tesselator.h"
#include "io/log.h"
#include "io/error.h"
#include "geom/geometry.h"

// must be last to avoid conflict with glbinding
#include <GL/glu.h>

namespace MO {
namespace GEOM {

// -- callbacks --

#ifndef CALLBACK
#   define CALLBACK
#endif

void CALLBACK moTesselatorBegin(GLenum which, void * usr);
void CALLBACK moTesselatorEnd(void * usr);
void CALLBACK moTesselatorVertex(DVec2 * data, void * usr);
void CALLBACK moTesselatorCombine(
        GLdouble coords[3], void * d[4], GLfloat w[4], DVec2 ** out, void * usr);


// -- Private --

class TesselatorPrivate
{
public:

    GLUtesselator * ctx;

    QVector<DVec2> input, output;
    QVector<DVec2*> temp;

    GLenum primitiveType;

    void clear();
    void tesselate();
};

// -- Tesselator --

Tesselator::Tesselator()
    : p_    (new TesselatorPrivate())
{
    p_->clear();

    // create tesselation object
    p_->ctx = gluNewTess();

    // set callbacks
    gluTessCallback(p_->ctx, GLU_TESS_BEGIN_DATA, (void (CALLBACK *)())moTesselatorBegin);
    gluTessCallback(p_->ctx, GLU_TESS_END_DATA, (void (CALLBACK *)())moTesselatorEnd);
    gluTessCallback(p_->ctx, GLU_TESS_VERTEX_DATA, (void (CALLBACK *)())moTesselatorVertex);
    gluTessCallback(p_->ctx, GLU_TESS_COMBINE_DATA, (void (CALLBACK *)())moTesselatorCombine);
}

Tesselator::~Tesselator()
{
    gluDeleteTess(p_->ctx);
    p_->clear();
    delete p_;
}


void Tesselator::tesselate(const QVector<DVec2> & poly)
{
    // copy points
    p_->clear();
    p_->input = poly;

    p_->tesselate();
}

void Tesselator::tesselate(const QVector<Vec2> & poly)
{
    // copy points
    p_->clear();
    for (const Vec2& p : poly)
        p_->input.append(DVec2(p[0], p[1]));

    p_->tesselate();
}

void Tesselator::tesselate(const QVector<QPointF> & poly)
{
    // copy points
    p_->clear();
    for (const QPointF& p : poly)
        p_->input.append(DVec2(p.x(), p.y()));

    p_->tesselate();
}

void TesselatorPrivate::tesselate()
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

void TesselatorPrivate::clear()
{
    input.clear();
    output.clear();

    for (auto it : temp)
        delete it;
    temp.clear();

    primitiveType = 0;
}


void moTesselatorBegin(GLenum which, void * usr)
{
    MO_DEBUG("begin " << glbinding::Meta::getString(gl::GLenum(which)));

    ((TesselatorPrivate*)usr)->primitiveType = which;
}

void moTesselatorEnd(void * )
{
    MO_DEBUG("end");
}

void moTesselatorVertex(DVec2 * p, void * usr)
{
    MO_DEBUG("vertex " << *p);

    ((TesselatorPrivate*)usr)->output.append(*p);
}

void moTesselatorCombine(
        GLdouble coords[3], void *[], GLfloat [], DVec2 **out, void *usr)
{
    MO_DEBUG("combine");

    // simply copy the new point (no other information needed)
    *out = new DVec2(coords[0], coords[1]);

    // append to list for memory managment
    ((TesselatorPrivate*)usr)->temp.append(*out);
}


bool Tesselator::isValid() const
{
    return ((p_->primitiveType == GL_TRIANGLES
            || p_->primitiveType == GL_TRIANGLE_STRIP
            || p_->primitiveType == GL_TRIANGLE_FAN)
            && p_->output.count() >= 3);
}

uint Tesselator::numVertices() const
{
    return p_->output.count();
}

uint Tesselator::numTriangles() const
{
    if (p_->primitiveType == GL_TRIANGLES)
        return p_->output.count() / 3;
    if (p_->primitiveType == GL_TRIANGLE_STRIP
        || p_->primitiveType == GL_TRIANGLE_FAN)
        return p_->output.count() - 2;

    MO_WARNING("undefined primitive tyoe in Tesselator::numTriangles()");
    return 0;
}

Geometry * Tesselator::getGeometry() const
{
    auto g = new Geometry();
    getGeometry(*g);
    return g;
}

void Tesselator::getGeometry(Geometry & g) const
{
    // makes sure that primitiveType is defined
    // and there is at least one triangle
    if (!isValid())
    {
        MO_WARNING("invalid data on call to Tesselator::getGeometry()");
        return;
    }

    if (p_->primitiveType == GL_TRIANGLES)
    {
        for (int i=0; i<p_->output.count()/3; ++i)
        {
            const Geometry::IndexType
                    p1 = g.addVertex(p_->output.at(i*3)[0], p_->output.at(i*3)[1], 0),
                    p2 = g.addVertex(p_->output.at(i*3+1)[0], p_->output.at(i*3+1)[1], 0),
                    p3 = g.addVertex(p_->output.at(i*3+2)[0], p_->output.at(i*3+2)[1], 0);
            g.addTriangle(p1,p2,p3);
        }

        return;
    }


    if (p_->primitiveType == GL_TRIANGLE_FAN)
    {
        const Geometry::IndexType
                origin = g.addVertex(p_->output.at(0)[0], p_->output.at(0)[1], 0);

        for (int i=1; i<p_->output.count()-1; ++i)
        {
            const Geometry::IndexType
                    p1 = g.addVertex(p_->output.at(i)[0], p_->output.at(i)[1], 0),
                    p2 = g.addVertex(p_->output.at(i+1)[0], p_->output.at(i+1)[1], 0);

            g.addTriangle(origin, p1, p2);
        }

        return;
    }


    if (p_->primitiveType == GL_TRIANGLE_STRIP)
    {
        // first triangle
        Geometry::IndexType
                p1 = g.addVertex(p_->output.at(0)[0], p_->output.at(0)[1], 0),
                p2 = g.addVertex(p_->output.at(1)[0], p_->output.at(1)[1], 0),
                p3 = g.addVertex(p_->output.at(2)[0], p_->output.at(2)[1], 0);

        g.addTriangle(p1,p2,p3);

        for (int i=1; i<p_->output.count()-1; ++i)
        {
            p1 = p2;
            p2 = p3;
            p3 = g.addVertex(p_->output.at(i+2)[0], p_->output.at(i+2)[1], 0);

            if (i&1)
                g.addTriangle(p2,p1,p3);
            else
                g.addTriangle(p1, p2, p3);
        }
    }
}


} // namespace GEOM
} // namespace MO
