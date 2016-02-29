/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#ifndef MOSRC_PYTHON_27_GEOMETRYMODULE_H
#define MOSRC_PYTHON_27_GEOMETRYMODULE_H

namespace MO {
namespace GEOM { class Geometry; }
namespace PYTHON27 {

    /** Adds the Geometry object to the module.
        @p module is PyObject* */
    void initGeometry(void* module);

    /** Wraps a Geometry into it's python object.
        Steals a reference from the Geometry.
        @returns PyObject* */
    void* createGeometryObject(MO::GEOM::Geometry*);

} // namespace PYTHON27
} // namespace MO


#endif // MOSRC_PYTHON_27_GEOMETRYMODULE_H

#endif // MO_ENABLE_PYTHON27
