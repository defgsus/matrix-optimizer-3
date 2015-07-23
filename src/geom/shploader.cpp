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

void ShpLoader::loadFile(const QString &filename, std::function<void(double)> progressFunc)
{
    p_->loadFile(filename, progressFunc);
}

void ShpLoader::getGeometry(Geometry * g, std::function<void(double)> progressFunc) const
{
    p_->getGeometry(g, progressFunc);
}

void ShpLoader::Private::getShpObject(SHPObject * shp)
{
    Tesselator tess;

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

void ShpLoader::Private::getPolyPart(SHPObject * shp, int start, int end, bool hasZ)
{
    QVector<Vec2> points;
    for (int j=start; j < end; ++j)
    {
        points << Vec2(shp->padfX[j], shp->padfY[j]);
//                        hasZ ? shp->padfZ[j] : 0.f);
    }

    Tesselator tess;
    tess.tesselate(points);
    tess.getGeometry(*geometry);
}

void ShpLoader::getGeometry(const QString &filename, Geometry * g, std::function<void(double)> progressFunc)
{
    QMutexLocker lock(&Private::mutex);

    auto i = Private::instances.find(filename);

    // reuse
    if (i != Private::instances.end())
    {
        i->second->getGeometry(g);
        return;
    }

    // create and load
    ShpLoader * loader = new ShpLoader();
    loader->loadFile(filename, progressFunc);
    loader->getGeometry(g, progressFunc);

    // store for later
    Private::instances.insert(std::make_pair(filename, loader));
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
}

void ShpLoader::Private::getGeometry(Geometry * geom, std::function<void(double)> progressFunc)
{
    if (!geometry)
        return;

    geom->addGeometry(*geometry);
}


} // namespace GEOM
} // namespace MO

#endif // #ifndef MO_DISABLE_SHP
