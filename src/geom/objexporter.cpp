/** @file objexporter.cpp

    @brief Wavefront .obj exporter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include <QFile>
#include <QTextStream>

#include "objexporter.h"
#include "io/error.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

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

    // --- write vertices ---

    io << "# " << geo->numVertices() << " vertices\n";

    const Geometry::VertexType * verts = geo->vertices();
    for (uint i=0; i<geo->numVertices(); ++i, verts += geo->numVertexComponents())
    {
        io << "v " << verts[0] << " " << verts[1] << " " << verts[2] << "\n";
    }

    // --- write normals ---

    if (options_ & EO_NORMALS)
    {
        const Geometry::NormalType * norms = geo->normals();
        for (uint i=0; i<geo->numVertices(); ++i, norms += geo->numNormalComponents())
        {
            io << "vn " << norms[0] << " " << norms[1] << " " << norms[2] << "\n";
        }
    }

    // --- write tex coords ---

    if (options_ & EO_TEX_COORDS)
    {
        const Geometry::TextureCoordType * texs = geo->textureCoords();
        for (uint i=0; i<geo->numVertices(); ++i,
                texs += geo->numTextureCoordComponents())
        {
            io << "vt " << texs[0] << " " << texs[1] << "\n";
        }
    }


    // --- write triangles ---

    io << "# " << geo->numTriangles() << " triangles\n";

    const Geometry::IndexType * tris = geo->triangleIndices();

    // export all
    if ((options_ & EO_TEX_COORDS) && (options_ & EO_NORMALS))
        for (uint i=0; i<geo->numTriangles(); ++i,
            tris += geo->numTriangleIndexComponents())
                io << "f " << (tris[0]+1) << "/" << (tris[0]+1) << "/" << (tris[0]+1)
                   << " " <<  (tris[1]+1) << "/" << (tris[1]+1) << "/" << (tris[1]+1)
                   << " " <<  (tris[2]+1) << "/" << (tris[2]+1) << "/" << (tris[2]+1) << "\n";

    else
    // normals
    if ((options_ & EO_NORMALS))
        for (uint i=0; i<geo->numTriangles(); ++i,
             tris += geo->numTriangleIndexComponents())
                io << "f " << (tris[0]+1) << "//" << (tris[0]+1)
                   << " " <<  (tris[1]+1) << "//" << (tris[1]+1)
                   << " " <<  (tris[2]+1) << "//" << (tris[2]+1) << "\n";

    else
    // tex-coords only (XXX is this legal?)
    if ((options_ & EO_TEX_COORDS))
        for (uint i=0; i<geo->numTriangles(); ++i,
             tris += geo->numTriangleIndexComponents())
                io << "f " << (tris[0]+1) << "/" << (tris[0]+1)
                   << " " <<  (tris[1]+1) << "/" << (tris[1]+1)
                   << " " <<  (tris[2]+1) << "/" << (tris[2]+1) << "\n";


}




} // namespace GEOM
} // namespace MO
