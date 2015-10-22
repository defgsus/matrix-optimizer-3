/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2015</p>
*/

#ifndef MO_DISABLE_SHP

#include <shapefil.h>

#include <QMutex>
#include <QMutexLocker>
#include <QPolygonF>

#include "shploader.h"
#include "geometry.h"
#include "tesselator.h"
#include "io/error.h"

namespace MO {
namespace GEOM {

struct ShpLoader::Private
{
    Private(ShpLoader * loader)
        : loader    (loader)
        , handle    (0)
        , progress  (0)
        , geometry  (0)
        , triMesh   (TM_NONE)
        , meshSpace (10.)
    {

    }

    ~Private()
    {
        close();
        if (geometry)
            geometry->releaseRef();
    }

    struct Vertex
    {
        float x, y;
    };
    struct VertexGroup
    {
        std::vector<Vertex> vertices;
    };

    void close();
    void loadFile(const QString &filename, std::function<void(double)> progressFunc = 0);
    void getShpObject(SHPObject * );
    void getPolyPart(SHPObject *, int start, int end, bool hasZ);
    void getGeometry(Geometry * g, std::function<void(double)> progressFunc = 0);

    ShpLoader * loader;
    SHPHandle handle;

    double progress;
    GEOM::Geometry * geometry;
    TriangulationMesh triMesh;
    DVec2 meshSpace;

    QVector<QPolygonF> polygons;

