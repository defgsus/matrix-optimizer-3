/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_VECTOR_H
#define MOSRC_PYTHON_34_PYTHON_VECTOR_H

#include "py_utils.h"
#include "math/vector.h"

namespace MO {
namespace MATH { template <typename F> class ArithmeticArray; }
namespace PYTHON34 {

    PyMODINIT_FUNC vecCreateModule();

    /** Adds the Vector objects to the module. */
    void initVector(PyObject* module);

    bool isVector(PyObject* pyObject);

    bool get_vector(PyObject* args_, int len, double v[]);
    bool get_vector_var(PyObject* args_, int *len, double vout[4]);

    PyObject* buildVector(const Vec2& v);
    PyObject* buildVector(const Vec3& v);
    PyObject* buildVector(const Vec4& v);
    PyObject* buildVector(const DVec2& v);
    PyObject* buildVector(const DVec3& v);
    PyObject* buildVector(const DVec4& v);
    PyObject* buildVector(const double v[], int len);
    PyObject* buildVector(const float v[], int len);
    PyObject* buildVector(double x, double y);
    PyObject* buildVector(double x, double y, double z);
    PyObject* buildVector(double x, double y, double z, double w);
    PyObject* buildVector(const MATH::ArithmeticArray<double>& v);

    DVec2 getVector2(PyObject* pyObject);
    DVec3 getVector3(PyObject* pyObject);
    DVec4 getVector4(PyObject* pyObject);
    bool  getVector(PyObject* pyObjectWhichIsVector, int *lenOut, double vOut[4]);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_VECTOR_H

#endif // MO_ENABLE_PYTHON34
