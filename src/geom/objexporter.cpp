/** @file objexporter.cpp

    @brief Wavefront .obj exporter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include <QFile>
#include <QTextStream>
#include <QMap>

#include "objexporter.h"
#include "io/error.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

namespace
{
    struct Hash2
    {
        Float x,y;
        Hash2(Float x, Float y) : x(x), y(y) { }

        bool operator < (const Hash2& r) const
        {
            if (y == r.y)
                return x < r.x;
            return y < r.y;
        }
    };

    typedef QMap<Hash2, uint> Map2;

    struct Hash3
    {
        Float x,y,z;
        Hash3(Float x, Float y, Float z) : x(x), y(y), z(z) { }

        bool operator < (const Hash3& r) const
        {
            if (z == r.z)
            {
                if (y == r.y)
                    return x < r.x;
                return y < r.y;
            }
            return z < r.z;
        }
    };


    typedef QMap<Hash3, uint> Map3;

}



ObjExporter::ObjExporter()
    : options_      (EO_ALL)
{
}


void ObjExporter::setOption(ExportOption option, bool enable)
{
    if (enable)
        options_ |= option;
    else
        options_ &= ~option;
}


void ObjExporter::exportGeometry(const QString &filename, const Geometry * geo)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        MO_IO_ERROR(WRITE, "could not create OBJ file '" << filename << "'");
    }

    QTextStream io(&file);

    io << "# exported from Matrix Optimizer III\n"
       << "# http://modular-audio-graphics.com\n\n";

    Map3 vertexMap, normalMap;
    Map2 texMap;

    // --- write vertices ---

    // create vertex map
    const Geometry::VertexType * verts = geo->vertices();
    for (uint i=0; i<geo->numVertices(); ++i, verts += geo->numVertexComponents())
    {
        vertexMap.insert(Hash3(verts[0], verts[1], verts[2]), 0);
    }

    io << "# " << vertexMap.size() << " vertices";
    if (vertexMap.size() != (int)geo->numVertices())
        io << " (originally " << geo->numVertices() << ")";
    io << "\n";

    // write map
    int k=0;
    for (auto i = vertexMap.begin(); i!=vertexMap.end(); ++i, ++k)
    {
        io << "v " << i.key().x << " " << i.key().y << " " << i.key().z << "\n";
        // store order
        i.value() = k;
    }

    // --- write normals ---

    if (options_ & EO_NORMALS)
    {
        // create normal map
        const Geometry::NormalType * norms = geo->normals();
        for (uint i=0; i<geo->numVertices(); ++i, norms += geo->numNormalComponents())
        {
            normalMap.insert(Hash3(norms[0], norms[1], norms[2]), 0);
        }

        io << "\n# " << normalMap.size() << " normals\n";

        // write map
        int k=0;
        for (auto i = normalMap.begin(); i!=normalMap.end(); ++i, ++k)
        {
            io << "vn " << i.key().x
               << " " << i.key().y
               << " " << i.key().z << "\n";
            // save the order
            i.value() = k;
        }
    }

    // --- write tex coords ---

    if (options_ & EO_TEX_COORDS)
    {
        // create tex map
        const Geometry::TextureCoordType * texs = geo->textureCoords();
        for (uint i=0; i<geo->numVertices(); ++i,
                texs += geo->numTextureCoordComponents())
        {
            texMap.insert(Hash2(texs[0], texs[1]), 0);
        }

        io << "\n# " << texMap.size() << " texture coordinates\n";

        // write map
        int k=0;
        for (auto i = texMap.begin(); i!=texMap.end(); ++i, ++k)
        {
            io << "vt " << i.key().x << " " << i.key().y << "\n";
            // save order
            i.value() = k;
        }
    }


    // --- write triangles ---

    io << "\n# " << geo->numTriangles() << " triangles\n";

    const Geometry::IndexType * tris = geo->triangleIndices();

    for (uint i=0; i<geo->numTriangles(); ++i, tris += geo->numTriangleIndexComponents())
    {
        int v1,t1,n1, v2,t2,n2, v3,t3,n3;

        v1 = tris[0];
        v2 = tris[1];
        v3 = tris[2];

        // get indices into normal map
        if (options_ & EO_NORMALS)
        {
            const Geometry::NormalType * norms =
                    &geo->normals()[v1 * geo->numNormalComponents()];
            auto i = normalMap.find(Hash3(norms[0], norms[1], norms[2]));
            MO_ASSERT(i != normalMap.end(), "duh?");
            n1 = i.value() + 1;

            norms = &geo->normals()[v2 * geo->numNormalComponents()];
            i = normalMap.find(Hash3(norms[0], norms[1], norms[2]));
            MO_ASSERT(i != normalMap.end(), "duh?");
            n2 = i.value() + 1;

            norms = &geo->normals()[v3 * geo->numNormalComponents()];
            i = normalMap.find(Hash3(norms[0], norms[1], norms[2]));
            MO_ASSERT(i != normalMap.end(), "duh?");
            n3 = i.value() + 1;
        }

        // get indices into tex-coord map
        if (options_ & EO_TEX_COORDS)
        {
            const Geometry::TextureCoordType * texs =
                    &geo->textureCoords()[v1 * geo->numTextureCoordComponents()];
            auto i = texMap.find(Hash2(texs[0], texs[1]));
            MO_ASSERT(i != texMap.end(), "duh?");
            t1 = i.value() + 1;

            texs = &geo->textureCoords()[v2 * geo->numTextureCoordComponents()];
            i = texMap.find(Hash2(texs[0], texs[1]));
            MO_ASSERT(i != texMap.end(), "duh?");
            t2 = i.value() + 1;

            texs = &geo->textureCoords()[v3 * geo->numTextureCoordComponents()];
            i = texMap.find(Hash2(texs[0], texs[1]));
            MO_ASSERT(i != texMap.end(), "duh?");
            t3 = i.value() + 1;
        }

        // get indices from vertex map
        const Geometry::VertexType * verts =
                &geo->vertices()[v1 * geo->numVertexComponents()];
        auto j = vertexMap.find(Hash3(verts[0], verts[1], verts[2]));
        MO_ASSERT(j != vertexMap.end(), "duh?");
        v1 = j.value() + 1;

        verts = &geo->vertices()[v2 * geo->numVertexComponents()];
        j = vertexMap.find(Hash3(verts[0], verts[1], verts[2]));
        MO_ASSERT(j != vertexMap.end(), "duh?");
        v2 = j.value() + 1;

        verts = &geo->vertices()[v3 * geo->numVertexComponents()];
        j = vertexMap.find(Hash3(verts[0], verts[1], verts[2]));
        MO_ASSERT(j != vertexMap.end(), "duh?");
        v3 = j.value() + 1;

        // write the face line

        if ((options_ & EO_TEX_COORDS) && (options_ & EO_NORMALS))
            io << "f " << v1 << "/" << t1 << "/" << n1
               << " "  << v2 << "/" << t2 << "/" << n2
               << " "  << v3 << "/" << t3 << "/" << n3 << "\n";
        else
        if (options_ & EO_TEX_COORDS)
            io << "f " << v1 << "/" << t1
               << " "  << v2 << "/" << t2
               << " "  << v3 << "/" << t3 << "\n";
        else
        if (options_ & EO_NORMALS)
            io << "f " << v1 << "//" << n1
               << " "  << v2 << "//" << n2
               << " "  << v3 << "//" << n3 << "\n";
        else
            io << "f " << v1 << " " << v2 << " " << v3 << "\n";
    }

}




} // namespace GEOM
} // namespace MO
