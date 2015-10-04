#include <QDebug>
#include <QPainterPath>
#include <QMap>
#include <QVector>
#include <QTransform>

#include "3rd/polypartition/src/polypartition.h"

#include "textmesh.h"
#include "geometry.h"
#include "types/properties.h"


namespace MO {
namespace GEOM {

struct TextMesh::Private
{
    struct Char
    {
        QList<QPolygonF> polys;
        QSizeF size;
        std::list<TPPLPoly> tris;
    };

    Private(TextMesh * p) : p(p)
    {

    }

    void createProps();

    void getPolys();
    void triangulate(Char&);
    void createLines(Geometry * g);
    void createTriangles(Geometry * g);
    void getGeometry(Geometry * g);

    TextMesh * p;
    Properties props;

    QMap<QChar, Char> polyMap;
};


TextMesh::TextMesh()
    : p_        (new Private(this))
{
    p_->createProps();
}

TextMesh::~TextMesh()
{
    delete p_;
}

const Properties& TextMesh::properties() const
{
    return p_->props;
}

void TextMesh::setProperties(const Properties & p)
{
    p_->props = p;
}

void TextMesh::setText(const QString& t)
{
    p_->props.set("text", t);
}

void TextMesh::setFont(const QFont& f)
{
    p_->props.set("font", f);
}

void TextMesh::getGeometry(Geometry *g) const
{
    p_->getGeometry(g);
}

void TextMesh::Private::createProps()
{
    props.set(
        "text", QObject::tr("text"),
        QObject::tr("The text to create as geometry"),
        QObject::tr("Text"));
    props.setSubType("text", Properties::ST_TEXT);

    Properties::NamedValues modes;
    modes.set("polyline", QObject::tr("polygon outline"), M_POLYLINE);
    modes.set("tess", QObject::tr("tesselate"), M_TESS);
    props.set("mode", QObject::tr("mode"),
                     QObject::tr("Mode of tesselation / geometry creation"),
                     modes, int(M_POLYLINE));

    props.set("depth", QObject::tr("depth"),
              QObject::tr("The visual depth in default units for extruded geometry"),
              0.f);

    props.set("font", QObject::tr("font"),
            QObject::tr("The font to use"),
            QFont());
}

void TextMesh::Private::getGeometry(Geometry *g)
{
    getPolys();

    int mode = props.get("mode").toInt();
    if (mode == M_POLYLINE)
    {
        createLines(g);
        return;
    }

    for (auto & c : polyMap)
        triangulate(c);

    if (mode == M_TESS)
    {
        createTriangles(g);
        return;
    }

}

void TextMesh::Private::getPolys()
{
    polyMap.clear();

    const QString text = props.get("text").toString();
    const QFont font = props.get("font").value<QFont>();

    for (auto chr : text)
    {
        // don't create duplicates
        {
            auto i = polyMap.find(chr);
            if (i != polyMap.end())
                continue;
        }

        // get the painterpath of each char
        QPainterPath path;
        path.addText(0, 0, font, chr);

        // create the poly struct
        Char c;
        // flip y
        // XXX break triangulation
        QTransform trans;
        //trans.scale(1., -1.);
        // get list of polygons
        c.polys = path.toSubpathPolygons(trans);

        // flip y
        /*for (auto & poly : c.polys)
            for (auto & p : poly)
                p.ry() = -p.y();
        */

        // get extend
        c.size = path.boundingRect().size();

        // remember
        polyMap.insert(chr, c);
    }
}

void TextMesh::Private::triangulate(Char & c)
{
    std::list<TPPLPoly> inp;

    for (const QPolygonF& poly : c.polys)
    {
        // convert to TPPLPoly
        TPPLPoly tp;
        tp.Init(poly.size());
        for (int i=0; i<poly.size(); ++i)
            tp.GetPoint(i) = TPPLPoint(poly[i].x(), poly[i].y());

        inp.push_back(tp);
    }

    // triangulate

    TPPLPartition part;

    std::list<TPPLPoly> tris;
    part.Triangulate_EC(&inp, &tris);

    // get triangles
    c.tris.clear();
    for (TPPLPoly & tri : tris)
    {
        if (tri.GetNumPoints() != 3)
            continue;

        // fix orientation
        if (tri.GetOrientation() == TPPL_CCW)
        {
            std::swap(tri.GetPoint(1), tri.GetPoint(2));
        }

        tri.GetPoint(0).y *= -1.;
        tri.GetPoint(1).y *= -1.;
        tri.GetPoint(2).y *= -1.;

        c.tris.push_back( tri );
    }
}

void TextMesh::Private::createLines(Geometry *g)
{
    const QString text = props.get("text").toString();

    qreal x = 0., y = 0.;
    for (auto chr : text)
    {
        auto mapi = polyMap.find(chr);
        if (mapi == polyMap.end())
            continue;

        const Char& c = mapi.value();

        for (const QPolygonF& p : c.polys)
        {
            if (p.empty())
                continue;

            int i;
            for (i=0; i<p.size()-2; ++i)
            {
                auto p1 = g->addVertex( p[i].x() + x,
                                       -p[i].y() + y, 0),
                     p2 = g->addVertex( p[i+1].x() + x,
                                       -p[i+1].y() + y, 0);
                g->addLine(p1, p2);
            }
            if (p.isClosed())
            {
                auto p1 = g->addVertex( p[i].x() + x,
                                       -p[i].y() + y, 0),
                     p2 = g->addVertex( p[0].x() + x,
                                       -p[0].y() + y, 0);
                g->addLine(p1, p2);
            }

            x += (c.size.width()) * 1.1;
        }

    }

}

void TextMesh::Private::createTriangles(Geometry *g)
{
    const QString text = props.get("text").toString();
    const Float depth = props.get("depth").toFloat();

    qreal x = 0., y = 0.;
    for (auto chr : text)
    {
        auto mapi = polyMap.find(chr);
        if (mapi == polyMap.end())
            continue;

        const Char& c = mapi.value();

        for (const TPPLPoly& p : c.tris)
        {
            if (p.GetNumPoints() < 3)
                continue;

            Float x1 = p.GetPoint(0).x + x,
                  x2 = p.GetPoint(1).x + x,
                  x3 = p.GetPoint(2).x + x,
                  y1 = p.GetPoint(0).y + y,
                  y2 = p.GetPoint(1).y + y,
                  y3 = p.GetPoint(2).y + y;

            auto p1 = g->addVertex(x1, y1, 0),
                 p2 = g->addVertex(x2, y2, 0),
                 p3 = g->addVertex(x3, y3, 0);

            g->addTriangle(p1, p2, p3);

            if (depth > 0.f)
            {
                if (g->sharedVertices())
                {
                    auto pd1 = g->addVertex(x1, y1, -depth),
                         pd2 = g->addVertex(x2, y2, -depth),
                         pd3 = g->addVertex(x3, y3, -depth);

                    g->addTriangle(p1, pd1, p2);
                    g->addTriangle(p2, pd1, pd2);
                    g->addTriangle(p2, pd2, p3);
                    g->addTriangle(p3, pd2, pd3);
                    g->addTriangle(p3, pd1, p1);
                    g->addTriangle(p3, pd3, pd1);

                    g->addTriangle(pd1, pd3, pd2);
                }
                else
                {
                    const Vec3
                            v1 =  Vec3(x1, y1, 0),
                            v2 =  Vec3(x2, y2, 0),
                            v3 =  Vec3(x3, y3, 0),
                            vd1 = Vec3(x1, y1, -depth),
                            vd2 = Vec3(x2, y2, -depth),
                            vd3 = Vec3(x3, y3, -depth);

                    g->addTriangle(v1, vd1, v2);
                    g->addTriangle(v2, vd1, vd2);
                    g->addTriangle(v2, vd2, v3);
                    g->addTriangle(v3, vd2, vd3);
                    g->addTriangle(v3, vd1, v1);
                    g->addTriangle(v3, vd3, vd1);

                    g->addTriangle(vd1, vd3, vd2);
                }

            }
        }

        x += (c.size.width()) * 1.1;
    }

}


} // namespace GEOM
} // namespace MO
