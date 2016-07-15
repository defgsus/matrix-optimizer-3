/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/15/2016</p>
*/

#ifndef MOSRC_MATH_FFTWINDOW_H
#define MOSRC_MATH_FFTWINDOW_H

#include <cstddef>
#include <cmath>
#include <vector>

namespace MO {
namespace MATH {

/** Namespace for generating a windowing function */
class FftWindow
{
public:
    /** @todo implement all of, e.g.,
    http://www.iowahills.com/Example%20Code/WindowedFIRFilterWebCode.txt
    */
    enum Type
    {
        T_COSINE,
        T_TRIANGULAR
    };

    /** Generates a window of @p num samples into data */
    template <typename F>
    static void makeWindow(F* data, size_t num, Type type)
    {
        const size_t num2 = num;
        switch (type)
        {
            case T_COSINE:
                for (size_t i=0; i<num2; ++i)
                    data[i] = F(.5)-F(.5)*std::cos(F(i)/num2 * 3.14159265f);
            break;

            case T_TRIANGULAR:
                for (size_t i=0; i<num2; ++i)
                    data[i] = F(i) / (num2-1);
            break;
        }

        for (size_t i=0; i<num2; ++i)
            data[num-1-i] = data[i];
    }

    template <typename F>
    static void makeWindow(std::vector<F>& data, size_t num, Type type)
    {
        data.resize(num);
        makeWindow(data.data(), num, type);
    }

    template <typename F>
    static void makeWindow(std::vector<F>& data, Type type)
    {
        makeWindow(data.data(), data.size(), type);
    }

};

} // namespace MATH
} //namespace MO

#endif // MOSRC_MATH_FFTWINDOW_H

