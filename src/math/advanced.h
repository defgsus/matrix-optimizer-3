/** @file advanced.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MOSRC_MATH_ADVANCED_H
#define MOSRC_MATH_ADVANCED_H

#include "math/functions.h"
#include "math/noiseperlin.h"
#include "math/interpol.h"
#include "math/constants.h"
#include "types/conversion.h"

namespace MO {
namespace MATH {

/*
#ifdef PPP_USE_NDIV_TABLE
#	include "int_table_ndiv.inc"
#endif // USE_NDIV_TABLE

#ifdef PPP_USE_DIVISORS_TABLE
#	include "int_table_divisors.inc"
#endif // USE_DIVISORS_TABLE
*/


// ------------------------- generic math functions ------------------------

#if 1
#	define MO__SAVE_MOD(a__, b__) ( ((a__)!=0 && (b__)!=0)? (a__) % (b__) : 0 )
#else
#	define MO__SAVE_MOD(a__, b__) ((a__) % (b__))
#endif


/** reoccuring procedures for integer arithmetic */
template <typename I>
struct advanced_int
{
    static I sign(I x) { return x > 0 ? 1 : x < 0 ? -1 : 0; }

    static I num_div(I x)
    {
        x = std::abs(x);
#ifdef PPP_USE_NDIV_TABLE
        if (x==0) return 0;
        x = std::abs(x);
        if (x==1) return 1;

        if (x < int_table_ndiv_size)
            return int_table_ndiv[x];
#else
        if (x==0) return 0;
        if (x==1) return 1;
#endif
        I n = 2;
        const I x2 = x >> 1;
        for (I i=2; i<=x2; ++i)
            if (x % i == 0) ++n;
        return n;
    }

