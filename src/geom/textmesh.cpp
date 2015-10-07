#include <map>

#include <QList>
#include <QPainterPath>
#include <QTransform>
#include <QFontMetrics>
#include <QFontInfo>

#include "3rd/polypartition/src/polypartition.h"

#include "textmesh.h"
#include "geometry.h"
#include "types/properties.h"
#include "io/error.h"

#if 0
#   include <QDebug>
#   include "io/log.h"
#   define MO_DEBUG_TM(arg__) MO_PRINT("TextMesh::" << arg__)
#else
#   define MO_DEBUG_TM(unused__)
#endif

namespace MO {
namespace GEOM {

struct TextMesh::Private
{
    struct PolyLoop
    {
        int start, len;
    };

    struct Char
    {
        QList<QPolygonF> polys;
        QRectF rect;
        std::list<TPPLPoly> tris;
        qreal x;
        // for each final poly point (loop-point)
        // stores the start index and the length of the poly
        std::map<int, PolyLoop> loopMap;
    };

    Private(TextMesh * p) : p(p)
    {

    }

    void createProps();

    static void removeCurves(const QPainterPath& in, QPainterPath& out);
    static bool isNext(const Char& c, int i1, int i2);
    void getPolys();
    void getPolys(const QPainterPath& path, QList<QPolygonF>& polys);
    void getOffset(qreal& x, qreal& y);
    void triangulate(Char&);
    void createLines(Geometry * g, bool shared);
    void createTrianglesShared(Geometry * g);
    void createTrianglesUnshared(Geometry * g);
    void getGeometry(Geometry * g, bool shared);

    TextMesh * p;
    Properties props;

