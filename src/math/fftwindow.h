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
        T_HANNING,
        T_FLATTOP,
        T_TRIANGULAR,
        T_GAUSS
    };

    /** Generates a window of @p num samples into data */
    template <typename F>
    static void makeWindow(F* data, size_t num, Type type)
    {
        const size_t num2 = num / 2;
        const F PI = F(3.14159265);
                //PI2 = PI * F(2);
        switch (type)
        {
            case T_HANNING:
                for (size_t i=0; i<num2; ++i)
                    data[i] = F(.5)-F(.5)*std::cos(F(i)/(num2-1) * PI);
            break;

            case T_FLATTOP:
                for (size_t i=0; i<num2; ++i)
                {
                    F t = F(i)/(num2-1) * PI;
                    data[i] = F(1)
                     - F(1.93293488969227) * std::cos(t)
                     + F(1.28349769674027) * std::cos(t * F(2))
                     - F(0.38130801681619) * std::cos(t * F(3))
                     + F(0.02929730258511) * std::cos(t * F(4));
                    data[i] /= 4.62708;
                }
            break;

            case T_TRIANGULAR:
                for (size_t i=0; i<num2; ++i)
                    data[i] = F(i) / (num2-1);
            break;

            case T_GAUSS:
                for (size_t i=0; i<num2; ++i)
                    data[i] = std::exp(-std::pow(
                            (F(i)-F(num2))/F(num2)*F(2.7183), F(2)));
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