    /* return the 'index'th divisor of 'x', starting with 1 for 'x' == 0,
        or 0 if 'index' is out of range */
    static I divisor(I x, I index)
    {
        I sgn = sign(x);
        x = std::abs(x);
        if (x==0 || index < 0) return 0;
        if (index==0) return 1;
        if (x==1) return 0; /* already out of range (index!=0) */
        if (x==2) return index==1? 2 : 0;
        if (x==3) return index==1? 3 : 0;

#ifdef PPP_USE_NDIV_TABLE
        // early test against number-divisors-table
        if (x < int_table_ndiv_size)
        {
            const I n = int_table_ndiv[x] - 1;
            // out of range?
            if (index > n) return 0;
            // number itself?
            if (index == n) return x * sgn;
        }
#endif

#ifdef PPP_USE_DIVISORS_TABLE
        // look up divisors-table
        if (x < int_table_div_size)
        {
            const int * t = &int_table_div_data[int_table_div_index[x]];
            I k = 1;
            while (*t)
            {
                if (k == index) return *t * sgn;
                ++k;
                ++t;
            }
            // number itself
            if (k == index) return x * sgn;
            return 0;
        }
#endif
        // brute force
        I k = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            if (k == index) return (x / i) * sgn;
            ++k;
        }
        if (k == index) return x * sgn;
        return 0;
    }

    static I sum_div(I x)
    {
        I sgn = sign(x);
        x = std::abs(x);
        if (x<4) return sgn;

#ifdef PPP_USE_NDIV_TABLE
        // early test against number-divisors-table
        if (x < int_table_ndiv_size)
        {
            const I n = int_table_ndiv[x];
            if (n == 2) return 1;
        }
#endif

#ifdef PPP_USE_DIVISORS_TABLE
        // look up divisors-table
        if (x < int_table_div_size)
        {
            const int * t = &int_table_div_data[int_table_div_index[x]];
            I sum = 1;
            while (*t)
            {
                sum += *t;
                ++t;
            }
            return sum * sgn;
        }
#endif
        // brute force
        I sum = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            sum += i;
        }
        return sum * sgn;
    }


    static I prod_div(I x)
    {
        I sgn = sign(x);
        x = std::abs(x);
        if (x<4) return sgn;

#ifdef PPP_USE_NDIV_TABLE
        // early test against number-divisors-table
        if (x < int_table_ndiv_size)
        {
            const I n = int_table_ndiv[x];
            if (n == 2) return 1;
        }
#endif

#ifdef PPP_USE_DIVISORS_TABLE
        // look up divisors-table
        if (x < int_table_div_size)
        {
            const int * t = &int_table_div_data[int_table_div_index[x]];
            I sum = 1;
            while (*t)
            {
                sum *= *t;
                ++t;
            }
            return sum * sgn;
        }
#endif
        // brute force
        I sum = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            sum *= i;
        }
        return sum * sgn;
    }


    static I next_div(I x, I d)
    {
        I sgn = sign(x);
        x = std::abs(x);
        d = std::abs(d); // XXX not perfect
        if (d >= x) return x * sgn;

        for (I i = d; i < x; ++i)
            if (x % i == 0) return i * sgn;
        return x * sgn;
    }


    static bool is_prime(I k)
    {
        k = std::abs(k);
        if (k < 2 || k == 4) { return false; } else
        if (k == 2) { return true; } else
        // even?
        if (!(k & 1)) { return false; }
#ifdef PPP_USE_NDIV_TABLE
        // test against number-divisors-table
        if (k < int_table_ndiv_size)
        {
            const I n = int_table_ndiv[k];
            return (n == 2);
        }
#endif
        const I m = std::sqrt(k)+1;
        // test divisors
        for (I i=2; i<m; ++i)
            if (k % i == 0) { return false; }

        return true;
    }


    /** greates common divisor */
    static I gcd(I a, I b)
    {
        a = std::abs(a);
        b = std::abs(b);
        while (true)
        {
            a = MO__SAVE_MOD(a , b);
            if (a == 0)	return b;

            b = MO__SAVE_MOD(b , a);
            if (b == 0) return a;
        }
    }

    static I congruent(I a, I b, I m)
    {
        return MO__SAVE_MOD(b-a, m) == 0;
    }

    static I factorial(I n)
    {
        I r = 1;
        for (I i=2; i<=n; ++i)
            r *= i;
        return r;
    }

    static I num_digits(I n, I base = 10)
    {
        I d = 1;
        while (n >= base) { ++d; n /= base; };
        return d;
    }

    static I ulam_spiral(I x, I y)
    {
        I d;
        if ((d=x )>std::abs(y)) return (d * 4 - 3) * d + y + 1;
        if ((d=-x)>std::abs(y)) return (d * 4 + 1) * d - y + 1;
        {
            d = abs(y);
            if (y > 0) return (d * 4 - 1) * d - x + 1;
                else   return (d * 4 + 3) * d + x + 1;
        }
    }

    static I ulam_spiral_width(I x, I y, I w)
    {
        I d;
        const I wr = w >> 1, wl = w - wr, w2 = w << 1;
        if ((d= x-wr)>std::abs(y)) return (d * 4 - 3 + w2) * d - w + y + 1;
        if ((d=-x-wl)>std::abs(y)) return (d * 4 + 1 + w2) * d     - y + 1;
        {
            d = abs(y);
            if (y > 0) return (d * 4 - 1 + w2) * d - wl - x + 1;
                else   return (d * 4 + 3 + w2) * d + wl + x + 1;
        }
    }

    static I tri_spiral(I x, I y)
    {
        I d;
        if ((x ^ y) & 1)
            return 0;
        else
        if (y < 0 && y*3 <= x && x <= y*-3)
            return ( (y*9-6)*y/2 + x/2 + 1 );
        else
        {
            d = std::abs(x) + y;
            return ( (d*9-6)*d/8 - x + 1 );
        }
    }

    static I quer(I k)
    {
        k = std::abs(k);
        I q = 0;
        do
        {
            if (k<10)
            {
                q += k;
                return q;
            }
            else
            {
                q += k % 10;
                k /= 10;
            }
        }
        while (true);
    }

    static I fibonacci (I index)
    {
        I f = 1, f0 = 0, f1;
        for (I i=1; i<index; ++i, f1 = f, f += f0, f0 = f1);
        return f;
    }

    static I harmonic_2(I A, I B)
    {
        if (A == 0 || B == 0) return 0;
        if (A % B == 0) return (A / B);
        if (B % A == 0) return (B / A);
        return 0;
    }

    static I harmonic_3(I A, I B, I C)
    {
        if (A == 0 || B == 0 || C == 0) return 0;
        /* XXX seems to be wrong
        if (A % B == 0) { I D = A / B; if (D % C == 0) return (D / C);
                                       if (C % D == 0) return (C / D); }
        if (B % A == 0) { I D = B / A; if (D % C == 0) return (D / C);
                                       if (C % D == 0) return (C / D); }
        if (A % C == 0) { I D = A / C; if (D % B == 0) return (D / B);
                                       if (B % D == 0) return (B / D); }
        if (C % A == 0) { I D = C / A; if (D % B == 0) return (D / B);
                                       if (B % D == 0) return (B / D); }
        if (C % B == 0) { I D = C / B; if (D % A == 0) return (D / A);
                                       if (A % D == 0) return (A / D); }
        if (B % C == 0) { I D = B / C; if (D % A == 0) return (D / A);
                                       if (A % D == 0) return (A / D); }
        */
        I
        D = A / B / C; if (float(A) / B / C == D) return D;
        D = A / C / B; if (float(A) / C / B == D) return D;
        D = B / A / C; if (float(B) / A / C == D) return D;
        D = B / C / A; if (float(B) / C / A == D) return D;
        D = C / A / B; if (float(C) / A / B == D) return D;
        D = C / B / A; if (float(C) / B / A == D) return D;
        return 0;
    }

    static bool is_harmonic_2(I A, I B)
    {
        return (A != 0 && B != 0 && ((A % B == 0) || (B % A == 0)));
    }

    static bool is_harmonic_3(I A, I B, I C) { return harmonic_3(A, B, C) != 0; }
};