    QMap<QChar, Char> polyMap;
    QVector<qreal> xOffset;
    QRectF boundingRect;
    qreal descent;
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

void TextMesh::getGeometry(Geometry *g, bool shared_vertices) const
{
    p_->getGeometry(g, shared_vertices);
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

    Properties::NamedValues alignH;
    alignH.set("left", QObject::tr("left"), QObject::tr("Align on the left side"), (int)Qt::AlignLeft);
    alignH.set("center", QObject::tr("center"), QObject::tr("Align on the center"), (int)Qt::AlignCenter);
    alignH.set("right", QObject::tr("right"), QObject::tr("Align on the right side"), (int)Qt::AlignRight);
    props.set("align_h", QObject::tr("alignment horizontal"),
            QObject::tr("Selects horizontal alignment for the text"), alignH, (int)Qt::AlignCenter);

    Properties::NamedValues alignV;
    alignV.set("top", QObject::tr("top"), QObject::tr("Align on the top"), (int)Qt::AlignTop);
    alignV.set("center", QObject::tr("center"), QObject::tr("Align on the center"), (int)Qt::AlignCenter);
    alignV.set("bottom", QObject::tr("bottom"), QObject::tr("Align on the bottom"), (int)Qt::AlignBottom);
    alignV.set("baseline", QObject::tr("baseline"),
               QObject::tr("Align on the baseline of the font"), (int)Qt::AlignBaseline);
    props.set("align_v", QObject::tr("alignment vertical"),
            QObject::tr("Selects vertical alignment for the text"), alignV, (int)Qt::AlignBottom);

    Properties::NamedValues triMode;
    triMode.set("ec", QObject::tr("ear clipping"),
               QObject::tr("Low-cost ear clipping method"), 0);
    triMode.set("opt", QObject::tr("optimal"),
               QObject::tr("Optimal triangulation in terms of minimal edge length"), 1);
    props.set("tri_method", QObject::tr("triangulation method"),
            QObject::tr("The method used for triangulation"), triMode, 0);

    props.setUpdateVisibilityCallback([](Properties&p)
    {
        int mode = p.get("mode").toInt();
        p.setVisible("tri_method", mode == M_TESS);
    });
}

void TextMesh::Private::getGeometry(Geometry *g, bool shared)
{
    getPolys();

    int mode = props.get("mode").toInt();
    if (mode == M_POLYLINE)
    {
        createLines(g, shared);
        return;
    }

    for (auto & c : polyMap)
        triangulate(c);

    if (mode == M_TESS)
    {
        if (shared)
            createTrianglesShared(g);
        else
            createTrianglesUnshared(g);
        return;
    }

}

void TextMesh::Private::removeCurves(const QPainterPath &in, QPainterPath &out)
{
    out.moveTo(in.pointAtPercent(0.));
    qreal l_ang = in.angleAtPercent(0.);
    qreal st = 0.1;

    for (qreal t = 0.; t<=1.;)
    {
    again_:
        qreal ang = in.angleAtPercent(t);
        bool diff = std::abs(ang - l_ang) > 10.;

        if (diff && st > 0.001)
        {
            st /= 2;
            t -= st;
            goto again_;
        }
        else if (st < 0.01)
            st *= 2.;

        l_ang = ang;

        out.lineTo(in.pointAtPercent(t));

        t += st;
        //t += std::max(qreal(0.001), 0.01 - 0.1*std::abs(in.slopeAtPercent(t)));
    }

}

void TextMesh::Private::getPolys(const QPainterPath &path, QList<QPolygonF> &polys)
{
    QPolygonF poly;
    // get first outline
    int i;
    for (i=0; i<path.elementCount(); ++i)
    {
        auto e = path.elementAt(i);
        if (e.isMoveTo() && i != 0)
            break;

        poly << QPointF(e);
    }
    polys << poly;
    // additional polygons?
    if (i < path.elementCount() - 1)
    {
        poly.clear();

        for (; i<path.elementCount(); ++i)
        {
            auto e = path.elementAt(i);
            if (e.isMoveTo() && !poly.empty())
            {
                polys << poly;
                poly.clear();
            }
            poly << QPointF(e);
        }
        if (poly.size() > 2)
            polys << poly;
    }
}

void TextMesh::Private::getPolys()
{
    MO_DEBUG_TM("getPolys()");

    polyMap.clear();
    xOffset.clear();

    const QString text = props.get("text").toString();
    QFont font = props.get("font").value<QFont>();
    font.setStyleStrategy(QFont::PreferOutline);
    QFontMetrics fm(font);
    //QFontInfo fi(font);
    //qreal pfac = qreal(fm.height()) / fi.pointSizeF();
    descent = fm.descent();

    int k=0;
    for (auto chr : text)
    {
        // store x-offset
        xOffset.append( fm.width(text, k++) );

        // don't create duplicates
        {
            auto i = polyMap.find(chr);
            if (i != polyMap.end())
                continue;
        }

        // get the painterpath of each char
        QPainterPath path;
        path.addText(0, 0, font, chr);

        MO_DEBUG_TM("QPainterPath.elementCount() == " << path.elementCount());

        // create the poly struct
        Char c;

        // flip y
        QTransform trans;
        // XXX changes winding order and breaks triangulation
        //trans.scale(1., -1.);

        // make curves be made of many lines
        //QPainterPath spath;
        //removeCurves(path, spath);

        // get list of polygons
        //c.polys = spath.simplified().toSubpathPolygons(trans);
        getPolys(path, c.polys);

        // debugging
        MO_DEBUG_TM(c.polys.size() << " polys:");
        for (const QPolygonF& p : c.polys)
        {
            Q_UNUSED(p);
            MO_DEBUG_TM(p.size() << " points");
            //qInfo() << p.poly;
        }

        // get extend
        c.rect = path.boundingRect();

        // remember
        polyMap.insert(chr, c);
    }

    // -- get bounding rect --

    boundingRect = QRectF();

    if (polyMap.isEmpty())
        return;

    k = 0;
    for (auto chr : text)
    {
        auto i = polyMap.find(chr);
        MO_ASSERT(i != polyMap.end(), "");

        QRectF r = i.value().rect;
        r.moveRight(r.right() + xOffset[k++]);
        boundingRect = boundingRect.united(r);
    }
    // XXX Bounding rect is +x, +y,
    // while char polygons are still +x, -y !!
    boundingRect.setTop(-boundingRect.top());
    boundingRect.setBottom(-boundingRect.bottom());
}

void TextMesh::Private::getOffset(qreal &x, qreal &y)
{
    int ah = props.get("align_h").toInt(),
        av = props.get("align_v").toInt();
    switch (ah)
    {
        default:
        case Qt::AlignLeft: x = -boundingRect.left(); break;
        case Qt::AlignRight: x = -boundingRect.right(); break;
        case Qt::AlignCenter: x = -boundingRect.center().x(); break;
    }
    switch (av)
    {
        default:
        case Qt::AlignTop: y = -boundingRect.top(); break;
        case Qt::AlignBottom: y = -boundingRect.bottom(); break;
        case Qt::AlignBaseline: y = -boundingRect.bottom() - descent; break;
        case Qt::AlignCenter: y = -boundingRect.center().y(); break;
    }
}


void TextMesh::Private::triangulate(Char & c)
{
    std::list<TPPLPoly> inp;

    // convert to TPPLPoly
    int idx = 0;
    for (const QPolygonF& poly : c.polys)
    {
        // avoid duplicate points for TPPL lib
        const int si = poly.isClosed() ? poly.size() - 1
                                       : poly.size();

        // store info about single polys and their loop-points
        PolyLoop pl;
        pl.start = idx;
        pl.len = si;
        c.loopMap.insert(std::make_pair(idx + si - 1, pl));

        // create the poly and copy pos and index
        TPPLPoly tp;
        tp.Init(si);
        for (int i=0; i<si; ++i)
            tp.GetPoint(i) = TPPLPoint(poly[i].x(), poly[i].y(), idx++);

        // holes are CW order
        tp.SetHole(tp.GetOrientation() == TPPL_CW);

        inp.push_back(tp);
    }

    // triangulate

    TPPLPartition part;

    std::list<TPPLPoly> tris;
    if (props.get("tri_method").toInt() == 0)
        part.Triangulate_EC(&inp, &tris);
    else
        part.Triangulate_OPT(&inp, &tris);

    // get triangles
    c.tris.clear();
    for (TPPLPoly & tri : tris)
    {
        MO_ASSERT(tri.GetNumPoints() == 3,
                  "illegal polygon from triangulation ("
                            << tri.GetNumPoints() << " vertices)");

        if (tri.GetNumPoints() != 3)
            continue;

        // fix orientation
        //   change it to CW...
        if (tri.GetOrientation() == TPPL_CCW)
        {
            std::swap(tri.GetPoint(1), tri.GetPoint(2));
        }
        // ... and flip y, so we get CCW again
        tri.GetPoint(0).y *= -1.;
        tri.GetPoint(1).y *= -1.;
        tri.GetPoint(2).y *= -1.;

        c.tris.push_back( tri );
    }
}

void TextMesh::Private::createLines(Geometry *g, bool shared)
{
    const QString text = props.get("text").toString();

    qreal xo, yo;
    getOffset(xo, yo);

    int k=0;
    for (auto chr : text)
    {
        auto mapi = polyMap.find(chr);
        if (mapi == polyMap.end())
            continue;

        const Char& c = mapi.value();
        qreal x = xOffset[k++] + xo, y = yo;

        for (const QPolygonF& poly : c.polys)
        {
            if (poly.empty())
                continue;

            g->setNormal(0,0,1);

            if (shared)
            {
                QVector<Geometry::IndexType> idx;
                for (auto & p : poly)
                    idx << g->addVertex(p.x() + x, -p.y() + y, 0.f);

                for (int i=0; i<idx.size()-1; ++i)
                {
                    g->addLine(idx[i], idx[i+1]);
                }
            }
            else
            {
                for (int i=0; i<poly.size()-1; ++i)
                {
                    g->addLine(Vec3(poly[i].x() + x, -poly[i].y() + y, 0.f),
                               Vec3(poly[i+1].x() + x, -poly[i+1].y() + y, 0.f));
                }
            }
        }
    }
#if 0
    // XXX debug boundingRect
    g->addLine(Vec3(boundingRect.left(), boundingRect.top(), 0.),
               Vec3(boundingRect.right(), boundingRect.top(), 0.));
    g->addLine(Vec3(boundingRect.left(), boundingRect.top(), 0.),
               Vec3(boundingRect.left(), boundingRect.bottom(), 0.));
#endif
}

bool TextMesh::Private::isNext(const Char& c, int i1, int i2)
{
    if ((i1 + 1) == i2 || (i2 + 1) == i1)
        return true;
    auto l = c.loopMap.find(i1);
    if (l != c.loopMap.end() && i2 == l->second.start)
        return true;
    l = c.loopMap.find(i2);
    if (l != c.loopMap.end() && i1 == l->second.start)
        return true;
    return false;
}

void TextMesh::Private::createTrianglesShared(Geometry *g)
{
    const QString text = props.get("text").toString();
    const Float depth = props.get("depth").toFloat();

    qreal xo, yo;
    getOffset(xo, yo);

    int k=0;
    for (auto chr : text)
    {
        auto mapi = polyMap.find(chr);
        if (mapi == polyMap.end())
            continue;

        const Char& c = mapi.value();
        // char offset + offset from alignment
        qreal x = xOffset[k++] + xo, y = yo;

        // get vertices
        std::map<int, int> indexMap;
        for (const TPPLPoly& p : c.tris)
        {
            if (p.GetNumPoints() < 3)
                continue;
            for (int j = 0; j<3; ++j)
            {
                const int idx = p.GetPoint(j).index;
                const Float
                        x1 = p.GetPoint(j).x + x,
                        y1 = p.GetPoint(j).y + y;

                // only create points once
                if (indexMap.find(idx) == indexMap.end())
                {
                    // add vertex and remember index
                    indexMap.insert(std::make_pair(idx, (int)g->addVertex(x1, y1, 0)));
                    // unique index for back-side
                    if (depth > 0.f)
                        indexMap.insert(std::make_pair(-idx-1, (int)g->addVertex(x1, y1, -depth)));
                }
            }
        }

        // connect triangles
        for (const TPPLPoly& p : c.tris)
        {
            if (p.GetNumPoints() < 3)
                continue;

            // original index
            const int
                    i1 = p.GetPoint(0).index,
                    i2 = p.GetPoint(1).index,
                    i3 = p.GetPoint(2).index,
            // index in geometry
                    v1 = indexMap.find(i1)->second,
                    v2 = indexMap.find(i2)->second,
                    v3 = indexMap.find(i3)->second;
            // see if consecutive in original polygon
            const bool
                    isNext1 = isNext(c, i1, i2),
                    isNext2 = isNext(c, i2, i3),
                    isNext3 = isNext(c, i3, i1);

            g->addTriangle(v1, v2, v3);

            if (depth > 0.f)
            {
                const int
                        vd1 = indexMap.find(-i1-1)->second,
                        vd2 = indexMap.find(-i2-1)->second,
                        vd3 = indexMap.find(-i3-1)->second;

                // extruded outline
                if (isNext1)
                {
                    g->addTriangle(v1, vd1, v2);
                    g->addTriangle(v2, vd1, vd2);
                }
                if (isNext2)
                {
                    g->addTriangle(v2, vd2, v3);
                    g->addTriangle(v3, vd2, vd3);
                }
                if (isNext3)
                {
                    g->addTriangle(v3, vd1, v1);
                    g->addTriangle(v3, vd3, vd1);
                }

                // back
                g->addTriangle(vd1, vd3, vd2);
            }
        }
    }

}



void TextMesh::Private::createTrianglesUnshared(Geometry *g)
{
    const QString text = props.get("text").toString();
    const Float depth = props.get("depth").toFloat();

    qreal xo, yo;
    getOffset(xo, yo);

    int k=0;
    for (auto chr : text)
    {
        auto mapi = polyMap.find(chr);
        if (mapi == polyMap.end())
            continue;

        const Char& c = mapi.value();
        qreal x = xOffset[k++] + xo, y = yo;

        for (const TPPLPoly& p : c.tris)
        {
            if (p.GetNumPoints() < 3)
                continue;

            const Float
                    x1 = p.GetPoint(0).x + x,
                    x2 = p.GetPoint(1).x + x,
                    x3 = p.GetPoint(2).x + x,
                    y1 = p.GetPoint(0).y + y,
                    y2 = p.GetPoint(1).y + y,
                    y3 = p.GetPoint(2).y + y;
            const int
                    i1 = p.GetPoint(0).index,
                    i2 = p.GetPoint(1).index,
                    i3 = p.GetPoint(2).index;
            const bool
                    isNext1 = isNext(c, i1, i2),
                    isNext2 = isNext(c, i2, i3),
                    isNext3 = isNext(c, i3, i1);

            auto p1 = g->addVertex(x1, y1, 0),
                 p2 = g->addVertex(x2, y2, 0),
                 p3 = g->addVertex(x3, y3, 0);

            g->addTriangle(p1, p2, p3);

            if (depth > 0.f)
            {
                const Vec3
                        v1 =  Vec3(x1, y1, 0),
                        v2 =  Vec3(x2, y2, 0),
                        v3 =  Vec3(x3, y3, 0),
                        vd1 = Vec3(x1, y1, -depth),
                        vd2 = Vec3(x2, y2, -depth),
                        vd3 = Vec3(x3, y3, -depth);

                // extruded outline
                if (isNext1)
                {
                    g->addTriangle(v1, vd1, v2);
                    g->addTriangle(v2, vd1, vd2);
                }
                if (isNext2)
                {
                    g->addTriangle(v2, vd2, v3);
                    g->addTriangle(v3, vd2, vd3);
                }
                if (isNext3)
                {
                    g->addTriangle(v3, vd1, v1);
                    g->addTriangle(v3, vd3, vd1);
                }
                // back
                g->addTriangle(vd1, vd3, vd2);
            }
        }
    }

}


} // namespace GEOM
} // namespace MO
