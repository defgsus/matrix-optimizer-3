/** @file fft.cpp

    @brief fft and ifft functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/8/2014</p>
*/

/* source:
 * Toth Laszlo
 * http://musicdsp.org/showArchiveComment.php?ArchiveID=79
 */

#include "fft.h"
#include "math/constants.h"

namespace MO {
namespace MATH {

// ----------------------- implementation ----------------------------------

namespace
{

    /////////////////////////////////////////////////////////
    // Sorensen in-place radix-2 FFT for real values
    // data: array of doubles:
    // re(0),re(1),re(2),...,re(size-1)
    //
    // output:
    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    // normalized by array length
    //
    // Source:
    // Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
    // IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
    template <typename F, typename I>
    void realfft_radix2(F * data, I n)
    {
        F  xt,a,e, t1, t2, cc, ss;
        I  i, j, k, n1, n2, n3, n4, i1, i2, i3, i4;

        n4 = n - 1;

        //data shuffling
        for (i=0, j=0, n2=n/2; i<n4; i++)
        {
            if (i < j)
            {
                xt = data[j];
                data[j] = data[i];
                data[i] = xt;
            }
            k = n2;
            while (k<=j)
            {
                j -= k;
                k >>= 1;
            }
            j += k;
        }

        /* -------------------- */
        for (i=0; i<n; i += 2)
        {
            xt = data[i];
            data[i] = xt + data[i+1];
            data[i+1] = xt - data[i+1];
        }

        /* ------------------------ */
        n2 = 1;
        for (k=n; k>2; k>>=1)
        {
            n4 = n2;
            n2 = n4 << 1;
            n1 = n2 << 1;
            e = TWO_PI / n1;
            for (i=0; i<n; i+=n1)
            {
                xt = data[i];
                data[i] = xt + data[i+n2];
                data[i+n2] = xt-data[i+n2];
                data[i+n4+n2] = -data[i+n4+n2];
                a = e;
                n3=n4-1;
                for (j = 1; j <= n3; j++)
                {
                    i1 = i+j;
                    i2 = i - j + n2;
                    i3 = i1 + n2;
                    i4 = i - j + n1;
                    cc = std::cos(a);
                    ss = std::sin(a);
                    a += e;
                    t1 = data[i3] * cc + data[i4] * ss;
                    t2 = data[i3] * ss - data[i4] * cc;
                    data[i4] = data[i2] - t2;
                    data[i3] = -data[i2] - t2;
                    data[i2] = data[i1] - t1;
                    data[i1] += t1;
                }
            }
        }

        //division with array length
        //for(i=0; i<n; i++)
        //    data[i]/=n;
    }




    template <typename F>
    void get_amp_phase_(F * data, uint num)
    {
        // for some reason this seems to give a good amplitude
        const F amp = F(1) / std::sqrt(num);

        for (uint i=0; i<num; ++i)
        {
            const F re = data[i],
                    im = data[i+num];

            // amplitude
            //data[i] = std::sqrt(re*re + im*im);
            data[i] = re * amp;
            // phase
            data[i+num] = std::atan2(im, re);

        }
    }

} // namespace



void real_fft(Float* data, uint num)
{
    realfft_radix2(data, num);
}

void real_fft(Double* data, uint num)
{
    realfft_radix2(data, num);
}

void get_amplitude_phase(Float *data, uint num)
{
    get_amp_phase_(data, num);
}

void get_amplitude_phase(Double *data, uint num)
{
    get_amp_phase_(data, num);
}


} // namespace MATH
} // namespace MO