class AdvancedPrivate
{
public:
    static NoisePerlin noise;
};


template <typename F>
struct advanced
{
    // ---------------------- 'logic' -----------------------------

    static F abs            (F A) { return std::abs(A); }
    static F floor			(F A) { return std::floor(A); }
    static F ceil			(F A) { return std::ceil(A); }
    static F round  		(F A) { return std::floor(A + F(0.5)); }
    static F frac			(F A) { return A - std::floor(A); }
    static F clamp          (F A, F B, F C) { return std::min(std::max(A, B), C); }
    static F quant          (F A, F B) { return std::floor(A / B) * B; }
    static F mod  			(F A, F B) { return std::fmod(A, B); }
    static F smod  			(F A, F B) { return (A<F(0))? std::fmod(A, B) : B - std::fmod(-A, B); }
    static F min			(F A, F B) { return std::min(A, B); }
    static F min2			(F A, F B, F C) { return std::min(A, std::min(B, C)); }
    static F min3			(F A, F B, F C, F D) { return std::min(A, std::min(B, std::min(C, D))); }
    static F max			(F A, F B) { return std::max(A, B); }
    static F max2			(F A, F B, F C) { return std::max(A, std::max(B, C)); }
    static F max3			(F A, F B, F C, F D) { return std::max(A, std::max(B, std::max(C, D))); }

    // ------------------- 'scientific' --------------------------

    static F sin			(F A) { return std::sin(A); }
    static F sinh			(F A) { return std::sinh(A); }
    static F asin			(F A) { return std::asin(A); }
    static F asinh			(F A) { return std::asinh(A); }
    static F cos			(F A) { return std::cos(A); }
    static F cosh			(F A) { return std::cosh(A); }
    static F acos			(F A) { return std::acos(A); }
    static F acosh			(F A) { return std::acosh(A); }
    static F tan			(F A) { return std::tan(A); }
    static F tanh			(F A) { return std::tanh(A); }
    static F atan			(F A) { return std::atan(A); }
    static F atan_2			(F A, F B) { return std::atan2(A, B); }
    static F atanh			(F A) { return std::atanh(A); }
    static F sinc			(F A) { return std::sin(A) / A; }

    static F exp			(F A) { return std::exp(A); }
    static F log			(F A) { return std::log(A); }
    static F log2			(F A) { return std::log2(A); }
    static F log10			(F A) { return std::log10(A); }

    static F pow			(F A, F B) { return std::pow(A, B); }
    static F sqrt			(F A) { return std::sqrt(A); }
    static F root			(F A, F B) { return std::pow(A, F(1) / B); }

    // ------------------- statistics ----------------------------

