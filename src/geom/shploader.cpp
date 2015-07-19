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
#include "io/error.h"

namespace MO {
namespace GEOM {

struct ShpLoader::Private
{
    Private(ShpLoader * loader)
        : loader    (loader)
        , handle    (0)
        , progress  (0)
    {

    }

    ~Private()
    {
        close();
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
    void getGeometry(Geometry * g, std::function<void(double)> progressFunc = 0);

    ShpLoader * loader;
    SHPHandle handle;

    double progress;
    std::vector<VertexGroup> vertexGroups;
    //GEOM::Geometry geometry;

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

    for (int i=0; i<numEntities; ++i)
    {
        SHPObject * shp = SHPReadObject(handle, i);
        if (!shp)
            continue;

        vertexGroups.push_back(VertexGroup());
        VertexGroup & vgroup = vertexGroups.back();

        // construct with lines
        for (int j=0; j < shp->nVertices; ++j)
        {
            Vertex v;
            v.x = shp->padfX[j];
            v.y = shp->padfY[j];
            vgroup.vertices.push_back(v);
        }

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
    if (!handle)
        return;

    for (size_t i=0; i<vertexGroups.size(); ++i)
    {
        const VertexGroup & vgroup = vertexGroups[i];

        Geometry::IndexType i0, i1;
        for (size_t j = 0; j<vgroup.vertices.size(); ++j)
        {
            i1 = geom->addVertex(vgroup.vertices[j].x,
                                 vgroup.vertices[j].y, 0);
            if (j>0)
                geom->addLine(i0, i1);
            i0 = i1;
        }

        progress = (double)i / vertexGroups.size() * 100.;
        if (progressFunc)
            progressFunc(progress);
    }

    /*int numEntities;
    SHPGetInfo(handle, &numEntities, 0, 0, 0);

    for (int i=0; i<numEntities; ++i)
    {
        SHPObject * shp = SHPReadObject(handle, i);
        if (!shp)
            continue;

        // construct with lines
        Geometry::IndexType i0, i1;
        for (int j=0; j < shp->nVertices; ++j)
        {
            i1 = geom->addVertex(shp->padfX[j], shp->padfY[j], 0);
            if (j>0)
                geom->addLine(i0, i1);
            i0 = i1;
        }

        SHPDestroyObject(shp);
    } */
}


} // namespace GEOM
} // namespace MO

#endif // #ifndef MO_DISABLE_SHP
