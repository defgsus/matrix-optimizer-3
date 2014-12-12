/** @file transformationbuffer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.12.2014</p>
*/

#ifndef MOSRC_MATH_TRANSFORMATIONBUFFER_H
#define MOSRC_MATH_TRANSFORMATIONBUFFER_H

#include <vector>

#include "types/vector.h"

namespace MO {

class TransformationBuffer
{
public:
    TransformationBuffer(uint bufferSize)
        :   p_m_   (bufferSize)
    {
        setIdentity();
    }

    // ---------------- getter ---------------------

    const Mat4& transformation(uint sample) const { return p_m_[sample]; }

    const Mat4 * transformations() const { return &p_m_[0]; }

    uint bufferSize() const { return p_m_.size(); }

    // ---------------- setter ---------------------

    void resize(uint bufferSize) { p_m_.resize(bufferSize); setIdentity(); }

    void setIdentity() { for (auto & m : p_m_) m = Mat4(1); }

    Mat4& transformation(uint sample) { return p_m_[sample]; }

    void setTransformation(const Mat4& t, uint sample) { p_m_[sample] = t; }

    void setTransformations(const Mat4 * t) { memcpy(&p_m_[0], t, bufferSize() * sizeof(Mat4)); }

private:
    std::vector<Mat4> p_m_;
};

} // namespace MO

#endif // MOSRC_MATH_TRANSFORMATIONBUFFER_H