    static F logistic		(F A) { return F(1) / (F(1) + std::exp(-A)); }
    static F erf          	(F A) { return std::erf(A); }
    static F erfc       	(F A) { return std::erfc(A); }
    // gauss(x, dev)
    static F gauss       	(F A, F B) { return std::exp(-((A*A)/(F(2)*B*B))); }
    // gauss(x, dev, center)
    static F gauss_3       	(F A, F B, F C) { return std::exp(-(std::pow(A-C,2)/(2.0*B*B))); }
    // cauchy(x, dev)
    static F cauchy       	(F A, F B) { return (1.0/PI) * (B/(A*A + B*B)); }
    // cauchy(x, dev, center)
    static F cauchy_3       (F A, F B, F C) { return (1.0/PI) * (B/(std::pow(A-C,2)+ B*B)); }


    // -------------------- common gfx ---------------------------

    static F mix             (F A, F B, F C) { return A + C * (B - A); }

    static F smoothstep      (F A, F B, F C) { return MO::MATH::smoothstep(A, B, C); }
    static F smootherstep    (F A, F B, F C) { return MO::MATH::smootherstep(A, B, C); }

    static F smoothladder    (F A, F B)
    { return MO::MATH::quant(A, B) + B * MO::MATH::smoothstep(F(0), B,
                MO::MATH::modulo(A, B)); }

    static F smootherladder  (F A, F B)
    { return MO::MATH::quant(A, B) + B * MO::MATH::smootherstep(F(0), B,
                MO::MATH::modulo(A, B)); }

    // -------------------- geometric ----------------------------

    static F beta			(F A) { F a = F(1) - A * A;
                                                return a > F(0)? std::sqrt(a) : F(0); }
    static F beta_2			(F A, F B) { F a = F(1) - A * A - B * B;
                                                return a > F(0)? std::sqrt(a) : F(0); }
    static F beta_3			(F A, F B, F C) { F a = F(1) - A * A - B * B - C * C;
                                                return a > F(0)? std::sqrt(a) : F(0); }
    static F beta_4			(F A, F B, F C, F D) { F a = F(1) - A * A - B * B - C * C - D * D;
                                                return a > F(0)? std::sqrt(a) : F(0); }

    static F mag 			(F A, F B) { return std::sqrt(A * A + B * B); }
    static F mag_3			(F A, F B, F C) { return std::sqrt(A * A + B * B + C * C); }
    static F mag_4			(F A, F B, F C, F D) { return std::sqrt(A * A + B * B + C * C + D * D); }

    // ----------------- oscillator ------------------------------

    static F ramp			(F A)
        { return MO::MATH::moduloSigned( A, F(1)         ); }

    static F saw			(F A)
        { return -F(1) + F(2) * MO::MATH::moduloSigned( A, F(1) ); }

    static F square 		(F A)
        { return (MO::MATH::moduloSigned( A, F(1) ) >= F(0.5)) ? -F(1) : F(1) ; }

    static F tri			(F A)
    {
        F p;
        p = MO::MATH::moduloSigned(A, F(1));
        return (p<F(0.5)) ? (p * F(4) - F(1)) : (F(3) - p * F(4));
    }

    // with pulsewidth
    static F square_2		(F A, F B)
        { return (MO::MATH::moduloSigned( A, F(1) ) >= B) ? -F(1) : F(1) ; }

    static F tri_2			(F A, F B)
    {
        F p;
        p = MO::MATH::moduloSigned(A, F(1));
        return (p<B)? p * F(2)/ B - F(1) : (F(1)-p) * F(2)/(F(1)-B) - F(1);
    }

    static F note2freq_1     (F A)
    {
        const F o = std::pow(F(2), F(1) / F(12));
        return std::pow(o, A) * F(16.35155);
    }

    // -------------- random ------------------------------------

    static F rnd_0			 (F A) { return (F)rand() / RAND_MAX; }

    static F noise           (F A) { return AdvancedPrivate::noise.noise(A); }
    static F noise_2         (F A, F B) { return AdvancedPrivate::noise.noise(A, B); }
    static F noise_3         (F A, F B, F C) { return AdvancedPrivate::noise.noise(A, B, C); }
    static F noiseoct        (F A, uint oct)
        { return AdvancedPrivate::noise.noiseoct(A, std::min((uint)10, oct)); }
    static F noiseoct_2      (F A, F B, uint oct)
        { return AdvancedPrivate::noise.noiseoct(A, B, std::min((uint)10, oct)); }
    static F noiseoct_3      (F A, F B, F C,uint oct)
        { return AdvancedPrivate::noise.noiseoct(A, B, C, std::min((uint)10, oct)); }

    // -------------- fractal ----------------------------------

