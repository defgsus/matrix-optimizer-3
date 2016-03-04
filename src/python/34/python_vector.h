/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/2/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_VECTOR_H
#define MOSRC_PYTHON_34_PYTHON_VECTOR_H

#include "math/vector.h"

namespace MO {
namespace PYTHON34 {

    /** Adds the Vector objects to the module.
        @p module is PyObject* */
    void initVector(void* module);

    bool isVector(void* pyObject);

    bool get_vector(void* PyObjectArgs_, int len, double v[]);
    bool get_vector_var(void* PyObjectArgs_, int *len, double vout[4]);

    void* buildVector(const Vec2& v);
    void* buildVector(const Vec3& v);
    void* buildVector(const Vec4& v);
    void* buildVector(const DVec2& v);
    void* buildVector(const DVec3& v);
    void* buildVector(const DVec4& v);
    void* buildVector(const double v[], int len);
    void* buildVector(double x, double y);
    void* buildVector(double x, double y, double z);
    void* buildVector(double x, double y, double z, double w);

    DVec2 getVector2(void* pyObject);
    DVec3 getVector3(void* pyObject);
    DVec4 getVector4(void* pyObject);
    bool  getVector(void* pyObjectWhichIsVector, int *lenOut, double vOut[4]);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_VECTOR_H

#endif // MO_ENABLE_PYTHON34
