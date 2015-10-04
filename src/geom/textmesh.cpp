
#include <QPainterPath>

#include "textmesh.h"
#include "geometry.h"
#include "types/properties.h"


namespace MO {
namespace GEOM {

struct TextMesh::Private
{
    Private(TextMesh * p) : p(p)
    {

    }

    void createProps();
    void getGeometry(Geometry * g) const;

    TextMesh * p;
    Properties props;
    QFont font;
    QString text;
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
    p_->font = f;
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
}

void TextMesh::Private::getGeometry(Geometry *g) const
{
    QPainterPath path;

    path.addText(0, 0, font, props.get("text").toString());
    auto polys = path.toFillPolygons();

    int mode = props.get("mode").toInt();
    if (mode == M_POLYLINE)
    {
        for (auto & p : polys)
            for (int i=1; i<p.size(); ++i)
            {
                auto p1 = g->addVertex(p[i-1].x(), -p[i-1].y(), 0),
                     p2 = g->addVertex(p[i].x(), -p[i].y(), 0);
                g->addLine(p1, p2);
            }
        return;
    }

}



} // namespace GEOM
} // namespace MO
