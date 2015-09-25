/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2015</p>
*/

#ifndef MO_DISABLE_SHP

#ifndef MOSRC_GEOM_SHPLOADER_H
#define MOSRC_GEOM_SHPLOADER_H

#include <functional>

#include <QString>
#include <QRectF>

class QPolygonF;

namespace MO {
namespace GEOM {

class Geometry;

class ShpLoader
{
    ShpLoader(const ShpLoader&);
    void operator=(const ShpLoader&);
public:

    enum TriangulationMesh
    {
        TM_NONE = 0,
        TM_QUAD
    };

    ShpLoader();
    ~ShpLoader();

    /** Sets the basis mesh for triangulation, or none */
    void setTriangulationMesh(TriangulationMesh m);
    void setTriangulationMeshSpacing(float x, float y);

    /** Loads the file into internal data.
        @throws IoException on file or parsing errors. */
    void loadFile(const QString& filename,
                  std::function<void(double)> progressFunc = 0);

    /** Loads the data into the Geometry container.
        Previous contents will be kept. */
    void getGeometry(Geometry *,
                     std::function<void(double)> progressFunc = 0) const;

    /** Returns the polygon data loaded from the SHP file. */
    void getPolygons(QVector<QPolygonF>& poly);

    QRectF getBoundingRect() const;

    /** [0,100] */
    double progress() const;

    // ----- buffered singelton access -----------

    /** Loads the data into the Geometry container.
        Previous contents will be kept.
        An ShpLoader class will be created for the file and kept.
        Throws IO::Exception on any errors. */
    static void getGeometry(const QString& filename, Geometry *,
                            std::function<void(double)> progressFunc = 0);

    static void getGeometry(const QString& filename, Geometry *,
                            TriangulationMesh m, float triMeshX, float triMeshY,
                            std::function<void(double)> progressFunc = 0);

private:

    struct Private;
    Private * p_;
};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_SHPLOADER_H

#endif // #ifndef MO_DISABLE_SHP
