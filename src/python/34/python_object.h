/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_OBJECT_H
#define MOSRC_PYTHON_34_PYTHON_OBJECT_H

namespace MO {
class Object;
namespace PYTHON34 {

    /** Adds the Geometry object to the module.
        @p module is PyObject* */
    void initObject(void* module);

    /** Creates a new matrixoptimizer.Object instance.
        @returns PyObject* */
    void* createObjectWrapper(Object*);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_OBJECT_H

#endif // MO_ENABLE_PYTHON34
