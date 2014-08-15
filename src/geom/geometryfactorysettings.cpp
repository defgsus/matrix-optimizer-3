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

namespace MO {
namespace GEOM {


const QStringList GeometryFactorySettings::typeIds =
{
    "file", "quad",
    "tetra", "hexa", "octa", "icosa", "dodeca",
    "cyl", "cylo", "torus", "uvsphere",
    "gridxz", "lgrid"
};

const QStringList GeometryFactorySettings::typeNames =
{
    QObject::tr("file"),
    QObject::tr("quad"),
    QObject::tr("tetrahedron"),
    QObject::tr("hexahedron (cube)"),
    QObject::tr("octahedron"),
    QObject::tr("icosahedron"),
    QObject::tr("dodecahedron"),
    QObject::tr("cylinder (closed)"),
    QObject::tr("cylinder (open)"),
    QObject::tr("torus"),
    QObject::tr("uv-sphere"),
    QObject::tr("coordinate system"),
    QObject::tr("line-grid")
};

GeometryFactorySettings::GeometryFactorySettings()
    : modifierChain (new GeometryModifierChain()),
      type          (T_BOX),
      asTriangles   (true),
      sharedVertices(true),
      smallRadius   (0.2f),
      colorR        (0.5f),
      colorG        (0.5f),
      colorB        (0.5f),
      colorA        (1.0f),
      segmentsX     (10),
      segmentsY     (10),
      segmentsZ     (10)
{
    MO_DEBUG_GEOM("GeometryFactorySettings::GeometryFactorySettings()");

    // default: calculate normals
    modifierChain->addModifier(new GeometryModifierNormals());
}

GeometryFactorySettings::~GeometryFactorySettings()
{
    MO_DEBUG_GEOM("GeometryFactorySettings::~GeometryFactorySettings()");
    delete modifierChain;
}

GeometryFactorySettings::GeometryFactorySettings(const GeometryFactorySettings &other)
{
    *this = other;
}

GeometryFactorySettings& GeometryFactorySettings::operator =(
        const GeometryFactorySettings& other)
{
    *modifierChain = *other.modifierChain;
    type = other.type;
    asTriangles = other.asTriangles;
    sharedVertices = other.sharedVertices;
    smallRadius = other.smallRadius;
    colorR = other.colorR;
    colorG = other.colorG;
    colorB = other.colorB;
    colorA = other.colorA;
    segmentsX = other.segmentsX;
    segmentsY = other.segmentsY;
    segmentsZ = other.segmentsZ;

    return *this;
}


void GeometryFactorySettings::serialize(IO::DataStream & io) const
{
    MO_DEBUG_GEOM("GeometryFactorySettings::serialize()");

    io.writeHeader("geomfacset", 9);

    io << typeIds[type];

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

    // v8
    io << asTriangles << sharedVertices
          << segmentsX << segmentsY << segmentsZ
             << smallRadius;

    modifierChain->serialize(io);

    // v9
    io << colorR << colorG << colorB << colorA;
}

void GeometryFactorySettings::deserialize(IO::DataStream & io)
{
    MO_DEBUG_GEOM("GeometryFactorySettings::deserialize()");

    const int ver = io.readHeader("geomfacset", 9);

    io.readEnum(type, T_BOX, typeIds);

    // convert old files to new GeometryModifierChain version
    if (ver <= 7)
    {
        QString equationX, equationY, equationZ,
                pEquationX, pEquationY, pEquationZ;
        bool calcNormals, invertNormals, convertToLines, tesselate,
            normalizeVertices, removeRandomly, transformWithEquation,
            transformPrimitivesWithEquation, calcNormalsBeforePrimitiveEquation,
            extrude, withCoords;
        Float colorR, colorG, colorB, colorA;
        Float scale, scaleX, scaleY, scaleZ, removeProb, normalization,
            extrudeConstant, extrudeFactor;
        uint gridSize, tessLevel, removeSeed;

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
            modifierChain->addModifier(geom);
            geom->setTesselationLevel(tessLevel);
        }

        if (normalizeVertices)
        {
            auto geom = new GeometryModifierNormalize();
            modifierChain->addModifier(geom);
            geom->setNormalization(normalization);
        }

        if (!sharedVertices)
        {
            auto geom = new GeometryModifierVertexGroup();
            modifierChain->addModifier(geom);
            geom->setShared(false);
        }

        if (transformWithEquation)
        {
            auto geom = new GeometryModifierVertexEquation();
            modifierChain->addModifier(geom);
            geom->setEquationX(equationX);
            geom->setEquationY(equationY);
            geom->setEquationZ(equationZ);
        }

        if (calcNormalsBeforePrimitiveEquation)
        {
            auto geom = new GeometryModifierNormals();
            modifierChain->addModifier(geom);
            geom->setCalcNormals(true);
        }

        if (transformPrimitivesWithEquation)
        {
            auto geom = new GeometryModifierPrimitiveEquation();
            modifierChain->addModifier(geom);
            geom->setEquationX(pEquationX);
            geom->setEquationY(pEquationY);
            geom->setEquationZ(pEquationZ);
        }

        if (extrude)
        {
            if (calcNormals)
            {
                auto geom = new GeometryModifierNormals();
                modifierChain->addModifier(geom);
                geom->setCalcNormals(true);
            }
            auto geom = new GeometryModifierExtrude();
            modifierChain->addModifier(geom);
            geom->setConstant(extrudeConstant);
            geom->setFactor(extrudeFactor);
        }

        if (convertToLines)
        {
            auto geom = new GeometryModifierConvertLines();
            modifierChain->addModifier(geom);
        }

        if (removeRandomly)
        {
            auto geom = new GeometryModifierRemove();
            modifierChain->addModifier(geom);
            geom->setRandomSeed(removeSeed);
            geom->setRemoveProbability(removeProb);
        }

        if (calcNormals || invertNormals)
        {
            auto geom = new GeometryModifierNormals();
            modifierChain->addModifier(geom);
            geom->setCalcNormals(calcNormals);
            geom->setInvertNormals(invertNormals);
        }

        {
            auto geom = new GeometryModifierScale();
            modifierChain->addModifier(geom);
            geom->setScaleAll(scale);
            geom->setScaleX(scaleX);
            geom->setScaleX(scaleY);
            geom->setScaleX(scaleZ);
        }

    }

    if (ver>=8)
    {
        io >> asTriangles >> sharedVertices
              >> segmentsX >> segmentsY >> segmentsZ
                 >> smallRadius;

        modifierChain->deserialize(io);
    }

    if (ver>=9)
        io >> colorR >> colorG >> colorB >> colorA;
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
