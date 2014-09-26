/** @file geometryfactorysettings.h

    @brief Settings for dynamically creating Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H
#define MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H

#include <QStringList>

#include "types/int.h"
#include "types/float.h"
#include "io/filetypes.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class GeometryModifierChain;

class GeometryFactorySettings
{
public:
    enum Type
    {
        T_FILE,
        T_QUAD,
        T_TETRAHEDRON,
        T_BOX,
        T_BOX_UV,
        T_OCTAHEDRON,
        T_ICOSAHEDRON,
        T_DODECAHEDRON,
        T_CYLINDER_CLOSED,
        T_CYLINDER_OPEN,
        T_TORUS,
        T_UV_SPHERE,
        T_GRID_XZ,
        T_LINE_GRID
    };
    static const uint numTypes = T_LINE_GRID + 1;
    static const QStringList typeIds;
    static const QStringList typeNames;

    GeometryFactorySettings();
    ~GeometryFactorySettings();

    GeometryFactorySettings(const GeometryFactorySettings& other);
    GeometryFactorySettings& operator=(const GeometryFactorySettings& other);

    // ---------- io ---------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);

    /** Appends the geometry file to the list if type is T_FILE */
    void getNeededFiles(IO::FileList &files);

    // ---- public member ----

    GeometryModifierChain * modifierChain;

    Type type;
    QString filename;
    bool asTriangles, sharedVertices;
    Float smallRadius, colorR, colorG, colorB, colorA;
    uint segmentsX, segmentsY, segmentsZ;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYFACTORYSETTINGS_H
