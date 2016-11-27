/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/27/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_MATRIX4_H
#define MOSRC_PYTHON_34_PYTHON_MATRIX4_H

#include "py_utils.h"
#include "math/vector.h"

namespace MO {
namespace MATH { template <typename F> class ArithmeticArray; }
namespace PYTHON34 {

    PyMODINIT_FUNC mat4CreateModule();

    /** Adds the Vector objects to the module. */
    void initMat4(PyObject* module);

    bool isMat4(PyObject* pyObject);

    PyObject* buildMat4(const DMat4& v);
    PyObject* buildMat4(const double v[]);

    DMat4 getMat4(PyObject* pyObject);
    bool  getMat4(PyObject* pyObjectWhichIsMat4, double vOut[16]);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_MATRIX4_H

#endif // MO_ENABLE_PYTHON34

