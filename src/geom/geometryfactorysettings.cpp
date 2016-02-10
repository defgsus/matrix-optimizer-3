/** @file geometryfactorysettings.cpp

    @brief Settings for dynamically creating Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QFile>

#include "geometryfactorysettings.h"
#include "geometrymodifierchain.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

// for old file conversion
#include "geom/geometrymodifierscale.h"
#include "geom/geometrymodifiertesselate.h"
#include "geom/geometrymodifiernormals.h"
#include "geom/geometrymodifiernormalize.h"
#include "geom/geometrymodifierconvertlines.h"
#include "geom/geometrymodifierremove.h"
#include "geom/geometrymodifiervertexgroup.h"
#include "geom/geometrymodifiervertexequation.h"
#include "geom/geometrymodifierprimitiveequation.h"
#include "geom/geometrymodifierextrude.h"
#include "geom/geometrymodifiercreate.h"

namespace MO {
namespace GEOM {


GeometryFactorySettings::GeometryFactorySettings(Object * o)
    : object_           (o),
      modifierChain_    (new GeometryModifierChain())
{
    MO_DEBUG_GEOM("GeometryFactorySettings::GeometryFactorySettings(" << o << ")");

    // default: create and calculate normals
    modifierChain_->addModifier(new GeometryModifierCreate());
    modifierChain_->addModifier(new GeometryModifierNormals());
}

bool GeometryFactorySettings::operator != (const GeometryFactorySettings& o) const
{
    return *modifierChain_ != *o.modifierChain_;
}

void GeometryFactorySettings::getNeededFiles(IO::FileList &files)
{
    modifierChain_->getNeededFiles(files);
}

GeometryFactorySettings::~GeometryFactorySettings()
{
    MO_DEBUG_GEOM("GeometryFactorySettings::~GeometryFactorySettings()");
    delete modifierChain_;
}

GeometryFactorySettings::GeometryFactorySettings(const GeometryFactorySettings &other)
{
    *this = other;
}

GeometryFactorySettings& GeometryFactorySettings::operator =(
        const GeometryFactorySettings& other)
{
    *modifierChain_ = *other.modifierChain_;
    object_ = other.object_;

    return *this;
}


void GeometryFactorySettings::serialize(IO::DataStream & io) const
{
    MO_DEBUG_GEOM("GeometryFactorySettings::serialize()");

    io.writeHeader("geomfacset", 11);

#if 0 && defined( THAT_WAS_VERSION_8_TO_10__ )
    io << typeIds[type];
#endif

#if 0 && defined( THAT_WAS_VERSION_BELOW_8__ )

    io << calcNormals << asTriangles << convertToLines << sharedVertices
            << tesselate << normalizeVertices << removeRandomly
            << colorR << colorG << colorB << colorA
            << scale << scaleX << scaleY << scaleZ
            << removeProb << gridSize
            << segmentsX << segmentsY << segmentsZ
            << tessLevel << removeSeed << withCoords;

        // v6
        io << invertNormals;

        // v2
        io << normalization;

        // v3
        io << transformWithEquation << equationX << equationY << equationZ;

        // v4
        io << smallRadius;

        // v5
        io << transformPrimitivesWithEquation << calcNormalsBeforePrimitiveEquation
           << pEquationX << pEquationY << pEquationZ;

        // v6
        io << extrude << extrudeConstant << extrudeFactor;
    }

#endif

#if 0 && defined( THAT_WAS_VERSION_8_TO_10__ )
    // v8
    io << asTriangles << sharedVertices
          << segmentsX << segmentsY << segmentsZ
             << smallRadius;

    modifierChain_->serialize(io);

    // v9
    io << colorR << colorG << colorB << colorA;

    // v10
    io << filename;
#endif

    modifierChain_->serialize(io);
}

void GeometryFactorySettings::deserialize(IO::DataStream & io)
{
    MO_DEBUG_GEOM("GeometryFactorySettings::deserialize()");

    const int ver = io.readHeader("geomfacset", 11);

    GeometryModifierCreate::Type pre11Type;
    if (ver >= 8 && ver <= 10)
        io.readEnum(pre11Type, GeometryModifierCreate::T_BOX_UV,
                            GeometryModifierCreate::typeIds);

    // convert old files to new GeometrymodifierChain_ version
    if (ver <= 7)
    {
        QString equationX="x", equationY="y", equationZ="z",
                pEquationX="x", pEquationY="y", pEquationZ="z";
        bool calcNormals = false, invertNormals = false, convertToLines = false, tesselate = false,
            normalizeVertices = false, removeRandomly = false, transformWithEquation = false,
            transformPrimitivesWithEquation = false, calcNormalsBeforePrimitiveEquation = false,
            extrude = false, withCoords = true;
        Float colorR, colorG, colorB, colorA;
        Float scale, scaleX, scaleY, scaleZ, removeProb, normalization,
            extrudeConstant, extrudeFactor;
        uint gridSize, tessLevel, removeSeed;

        bool asTriangles, sharedVertices;
        uint segmentsX, segmentsY, segmentsZ;
        Float smallRadius;

        io >> calcNormals >> asTriangles >> convertToLines >> sharedVertices
            >> tesselate >> normalizeVertices >> removeRandomly
            >> colorR >> colorG >> colorB >> colorA
            >> scale >> scaleX >> scaleY >> scaleZ
            >> removeProb >> gridSize
            >> segmentsX >> segmentsY >> segmentsZ
            >> tessLevel >> removeSeed >> withCoords;

        if (ver >= 6)
            io >> invertNormals;

        if (ver >= 2)
            io >> normalization;

        if (ver >= 3)
            io >> transformWithEquation >> equationX >> equationY >> equationZ;

        if (ver >= 4)
            io >> smallRadius;

        if (ver >= 5)
            io >> transformPrimitivesWithEquation >> calcNormalsBeforePrimitiveEquation
               >> pEquationX >> pEquationY >> pEquationZ;

        if (ver >= 7)
            io >> extrude >> extrudeConstant >> extrudeFactor;

        // --- convert to modifier-chain -----

        if (tesselate)
        {
            auto geom = new GeometryModifierTesselate();
            modifierChain_->addModifier(geom);
            geom->properties().set("level", tessLevel);
        }

        if (normalizeVertices)
        {
            auto geom = new GeometryModifierNormalize();
            modifierChain_->addModifier(geom);
            geom->properties().set("amt", normalization);
        }

        if (!sharedVertices)
        {
            auto geom = new GeometryModifierVertexGroup();
            modifierChain_->addModifier(geom);
            geom->properties().set("shared", false);
        }

        if (transformWithEquation)
        {
            auto geom = new GeometryModifierVertexEquation();
            modifierChain_->addModifier(geom);
            QString equ = "x = " + equationX + ";\n"
                          "y = " + equationY + ";\n"
                          "z = " + equationZ;
            geom->properties().set("equ", equ);
        }

        if (calcNormalsBeforePrimitiveEquation)
        {
            auto geom = new GeometryModifierNormals();
            modifierChain_->addModifier(geom);
            geom->properties().set("tri", true);
        }

        if (transformPrimitivesWithEquation)
        {
            auto geom = new GeometryModifierPrimitiveEquation();
            modifierChain_->addModifier(geom);
            geom->properties().set("equ",
                                    "x = " + pEquationX + ";\n"
                                    "y = " + pEquationY + ";\n"
                                    "z = " + pEquationZ);
        }

        if (extrude)
        {
            if (calcNormals)
            {
                auto geom = new GeometryModifierNormals();
                modifierChain_->addModifier(geom);
                geom->properties().set("tri", true);
            }
            auto geom = new GeometryModifierExtrude();
            modifierChain_->addModifier(geom);
            geom->properties().set("const", extrudeConstant);
            geom->properties().set("factor", extrudeFactor);
        }

        if (convertToLines)
        {
            auto geom = new GeometryModifierConvertLines();
            modifierChain_->addModifier(geom);
        }

        if (removeRandomly)
        {
            auto geom = new GeometryModifierRemove();
            modifierChain_->addModifier(geom);
            geom->properties().set("prob", removeProb);
            geom->properties().set("seed", removeSeed);
        }

        if (calcNormals || invertNormals)
        {
            auto geom = new GeometryModifierNormals();
            modifierChain_->addModifier(geom);
            geom->properties().set("tri", calcNormals);
            geom->properties().set("invert", invertNormals);
        }

        {
            auto geom = new GeometryModifierScale();
            modifierChain_->addModifier(geom);
            geom->properties().set("all", scale);
            geom->properties().set("x", scaleX);
            geom->properties().set("y", scaleY);
            geom->properties().set("z", scaleZ);
        }

    }

    // convert 8 to 10
    if (ver >= 8 && ver <= 10)
    {
        bool asTriangles, sharedVertices;
        uint segmentsX, segmentsY, segmentsZ;
        Float smallRadius;

        io >> asTriangles >> sharedVertices
              >> segmentsX >> segmentsY >> segmentsZ
                 >> smallRadius;

        modifierChain_->deserialize(io);

        Float colorR=0.5, colorG=0.5, colorB=0.5, colorA=1.0;
        QString filename;

        if (ver>=9)
            io >> colorR >> colorG >> colorB >> colorA;

        if (ver>=10)
            io >> filename;

        // convert above stuff

        auto geom = new GeometryModifierCreate();
        geom->properties().set("type", int(pre11Type));
        geom->properties().set("asTriangles", asTriangles);
        geom->properties().set("shared", sharedVertices);
        geom->properties().set("radius", smallRadius);
        geom->properties().set("filename-obj", filename);
        geom->properties().set("filename-shp", filename);
        geom->properties().set("segments", QVector<uint>()
                     << segmentsX << segmentsY << segmentsZ);
        geom->properties().set("color", QVector<Float>()
                    << colorR << colorG << colorB << colorA);

        modifierChain_->insertModifier(geom, 0);

    }

    if (ver >= 11)
    {
        modifierChain_->deserialize(io);
    }
}

void GeometryFactorySettings::saveFile(const QString &filename) const
{
    MO_DEBUG_GEOM("GeometryFactorySettings::saveFile('" << filename << "')");

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not create geometry-settings file '" << filename
                    << "'\n" << f.errorString());

    IO::DataStream stream(&f);

    MO_EXTEND_EXCEPTION(
        serialize(stream),
        "on writing geometry-settings file '" << filename << "'"
        );
}


void GeometryFactorySettings::loadFile(const QString &filename)
{
    MO_DEBUG_GEOM("GeometryFactorySettings::loadFile('" << filename << "')");

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(WRITE, "Could not open geometry-settings file '" << filename
                    << "'\n" << f.errorString());

    IO::DataStream stream(&f);
    MO_EXTEND_EXCEPTION(
        deserialize(stream),
        "on reading geometry-settings file '" << filename << "'"
        );
}



} // namespace GEOM
} // namespace MO