    static QMutex mutex;
    static std::map<QString, ShpLoader*> instances;
};

QMutex ShpLoader::Private::mutex;
std::map<QString, ShpLoader*> ShpLoader::Private::instances;


ShpLoader::ShpLoader()
    : p_        (new Private(this))
{

}

ShpLoader::~ShpLoader()
{
    delete p_;
}

double ShpLoader::progress() const { return p_->progress; }

void ShpLoader::setTriangulationMesh(TriangulationMesh m)
{
    p_->triMesh = m;
}

void ShpLoader::setTriangulationMeshSpacing(float x, float y)
{
    p_->meshSpace = DVec2(x, y);
}

void ShpLoader::loadFile(const QString &filename, std::function<void(double)> progressFunc)
{
    p_->loadFile(filename, progressFunc);
}

void ShpLoader::getGeometry(Geometry * g, std::function<void(double)> progressFunc) const
{
    p_->getGeometry(g, progressFunc);
}

void ShpLoader::getPolygons(QVector<QPolygonF> & poly)
{
    poly = p_->polygons;
}

QRectF ShpLoader::getBoundingRect() const
{
    QRectF rect;
    for (const QPolygonF & polygon : p_->polygons)
    {
        rect = rect.united( polygon.boundingRect() );
    }

    return rect;
}

void ShpLoader::Private::getShpObject(SHPObject * shp)
{
    bool hasZ = false;
    switch (shp->nSHPType)
    {
        case SHPT_POINTZ:
            hasZ = true;
        case SHPT_POINT:
        case SHPT_POINTM:
            for (int j=0; j < shp->nVertices; ++j)
            {
                auto i0 = geometry->addVertex(shp->padfX[j], shp->padfY[j],
                                              hasZ ? shp->padfZ[j] : 0.f);
                geometry->addPoint(i0);
            }
        break;

        case SHPT_ARCZ:
            hasZ = true;
        case SHPT_ARC:
        case SHPT_ARCM:
            if (shp->nParts)
            {
                for (int k=0; k < shp->nParts; ++k)
                {
                    const int
                        start = shp->panPartStart[k],
                        end = k < shp->nParts - 1
                            ? shp->panPartStart[k+1] : shp->nVertices;
                    for (int j=start+1; j < end; ++j)
                    {
                        auto i0 = geometry->addVertex(shp->padfX[j-1], shp->padfY[j-1],
                                                hasZ ? shp->padfZ[j-1] : 0.f);
                        auto i1 = geometry->addVertex(shp->padfX[j], shp->padfY[j],
                                                hasZ ? shp->padfZ[j] : 0.f);
                        geometry->addLine(i0, i1);
                    }
                }
            }
            else
            {
                for (int j=1; j < shp->nVertices; ++j)
                {
                    auto i0 = geometry->addVertex(shp->padfX[j-1], shp->padfY[j-1],
                                                  hasZ ? shp->padfZ[j-1] : 0.f);
                    auto i1 = geometry->addVertex(shp->padfX[j], shp->padfY[j],
                                                  hasZ ? shp->padfZ[j] : 0.f);
                    geometry->addLine(i0, i1);
                }
            }
        break;

        case SHPT_POLYGONZ:
            hasZ = true;
        case SHPT_POLYGON:
        case SHPT_POLYGONM:
            if (shp->nParts)
            {
                for (int k=0; k < shp->nParts; ++k)
                {
                    const int
                        start = shp->panPartStart[k],
                        end = k < shp->nParts - 1
                            ? shp->panPartStart[k+1] : shp->nVertices;
                    getPolyPart(shp, start, end, hasZ);
                }
            }
            else
            {
                for (int j=0; j < shp->nVertices; ++j)
                {
                    int j1 = (j+1) % shp->nVertices;
                    auto i0 = geometry->addVertex(shp->padfX[j], shp->padfY[j],
                                            hasZ ? shp->padfZ[j] : 0.f);
                    auto i1 = geometry->addVertex(shp->padfX[j1], shp->padfY[j1],
                                            hasZ ? shp->padfZ[j1] : 0.f);
                    geometry->addLine(i0, i1);
                }
            }
        break;
    }
}

void ShpLoader::Private::getPolyPart(SHPObject * shp, int start, int end, bool /*hasZ*/)
{
    QVector<DVec2> points;
    QPolygonF polygon;
    const int len = end - start;
    for (int j=0; j < len; ++j)
    {
        points << DVec2(shp->padfX[start+len-1-j], shp->padfY[start+len-1-j]);
//                        hasZ ? shp->padfZ[j] : 0.f);
        polygon << QPointF(shp->padfX[start+j], shp->padfY[start+j]);
    }
    polygons << polygon;

    Tesselator tess;
    if (triMesh > 0)
    {
        DVec2 minEx, maxEx;
        Tesselator::getExtend(points, minEx, maxEx);
        tess.createTriangulationMesh(minEx, maxEx, meshSpace);
    }
    tess.tesselate(points);
    tess.getGeometry(*geometry);
}

void ShpLoader::getGeometry(const QString &filename, Geometry * g, std::function<void(double)> progressFunc)
{
    getGeometry(filename, g, TM_NONE, 10., 10., progressFunc);
}

void ShpLoader::getGeometry(const QString &filename, Geometry * g,
                            TriangulationMesh triMesh, float triMeshX, float triMeshY, std::function<void(double)> progressFunc)
{
    QString key = filename + QString("_%1_%2_%3")
                             .arg(triMesh).arg(triMeshX).arg(triMeshY);
    {
        QMutexLocker lock(&Private::mutex);

        auto i = Private::instances.find(key);

        // reuse
        if (i != Private::instances.end())
        {
            i->second->getGeometry(g);
            return;
        }
    }

    // create and load
    ShpLoader * loader = new ShpLoader();
    loader->setTriangulationMesh(triMesh);
    loader->setTriangulationMeshSpacing(triMeshX, triMeshY);
    loader->loadFile(filename, progressFunc);
    loader->getGeometry(g, progressFunc);

    // store for later
    QMutexLocker lock(&Private::mutex);
    Private::instances.insert(std::make_pair(key, loader));
}


void ShpLoader::Private::loadFile(const QString &filename, std::function<void(double)> progressFunc)
{
    if (handle)
        close();

    handle = SHPOpen(filename.toStdString().c_str(), "rb");
    if (!handle)
        MO_IO_ERROR(READ, "Could not open shapefile " << filename << "'");

    // -- get vertex data --

    int numEntities;
    SHPGetInfo(handle, &numEntities, 0, 0, 0);

    polygons.clear();

    if (!geometry)
        geometry = new GEOM::Geometry();
    geometry->clear();
    geometry->setColor(.5, .5, .5, 1.);

    for (int i=0; i<numEntities; ++i)
    {
        SHPObject * shp = SHPReadObject(handle, i);
        if (!shp)
            continue;

        getShpObject(shp);

        progress = (double)i / numEntities * 100.;
        if (progressFunc)
            progressFunc(progress);

        SHPDestroyObject(shp);
    }

    close();
}

void ShpLoader::Private::close()
{
    if (handle)
        SHPClose(handle);
    handle = 0;
}

void ShpLoader::Private::getGeometry(
        Geometry * geom, std::function<void(double)> /*progressFunc*/)
{
    if (!geometry)
        return;

    geom->addGeometry(*geometry);
}


} // namespace GEOM
} // namespace MO

#endif // #ifndef MO_DISABLE_SHP
