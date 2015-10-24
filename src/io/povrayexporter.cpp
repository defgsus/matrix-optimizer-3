/** @file povrayexporter.cpp

    @brief Exporter of Scene to Povray script

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include <QString>
#include <QFile>
#include <QTextStream>

#include "povrayexporter.h"
#include "io/error.h"
#include "object/scene.h"
#include "object/visual/lightsource.h"
#include "object/visual/model3d.h"
#include "object/visual/camera.h"
#include "geom/geometry.h"

namespace MO {
namespace IO {

namespace
{
    QString povStr(const Vec2& v)
    {
        return QString("<%1, %2>").arg(v[0]).arg(v[1]);
    }

    QString povStr(const Vec3& v)
    {
        return QString("<%1, %2, %3>").arg(v[0]).arg(v[1]).arg(v[2]);
    }

    QString povStr(const Vec4& v)
    {
        return QString("<%1, %2, %3, %4>").arg(v[0]).arg(v[1]).arg(v[2]).arg(v[3]);
    }

    QString povRgbf(const Vec4& v)
    {
        return QString("<%1, %2, %3, %4>").arg(v[0]).arg(v[1]).arg(v[2]).arg(1-v[3]);
    }

    QString povStr(const Mat4& m)
    {
        return QString("<%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12>")
                .arg(m[0][0]).arg(m[0][1]).arg(m[0][2])
                .arg(m[1][0]).arg(m[1][1]).arg(m[1][2])
                .arg(m[2][0]).arg(m[2][1]).arg(m[2][2])
                .arg(m[3][0]).arg(m[3][1]).arg(m[3][2]);

    }
}


PovrayExporter::PovrayExporter()
    :   thread_ (MO_GFX_THREAD)
{
}

void PovrayExporter::setScene(const Scene * scene)
{
    scene_ = scene;
}

void PovrayExporter::exportScene(const QString &filename, Double time) const
{
    QFile f(filename);
    if (!f.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not create povray file '" << filename << "'\n"
                    << f.errorString());

    QTextStream io(&f);
    exportScene(io, time);
}

void PovrayExporter::exportScene(QTextStream & out, Double time) const
{
    out << "/************************\n"
        << " * MATRIX OPTIMIZER III *\n"
        << " ************************/\n\n";

    out << "#version 3.7;\n\n";

    exportCameras_(out, time);
    exportLights_(out, time);
    exportModels_(out, time);
}

void PovrayExporter::exportCameras_(QTextStream & out, Double) const
{
    out << "// ---------------- cameras ------------------\n\n";

    auto cams = scene_->findChildObjects<Camera>(QString(), true);

    int k = 0;
    for (Camera * c : cams)
    {
        ++k;

        const Mat4 mat = c->transformation();
        const Vec3
                right = Vec3(mat * Vec4(1,0,0,0)),
                up =    Vec3(mat * Vec4(0,1,0,0)),
                look =  Vec3(mat * Vec4(0,0,1,0)),
                pos =   Vec3(mat * Vec4(0,0,0,1));

        out << "// " << c->name() << "\n"
            << "// (" << c->idNamePath() << ")\n"
            << "#define Camera_" << k << " = camera\n"
            << "{\n"
                << "\t// almost working\n"
                << "\tlocation " << povStr(pos) << "\n"
                << "\tright <-1.3, 1, 1> * " << povStr(right) << "\n"
                << "\tup " << povStr(up) << "\n"
                << "\tlook_at " << povStr(pos + look) << "\n"
            << "}\n\n";
    }

    out << "camera { Camera_1 }\n\n";
}


void PovrayExporter::exportLights_(QTextStream & out, Double time) const
{
    out << "// ---------------- light sources ------------------\n\n";

    auto lights = scene_->findChildObjects<LightSource>(QString(), true);

    for (LightSource * l : lights)
    {
        if (!l->active(RenderTime(time, thread_)))
            continue;

        out << "// " << l->name() << "\n"
            << "// (" << l->idNamePath() << ")\n"
            << "light_source\n"
            << "{\n"
                << "\tlocation 0\n"
                << "\tcolor rgb " << povStr(Vec3(l->lightColor(RenderTime(time, thread_)))) << "\n"
                << "\tmatrix " << povStr(l->transformation()) << "\n"
            << "}\n\n";
    }
}

void PovrayExporter::exportModels_(QTextStream &out, Double ti) const
{
    out << "// ---------------- models ------------------\n\n";

    RenderTime time(ti, thread_);

    auto models = scene_->findChildObjects<Model3d>(QString(), true);

    for (Model3d * m : models)
    {
        if (!m->active(time)
            || !m->geometry())
            continue;

        out << "// " << m->name() << "\n"
            << "// (" << m->idNamePath() << ")\n"
            << "mesh\n"
            << "{\n";
        exportGeometry_(out, "\t", m->geometry());
        out     << "\tcolor rgbf " << povRgbf(m->modelColor(time)) << "\n"
                << "\tmatrix " << povStr(m->transformation()) << "\n"
            << "}\n\n";
    }
}

void PovrayExporter::exportGeometry_(
        QTextStream &out, const QString &prefix, const GEOM::Geometry * geom) const
{
    if (!geom->numTriangles())
    {
        return;
    }

    out << prefix << "// " << geom->numTriangles() << " triangles\n";
    for (uint i=0; i<geom->numTriangles(); ++i)
    {
        const GEOM::Geometry::VertexType
                t1 = geom->triangleIndex(i, 0),
                t2 = geom->triangleIndex(i, 1),
                t3 = geom->triangleIndex(i, 2);

        const Vec3
                p1 = geom->getVertex(t1),
                p2 = geom->getVertex(t2),
                p3 = geom->getVertex(t3);

        const Vec2
                uv1 = geom->getTexCoord(t1),
                uv2 = geom->getTexCoord(t2),
                uv3 = geom->getTexCoord(t3);

        out << prefix << "triangle {\n"
            << prefix << "\t" << povStr(p1) << ", " << povStr(p2) << ", " << povStr(p3) << "\n"
            << prefix << "\tuv_vectors "
                << povStr(uv1) << ", " << povStr(uv2) << ", " << povStr(uv3) << "\n"
            << prefix << "}\n";
    }
}


} // namespace IO
} // namespace MO
