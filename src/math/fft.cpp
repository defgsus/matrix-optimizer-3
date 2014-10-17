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

/** @page fft_pseudowisdom

    layout of
    real and imaginary parts

    r(0), r(1), r(2) ... r(n/2-2), r(n/2-1),  i(n/2-1), i(n/2-2), ... i(1), i(0)

    seems like:

    i(0)   : negative half amplitude of fundamental (1 hz in window)
             e.g.: i(0) = -0.5 makes a 1-period sine wave
    i(1)   : negative half amplitude of 2 hz sine wave
    r(0)   : positive half dc-offset (0 hz wave)
    r(1)   : positive half amplitude of fundamental cosine wave
    r(2)   : positive half amplitude of 2 hz cosine wave

  */



#include "fft.h"
#include "math/constants.h"

#include <cmath>

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
        for(i=0; i<n; i++)
            data[i] /= n;
    }



    /////////////////////////////////////////////////////////
    // Sorensen in-place inverse split-radix FFT for real values
    // data: array of doubles:
    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    //
    // output:
    // re(0),re(1),re(2),...,re(size-1)
    // NOT normalized by array length
    //
    // Source:
    // Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
    // IEEE Trans. ASSP, ASSP-35, No. 6, June 1987

    template <typename F, typename I>
    void irealfft_split(F * data, I n)
    {
        I i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8,n1;
        F t1,t2,t3,t4,t5,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;

        sqrt2 = std::sqrt(F(2));

        n1=n-1;
        n2=n<<1;

        for(k=n;k>2;k>>=1)
        {
            id = n2;
            n2 >>= 1;
            n4 = n2>>2;
            n8 = n2>>3;
            e = F(TWO_PI) / n2;
            i1=0;
            do
            {
                for (; i1<n; i1+=id)
                {
                    i2 = i1 + n4;
                    i3 = i2 + n4;
                    i4 = i3 + n4;
                    t1 = data[i1] - data[i3];
                    data[i1] += data[i3];
                    data[i2] *= 2;
                    data[i3] = t1 - F(2)*data[i4];
                    data[i4] = t1 + F(2)*data[i4];
                    if (n4!=1)
                    {
                        i0= i1 + n8;
                        i2 += n8;
                        i3 += n8;
                        i4 += n8;
                        t1 = (data[i2] - data[i0]) / sqrt2;
                        t2 = (data[i4] + data[i3]) / sqrt2;
                        data[i0] += data[i2];
                        data[i2] = data[i4] - data[i3];
                        data[i3] = F(2)*(-t2 - t1);
                        data[i4] = F(2)*(-t2 + t1);
                    }
                }
                id <<= 1;
                i1 = id - n2;
                id <<= 1;
            }
            while (i1 < n1);

            a = e;
            for (j=2; j<=n8; j++)
            {
                a3 = F(3) * a;
                cc1 = std::cos(a);
                ss1 = std::sin(a);
                cc3 = std::cos(a3);
                ss3 = std::sin(a3);
                a = j * e;
                i = 0;
                id = n2 << 1;
                do
                {
                    for (; i<n; i+=id)
                    {
                        i1 = i + j - 1;
                        i2 = i1 + n4;
                        i3 = i2 + n4;
                        i4 = i3 + n4;
                        i5 = i + n4 - j + 1;
                        i6 = i5 + n4;
                        i7 = i6 + n4;
                        i8 = i7 + n4;
                        t1 = data[i1] - data[i6];
                        data[i1] += data[i6];
                        t2 = data[i5] - data[i2];
                        data[i5] += data[i2];
                        t3 = data[i8] + data[i3];
                        data[i6] = data[i8] - data[i3];
                        t4 = data[i4] + data[i7];
                        data[i2] = data[i4] - data[i7];
                        t5 = t1 - t4;
                        t1 += t4;
                        t4 = t2 - t3;
                        t2 += t3;
                        data[i3] =  t5 * cc1 + t4 * ss1;
                        data[i7] = -t4 * cc1 + t5 * ss1;
                        data[i4] =  t1 * cc3 - t2 * ss3;
                        data[i8] =  t2 * cc3 + t1 * ss3;
                    }
                    id <<= 1;
                    i = id - n2;
                    id <<= 1;
                }
                while (i < n1);
            }
        }

        /*----------------------*/
        i0 = 0;
        id = 4;
        do
        {
            for (; i0<n1; i0+=id)
            {
                i1 = i0 + 1;
                t1 = data[i0];
                data[i0] = t1 + data[i1];
                data[i1] = t1 - data[i1];
            }
            id <<= 1;
            i0 = id - 2;
            id <<= 1;
        }
        while (i0 < n1);

        /*----------------------*/

        //data shuffling
        for (i=0, j=0, n2=n/2; i<n1 ; i++)
        {
            if (i<j)
            {
                t1 = data[j];
                data[j] = data[i];
                data[i] = t1;
            }
            k = n2;
            while (k <= j)
            {
                j -= k;
                k >>= 1;
            }

            j += k;
        }

    }



    template <typename F>
    void get_amp_phase_(F * data, uint num)
    {
        const uint half = num/2;
        for (uint i=0; i<half; ++i)
        {
            const F re = data[i],
                    im = data[num-1-i];

            // amplitude
            data[i] = std::sqrt(re*re + im*im) * 2.0;
            // phase
            data[num-1-i] = std::atan2(im, re);
        }
        // flip phase
        const uint half2 = half/2;
        for (uint i=0; i<half2; ++i)
            std::swap(data[half+i], data[num-1-i]);
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

void ifft(Float* data, uint num)
{
    irealfft_split(data, num);
}

void ifft(Double* data, uint num)
{
    irealfft_split(data, num);
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