    // takes x,y position, returns z
    static F mandel          (F A, F B)
    {
        F   x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0,
            z = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const F x2 = x*x,
                         y2 = y*y;
            z = x2 + y2;
            if (z > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return sqrt(z);
    }

    // takes x,y position, returns iteration
    static F mandeli          (F A, F B)
    {
        F
            x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const F x2 = x*x,
                         y2 = y*y;
            if (x2 + y2 > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return iter;
    }

    static F mandel_3        (F A, F B, uint maxiter)
    {
        F
            x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0,
            z = 0.0;
        uint iter = 0;
        while (iter < maxiter)
        {
            const F x2 = x*x,
                         y2 = y*y;
            z = x2 + y2;
            if (z > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return sqrt(z);
    }

    // takes x,y position and maxiter, returns iteration
    static F mandeli_3        (F A, F B, uint maxiter)
    {
        F
            x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0;
        uint iter = 0;
        while (iter < maxiter)
        {
            const F x2 = x*x,
                         y2 = y*y;
            if (x2 + y2 > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return iter;
    }

    static F julia         (F A, F B, F C, F D)
    {
        F
            x0 = C,
            y0 = D,
            x = A,
            y = B,
            tmp = 0.0,
            z = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const F x2 = x*x,
                         y2 = y*y;
            z = x2 + y2;
            if (z > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return sqrt(z);
    }

    static F juliai          (F A, F B, F C, F D)
    {
        F
            x0 = C,
            y0 = D,
            x = A,
            y = B,
            tmp = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const F x2 = x*x,
                         y2 = y*y;
            if (x2 + y2 > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        return iter;
    }

    // ------------ smoothed number theory --------------

    // the int type used here
    typedef int INT;
    #define MO__INT(f__) convert<F, INT>(f__)

    static F s_prime			(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::is_prime( MO__INT(A) ),
                    (F)advanced_int<INT>::is_prime( MO__INT(A) + 1 ));
    }

    static F s_numdiv			(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::num_div( MO__INT(A) ),
                    (F)advanced_int<INT>::num_div( MO__INT(A) + 1 ));
    }

    static F s_divisor			(F A, INT B)
    {
        return MO::MATH::interpol_smooth(
            MO::MATH::frac(A),
            (F)advanced_int<INT>::divisor( MO__INT(A), MO__INT(std::abs((F)B)) ),
            (F)advanced_int<INT>::divisor( MO__INT(A) + 1, MO__INT(std::abs((F)B)) ));
                                                            // XXX todo: this cast is for non-integer std::abs() implementations
    }

    static F s_sumdiv       		(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::sum_div( MO__INT(A) ),
                    (F)advanced_int<INT>::sum_div( MO__INT(A) + 1 ));
    }

    static F s_proddiv  			(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::prod_div( MO__INT(A) ),
                    (F)advanced_int<INT>::prod_div( MO__INT(A) + 1 ));
    }

    static F s_nextdiv  			(F A, INT B)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::next_div( MO__INT(A), MO__INT(B) ),
                    (F)advanced_int<INT>::next_div( MO__INT(A) + 1, MO__INT(B) ) );
    }

    static F s_gcd  				(F A, INT B)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::gcd( A, B ),
                    (F)advanced_int<INT>::gcd( A + 1, B ) );
    }

    static F s_congruent			(F A, INT B, INT C)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::congruent( A, B, C ),
                    (F)advanced_int<INT>::congruent( A + 1, B, C ) );
    }

    static F s_factorial			(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::factorial( A ),
                    (F)advanced_int<INT>::factorial( A + 1 ));
    }

    static F s_digits       		(F A)
    {
        return MO::MATH::interpol_smooth(
                    MO::MATH::frac(A),
                    (F)advanced_int<INT>::num_digits( A ),
                    (F)advanced_int<INT>::num_digits( A + 1 ));
    }

#undef MO__INT

    // ------------- float number theory ----------------

    static F zeta           	(F A)
    {
        F s = -A, r = F(0);
        for (F i=F(1); i<F(70); ++i)
            r += std::pow(i, s);
        return r;
    }

    static F zetap      		(F A, F B)
    {
        F i = F(1), k, s = -A, r = F(0);
        do
        {
            k = std::pow(i, s);
            r += k;
        } while (k > B && ++i <= 200000);
    }

};

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_ADVANCED_H
