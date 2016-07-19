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

/** Class for generating a windowing function */
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
        T_GAUSS,
        T_BLACKMAN,
        T_BLACKMAN_HARRIS
    };

    /** Defines the size of a flat top area.
        0.0 leaves the windowing functions unchanged.
        1.0 makes a rectangular window without transition. */
    double alpha;

    FftWindow(double alpha = 0.)
        : alpha     (alpha)
    { }

    /** Generates a window of @p num samples into data */
    template <typename F>
    void makeWindow(F* data, size_t num, Type type) const
    {
        const size_t numA = std::max(0, std::min(int(num)/2-1,
                                     int(num/2*alpha) ));
        const size_t num2 = num / 2 - numA;
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

            case T_BLACKMAN:
                for (size_t i=0; i<num2; ++i)
                {
                    F t = F(i)/(num2-1) * PI;
                    data[i] = F(0.42)
                            - F(0.50) * std::cos(t)
                            + F(0.08) * std::cos(t * F(2));
                }
            break;

            case T_BLACKMAN_HARRIS:
                for (size_t i=0; i<num2; ++i)
                {
                    F t = F(i)/(num2-1) * PI;
                    data[i] = F(0.35875)
                            - F(0.48829) * std::cos(t )
                            + F(0.14128) * std::cos(t * F(2))
                            - F(0.01168) * std::cos(t * F(3));
                }
            break;
        }

        // set flat-top area
        for (size_t i=num2; i<num/2; ++i)
            data[i] = F(1);
        // mirror
        for (size_t i=0; i<num/2; ++i)
            data[num-1-i] = data[i];
    }

    template <typename F>
    void makeWindow(std::vector<F>& data, size_t num, Type type) const
    {
        data.resize(num);
        makeWindow(data.data(), num, type);
    }

    template <typename F>
    void makeWindow(std::vector<F>& data, Type type) const
    {
        makeWindow(data.data(), data.size(), type);
    }

};

} // namespace MATH
} //namespace MO

#endif // MOSRC_MATH_FFTWINDOW_H

