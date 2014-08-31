/** @file povrayexporter.h

    @brief Exporter of Scene to Povray script

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#ifndef MOSRC_IO_POVRAYEXPORTER_H
#define MOSRC_IO_POVRAYEXPORTER_H

#include "types/float.h"
#include "object/object_fwd.h"

class QString;
class QTextStream;

namespace MO {
namespace GEOM { class Geometry; }
namespace IO {


class PovrayExporter
{
public:
    PovrayExporter();

    void setScene(const Scene *);

    void exportScene(const QString& filename, Double time) const;
    void exportScene(QTextStream& stream, Double time) const;

private:

    void exportCameras_(QTextStream& out, Double time) const;
    void exportLights_(QTextStream& out, Double time) const;
    void exportModels_(QTextStream& out, Double time) const;
    void exportGeometry_(QTextStream& out, const QString& prefix,
                         const GEOM::Geometry*) const;

    const Scene * scene_;
    uint thread_;
};


} // namespace IO
} // namespace MO


#endif // MOSRC_IO_POVRAYEXPORTER_H
