/** @file objexporter.h

    @brief Wavefront .obj exporter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#ifndef MOSRC_GEOM_OBJEXPORTER_H
#define MOSRC_GEOM_OBJEXPORTER_H

#include <QString>

namespace MO {
namespace GEOM {

class Geometry;

class ObjExporter
{
public:

    enum ExportOption
    {
        EO_NORMALS =        1<<0,
        EO_TEX_COORDS =     1<<1,

        EO_ALL = 0xfffffff
    };

    ObjExporter();

    // --------- getter -----------

    /** Returns the current options of type ExportOption */
    int options() const { return options_; }

    // --------- setter -----------

    /** Sets the export options of type ExportOption */
    void setOptions(int o) { options_ = o; }

    /** Enables or disables a specific option */
    void setOption(ExportOption option, bool enable);

    // ----------- io -------------

    /** Creates an .obj file from the given geometry.
        @throws IoException on io errors */
    void exportGeometry(const QString& filename, const Geometry *);

private:

    int options_;
};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_OBJEXPORTER_H
