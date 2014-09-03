/**	@file functions.h

    @brief static functions for use with math parser

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef FUNCTIONS_H_DEFINED
#define FUNCTIONS_H_DEFINED

#include "parser_defines.h"
#include "math/functions.h"
#include "math/noiseperlin.h"
#include "math/interpol.h"

namespace PPP_NAMESPACE {

#ifdef PPP_USE_NDIV_TABLE
#	include "int_table_ndiv.inc"
#endif // USE_NDIV_TABLE

#ifdef PPP_USE_DIVISORS_TABLE
#	include "int_table_divisors.inc"
#endif // USE_DIVISORS_TABLE





// -------------- lambda ------------------------------------

/** lambda type functions */
template <typename F>
struct lambda_func
{
    typedef std::function<void(Float ** v)> FuncPtr;

    /**
    the line,
    @code
    f(x) { 1/x }; series(f, +, 1, 10);
    @endcode
    produces the sum over 1/x for x=1 to 10
    */
    static void series_4(FuncPtr func, FuncPtr operand, Float ** v)
    {
//		std::cout << "\n" << v[0] << "\n" << v[1] << "\n" << v[2] << "\n" << v[3] << "\n" << v[4] << "\n";
        *v[0] = 0.0;
        Int from = cast::toInt(*v[3]),
            to = cast::toInt(*v[4]);
        Float tmp0, tmp1;
        Float * ptmp[] = { &tmp0, &tmp1 };
        Float * pres[] = { v[0], v[0], &tmp0 };
        for (Int i = from; i<=to; ++i)
        {
            // call function: tmp0 <- f(i);
            tmp1 = i;
            func( ptmp );
            // combine v[0] <- v[0] + tmp0
            operand( pres );
        }
    }
};


// ------------------------- generic math functions ------------------------

/** all functions of the parser */
template <typename F>
struct math_func
{
    /* not specialized */
};

#ifndef PPP_GMP
#	define PPP_SAVE_MOD(a__, b__) ( ((a__)!=0 && (b__)!=0)? (a__) % (b__) : 0 )
#else
#	define PPP_SAVE_MOD(a__, b__) ((a__) % (b__))
#endif

/** reoccuring procedures for integer arithmetic */
template <typename I>
struct generic_int
{
    static I num_div(I x)
    {
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
            if (index == n) return x;
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
                if (k == index) return *t;
                ++k;
                ++t;
            }
            // number itself
            if (k == index) return x;
            return 0;
        }
#endif
        // brute force
        I k = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            if (k == index) return x / i;
            ++k;
        }
        if (k == index) return x;
        return 0;
    }

    static I sum_div(I x)
    {
        if (x==0) return 0;
        if (x<4) return 1;

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
            return sum;
        }
#endif
        // brute force
        I sum = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            sum += i;
        }
        return sum;
    }


    static I prod_div(I x)
    {
        if (x==0) return 0;
        if (x<4) return 1;

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
            return sum;
        }
#endif
        // brute force
        I sum = 1;
        for (I i = x>>1; i >= 2; --i)
        if (x % i == 0)
        {
            sum *= i;
        }
        return sum;
    }


    static I next_div(I x, I d)
    {
        if (d >= x) return x;

        for (I i = d; i < x; ++i)
            if (x % i == 0) return i;
        return x;
    }


    static bool isPrime(I k)
    {
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
            a = PPP_SAVE_MOD(a , b);
            if (a == 0)	return b;

            b = PPP_SAVE_MOD(b , a);
            if (b == 0) return a;
        }
    }

    static I congruent(I a, I b, I m)
    {
        return PPP_SAVE_MOD(b-a, m) == 0;
    }

    static I factorial(I n)
    {
        Int r = 1;
        for (Int i=2; i<=n; ++i)
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

    static I ulam_spiral(I x, I y, I w)
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

    static Int fibonacci (I index)
    {
        Int f = 1, f0 = 0, f1;
        for (I i=1; i<index; ++i, f1 = f, f += f0, f0 = f1);
        return f;
    }
};



static MO::MATH::NoisePerlin noise_;

#ifdef PPP_DOUBLE
template <>
struct math_func<double>
{

    // ---------------------- logic  -----------------------------

    static void assign_1		(double ** v) { *v[0] = *v[1]; }
    static void neg_assign_1	(double ** v) { *v[0] = -*v[1]; }

    /** special case for setting variables. v[0] = v[1] = v[2] */
    static void assign_2		(double ** v) { *v[0] = *v[1] = *v[2]; }

    /** v[0] = v[1]!=0? v[2] : v[3] */
    static void assign_if_3		(double ** v) { *v[0] = *v[1] != 0? *v[2] : *v[3]; }

    static void abs_1			(double ** v) { *v[0] = std::abs(*v[1]); }
    static void sign_1			(double ** v) { *v[0] = *v[1] > 0.0? -1.0 : *v[1] < 0.0? -1.0 : 0.0; }
    static void floor_1			(double ** v) { *v[0] = std::floor(*v[1]); }
    static void ceil_1			(double ** v) { *v[0] = std::ceil(*v[1]); }
    static void round_1			(double ** v) { *v[0] = std::floor(*v[1] + 0.5); }
    static void frac_1			(double ** v) { *v[0] = *v[1] - std::floor(*v[1]); }

    static void min_2			(double ** v) { *v[0] = std::min(*v[1], *v[2]); }
    static void max_2			(double ** v) { *v[0] = std::max(*v[1], *v[2]); }
    static void clamp_3			(double ** v) { *v[0] = std::min(std::max(*v[1], *v[2]), *v[3]); }
    static void quant_2			(double ** v) { *v[0] = std::floor(*v[1] / *v[2]) * *v[2]; }
    static void mod_2			(double ** v) { *v[0] = std::fmod(*v[1], *v[2]); }
    static void smod_2			(double ** v) { *v[0] = (*v[0]<0.0)? std::fmod(*v[1], *v[2]) : *v[2] - std::fmod(-*v[1], *v[2]); }

    // ----------------- binary operator -------------------------

    static void add_2			(double ** v) { *v[0] = *v[1] + *v[2]; }
    static void sub_2			(double ** v) { *v[0] = *v[1] - *v[2]; }
    static void mul_2			(double ** v) { *v[0] = *v[1] * *v[2]; }
    static void div_2			(double ** v) { *v[0] = *v[1] / *v[2]; }

    static void add_1			(double ** v) { *v[0] += *v[1]; }
    static void sub_1			(double ** v) { *v[0] -= *v[1]; }
    static void mul_1			(double ** v) { *v[0] *= *v[1]; }
    static void div_1			(double ** v) { *v[0] /= *v[1]; }

    static void equal_2			(double ** v) { *v[0] = *v[1] == *v[2]; }
    static void not_equal_2		(double ** v) { *v[0] = *v[1] != *v[2]; }
    static void smaller_2		(double ** v) { *v[0] = *v[1] < *v[2]; }
    static void smaller_equal_2	(double ** v) { *v[0] = *v[1] <= *v[2]; }
    static void greater_2		(double ** v) { *v[0] = *v[1] > *v[2]; }
    static void greater_equal_2	(double ** v) { *v[0] = *v[1] >= *v[2]; }

    static void and_2			(double ** v) { *v[0] = ((int)(*v[1])) & ((int)(*v[2])); }
    static void or_2			(double ** v) { *v[0] = ((int)(*v[1])) | ((int)(*v[2])); }
    static void xor_2			(double ** v) { *v[0] = ((int)(*v[1])) ^ ((int)(*v[2])); }

    static void logic_and_2		(double ** v) { *v[0] = ((int)(*v[1])) && ((int)(*v[2])); }
    static void logic_or_2		(double ** v) { *v[0] = ((int)(*v[1])) || ((int)(*v[2])); }
    static void logic_xor_2		(double ** v) { *v[0] = (*v[1]>0.0) ^ (*v[2]>0.0); }

    typedef void (*FuncPtr)(double **);
    /** return operator call or NULL */
    static FuncPtr get_binary_op_func(const std::string& symbol);

    // ------------------- 'scientific' --------------------------

    static void sin_1			(double ** v) { *v[0] = std::sin(*v[1]); }
    static void sinh_1			(double ** v) { *v[0] = std::sinh(*v[1]); }
    static void asin_1			(double ** v) { *v[0] = std::asin(*v[1]); }
    static void cos_1			(double ** v) { *v[0] = std::cos(*v[1]); }
    static void cosh_1			(double ** v) { *v[0] = std::cosh(*v[1]); }
    static void acos_1			(double ** v) { *v[0] = std::acos(*v[1]); }
    static void tan_1			(double ** v) { *v[0] = std::tan(*v[1]); }
    static void tanh_1			(double ** v) { *v[0] = std::tanh(*v[1]); }
    static void atan_1			(double ** v) { *v[0] = std::atan(*v[1]); }
    static void atan_2			(double ** v) { *v[0] = std::atan2(*v[1], *v[2]); }
    static void sinc_1			(double ** v) { *v[0] = std::sin(*v[1]) / *v[1]; }

    static void exp_1			(double ** v) { *v[0] = std::exp(*v[1]); }
    static void ln_1			(double ** v) { *v[0] = std::log(*v[1]); }
    static void logistic_1		(double ** v) { *v[0] = 1.0 / (1.0 + std::exp(*v[1])); }

    static void pow_2			(double ** v) { *v[0] = std::pow(*v[1], *v[2]); }
    static void sqrt_1			(double ** v) { *v[0] = std::sqrt(*v[1]); }
    static void root_2			(double ** v) { *v[0] = std::pow(*v[1], 1.0 / *v[2]); }

    // -------------------- common gfx ---------------------------

    static void mix_3           (double ** v) { *v[0] = *v[1] + *v[3] * (*v[2] - *v[1]); }

    static void smoothstep_3    (double ** v) { *v[0] = MO::MATH::smoothstep(*v[1], *v[2], *v[3]); }
    static void smootherstep_3  (double ** v) { *v[0] = MO::MATH::smootherstep(*v[1], *v[2], *v[3]); }

    static void smoothladder_2  (double ** v)
    { *v[0] = MO::MATH::quant(*v[1], *v[2]) + *v[2] * MO::MATH::smoothstep(0.0, *v[2],
                MO::MATH::modulo(*v[1], *v[2])); }

    static void smootherladder_2(double ** v)
    { *v[0] = MO::MATH::quant(*v[1], *v[2]) + *v[2] * MO::MATH::smootherstep(0.0, *v[2],
                MO::MATH::modulo(*v[1], *v[2])); }
    //quant(x,f)+f*smstep(0,f,x%f)

    // -------------------- geometric ----------------------------

    static void beta_1			(double ** v) { double a = 1.0 - *v[1] * *v[1];
                                                *v[0] = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_2			(double ** v) { double a = 1.0 - *v[1] * *v[1] - *v[2] * *v[2];
                                                *v[0] = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_3			(double ** v) { double a = 1.0 - *v[1] * *v[1] - *v[2] * *v[2] - *v[3] * *v[3];
                                                *v[0] = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_4			(double ** v) { double a = 1.0 - *v[1] * *v[1] - *v[2] * *v[2] - *v[3] * *v[3] - *v[4] * *v[4];
                                                *v[0] = a > 0.0? std::sqrt(a) : 0.0; }

    static void mag_2			(double ** v) { *v[0] = std::sqrt(*v[1] * *v[1] + *v[2] * *v[2]); }
    static void mag_3			(double ** v) { *v[0] = std::sqrt(*v[1] * *v[1] + *v[2] * *v[2] + *v[3] * *v[3]); }
    static void mag_4			(double ** v) { *v[0] = std::sqrt(*v[1] * *v[1] + *v[2] * *v[2] + *v[3] * *v[3] + *v[4] * *v[4]); }

    static void dist_4			(double ** v) { *v[0] = std::sqrt(std::pow(*v[3] - *v[1], 2.0) + std::pow(*v[4] - *v[2], 2.0)); }

    /** rotate(x,y,radians) */
    static void rotater_3       (double ** v)
    {
        const double ca = cos(*v[3]), sa = sin(*v[3]);
        *v[0] = *v[1] * ca - *v[2] * sa;
    }

    /** rotate(x,y,degree) */
    static void rotate_3        (double ** v)
    {
        const double r = *v[3] / 180.0 * PI, ca = cos(r), sa = sin(r);
        *v[0] = *v[1] * ca - *v[2] * sa;
    }

    // ----------------- oscillator ------------------------------

    static void ramp_1			(double ** v)
        { *v[0] = MO::MATH::moduloSigned( *v[1], 1.0 ); }

    static void saw_1			(double ** v)
        { *v[0] = -1.0 + 2.0 * MO::MATH::moduloSigned( *v[1], 1.0 ); }

    static void square_1		(double ** v)
        { *v[0] = (MO::MATH::moduloSigned( *v[1], 1.0 ) >= 0.5) ? -1.0 : 1.0 ; }

    static void tri_1			(double ** v)
    {
        double p;
        p = MO::MATH::moduloSigned(*v[1], 1.0);
        *v[0] = (p<0.5) ? (p * 4. - 1.) : (3. - p * 4.);
    }

    // with pulsewidth
    static void square_2		(double ** v)
        { *v[0] = (MO::MATH::moduloSigned( *v[1], 1.0 ) >= *v[2]) ? -1.0 : 1.0 ; }

    static void tri_2			(double ** v)
    {
        double p;
        p = MO::MATH::moduloSigned(*v[1], 1.0);
        *v[0] = (p<*v[2])? p * 2.0/ *v[2] - 1.0 : (1.0-p) * 2.0/(1.0-*v[2]) - 1.0;
    }

    // -------------- random ------------------------------------

    static void rnd_0			(double ** v) { *v[0] = (double)std::rand() / RAND_MAX; }

    static void noise_1         (double ** v) { *v[0] = noise_.noise(*v[1]); }
    static void noise_2         (double ** v) { *v[0] = noise_.noise(*v[1], *v[2]); }
    static void noise_3         (double ** v) { *v[0] = noise_.noise(*v[1], *v[2], *v[3]); }
    static void noiseoct_2      (double ** v)
        { *v[0] = noise_.noiseoct(*v[1], std::min((uint)10, (uint)*v[2])); }
    static void noiseoct_3      (double ** v)
        { *v[0] = noise_.noiseoct(*v[1], *v[2], std::min((uint)10, (uint)*v[3])); }
    static void noiseoct_4      (double ** v)
        { *v[0] = noise_.noiseoct(*v[1], *v[2], *v[3], std::min((uint)10, (uint)*v[4])); }

    // -------------- fractal ----------------------------------

    // takes x,y position, returns z
    static void mandel_2        (double ** v)
    {
        double
            x0 = *v[1],
            y0 = *v[2],
            x = 0.0,
            y = 0.0,
            tmp = 0.0,
            z = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const double x2 = x*x,
                         y2 = y*y;
            z = x2 + y2;
            if (z > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        *v[0] = sqrt(z);
    }

    // takes x,y position, returns iteration
    static void mandeli_2        (double ** v)
    {
        double
            x0 = *v[1],
            y0 = *v[2],
            x = 0.0,
            y = 0.0,
            tmp = 0.0;
        int iter = 0;
        while (iter < 1000)
        {
            const double x2 = x*x,
                         y2 = y*y;
            if (x2 + y2 > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        *v[0] = iter;
    }

    static void mandel_3        (double ** v)
    {
        double
            x0 = *v[1],
            y0 = *v[2],
            x = 0.0,
            y = 0.0,
            tmp = 0.0,
            z = 0.0;
        int iter = 0, maxiter = *v[3];
        while (iter < maxiter)
        {
            const double x2 = x*x,
                         y2 = y*y;
            z = x2 + y2;
            if (z > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        *v[0] = sqrt(z);
    }

    // takes x,y position and maxiter, returns iteration
    static void mandeli_3        (double ** v)
    {
        double
            x0 = *v[1],
            y0 = *v[2],
            x = 0.0,
            y = 0.0,
            tmp = 0.0;
        int iter = 0, maxiter = *v[3];
        while (iter < maxiter)
        {
            const double x2 = x*x,
                         y2 = y*y;
            if (x2 + y2 > 2*2) break;
            tmp = x2 - y2 + x0;
              y = 2.0*x*y + y0;
            x = tmp;
            ++iter;
        }
        *v[0] = iter;
    }


    // ---------------- number theory ---------------

    static void odd_1			(double ** v) { *v[0] = int(*v[1]) & 1; }
    static void even_1			(double ** v) { *v[0] = !(int(*v[1]) & 1); }

    static void quer_1			(double ** v)
    {
        int k = *v[1];
        int q = 0;
        do
        {
            if (k<10)
            {
                q += k;
                *v[0] = q;
                return;
            }
            else
            {
                q += k % 10;
                k /= 10;
            }
        }
        while (true);
    }


    static void prime_1			(double ** v)
    {
        *v[0] = generic_int<Int>::isPrime(std::abs(static_cast<Int>(*v[1])));
        /*
        typedef long int I;
        const I k = std::abs(static_cast<I>(*v[1]));
        if (k < 2 || k == 4) { *v[0] = 0.0; return; } else
        if (k == 2) { *v[0] = 1.0; return; } else
        // even?
        if (!(k & 1)) { *v[0] = 0.0; return; }
        const I m = sqrt(k)+1;
        // test divisors
        for (I i=2; i<m; ++i)
            if (k % i == 0) { *v[0] = 0.0; return; }

        *v[0] = 1.0;
        */
    }

    static void sprime_1			(double ** v)
    {
        typedef long int I;
        const I k = *v[1];
        if (k < 2 || k == 4) { *v[0] = 0.0; return; } else
        if (k == 2) { *v[0] = 1.0; return; } else
        // even?
        if (!(k & 1)) { *v[0] = 0.0; return; }
        const I m = sqrt(k)+1;
        // test divisors
        for (I i=2; i<m; ++i)
            if (k % i == 0) { *v[0] = 0.0; return; }

        *v[0] = 1.0;
    }


    static void divisor_2			(double ** v)
    {
        *v[0] = generic_int<Int>::divisor( std::abs((Int)*v[1]), std::abs((Int)*v[2]) );
    }

    static void numdiv_1			(double ** v)
    {
        *v[0] = generic_int<Int>::num_div( std::abs((Int)*v[1]) );
    }

    static void sumdiv_1			(double ** v)
    {
        *v[0] = generic_int<Int>::sum_div( std::abs((Int)*v[1]) );
    }

    static void proddiv_1			(double ** v)
    {
        *v[0] = generic_int<Int>::prod_div( std::abs((Int)*v[1]) );
    }

    static void nextdiv_2			(double ** v)
    {
        *v[0] = generic_int<Int>::next_div( *v[1], *v[2] );
    }

    static void gcd_2				(double ** v)
    {
        *v[0] = generic_int<Int>::gcd( *v[1], *v[2] );
    }

    static void congruent_3			(double ** v)
    {
        *v[0] = generic_int<Int>::congruent( *v[1], *v[2], *v[3] );
    }

    static void factorial_1			(double ** v)
    {
        *v[0] = generic_int<Int>::factorial( *v[1] );
    }

    static void fibonacci_1		(double ** v)
    {
        *v[0] = generic_int<Int>::fibonacci( *v[1] );
    }

    static void ulam_spiral_2	(double ** v)
    {
        *v[0] = generic_int<Int>::ulam_spiral(*v[1], *v[2]);
    }

    static void ulam_spiral_3	(double ** v)
    {
        *v[0] = generic_int<Int>::ulam_spiral(*v[1], *v[2], *v[3]);
    }

    static void tri_spiral_2	(double ** v)
    {
        *v[0] = generic_int<Int>::tri_spiral(*v[1], *v[2]);
    }

    // ------------ smoothed number theory --------------

    static void s_prime_1			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::isPrime( std::abs((Int)*v[1]) ),
                    (double)generic_int<Int>::isPrime( std::abs((Int)*v[1] + 1) ));
    }

    static void s_numdiv_1			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::num_div( std::abs((Int)*v[1]) ),
                    (double)generic_int<Int>::num_div( std::abs((Int)*v[1] + 1) ));
    }

    static void s_divisor_2			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
            MO::MATH::frac(*v[1]),
            (double)generic_int<Int>::divisor( std::abs((Int)*v[1]), std::abs((Int)*v[2]) ),
            (double)generic_int<Int>::divisor( std::abs((Int)*v[1]+1), std::abs((Int)*v[2]) ));
    }

    static void s_sumdiv_1			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::sum_div( std::abs((Int)*v[1]) ),
                    (double)generic_int<Int>::sum_div( std::abs((Int)*v[1] + 1) ));
    }

    static void s_proddiv_1			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::prod_div( std::abs((Int)*v[1]) ),
                    (double)generic_int<Int>::prod_div( std::abs((Int)*v[1] + 1) ));
    }

    static void s_nextdiv_2			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::next_div( *v[1], *v[2] ),
                    (double)generic_int<Int>::next_div( *v[1] + 1, *v[2] ) );
    }

    static void s_gcd_2				(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::gcd( *v[1], *v[2] ),
                    (double)generic_int<Int>::gcd( *v[1] + 1, *v[2] ) );
    }

    static void s_congruent_3			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::congruent( *v[1], *v[2], *v[3] ),
                    (double)generic_int<Int>::congruent( *v[1] + 1, *v[2], *v[3] ) );
    }

    static void s_factorial_1			(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::factorial( *v[1] ),
                    (double)generic_int<Int>::factorial( *v[1] + 1 ));
    }

    static void s_digits_1      		(double ** v)
    {
        *v[0] = MO::MATH::interpol_smooth(
                    MO::MATH::frac(*v[1]),
                    (double)generic_int<Int>::num_digits( *v[1] ),
                    (double)generic_int<Int>::num_digits( *v[1] + 1 ));
    }

    // ------------- float number theory ----------------

    static void zeta_1			(double ** v)
    {
        *v[0] = 0.0;
        double s = -*v[1];
        for (double i=1.0; i<70.0; ++i)
            *v[0] += std::pow(i, s);
    }

    static void zetap_2			(double ** v)
    {
        *v[0] = 0.0;
        double i = 1.0, k, s = -*v[1];
        do
        {
            k = std::pow(i, s);
            *v[0] += k;
        } while (k > *v[2] && ++i <= 200000);
    }


    static void digits_1 		(double ** v)
    {
        *v[0] = generic_int<Int>::num_digits( *v[1] );
    }

    static void harmo_2			(double ** v)
    {
        if (*v[1] == 0.0 || *v[2] == 0.0) { *v[0] = 0.0; return; }
        double f = *v[1] / *v[2];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[2] / *v[1];
        *v[0] = (f == floor(f))? f : 0.0;
    }

    static void harmo_3			(double ** v)
    {
        if (*v[1] == 0.0 || *v[2] == 0.0 || *v[3] == 0.0) { *v[0] = 0.0; return; }
        double
        f = *v[1] / *v[2] / *v[3];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[1] / *v[3] / *v[2];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[2] / *v[1] / *v[3];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[2] / *v[3] / *v[1];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[3] / *v[1] / *v[2];
        if (f == floor(f)) { *v[0] = f; return; }
        f = *v[3] / *v[2] / *v[1];
        *v[0] = (f == floor(f))? f : 0.0;
    }

};

inline math_func<double>::FuncPtr
math_func<double>::get_binary_op_func(const std::string& symbol)
{
    if (symbol == "=") return math_func<double>::assign_2; else

    if (symbol == "+") return math_func<double>::add_2; else
    if (symbol == "-") return math_func<double>::sub_2; else
    if (symbol == "*") return math_func<double>::mul_2; else
    if (symbol == "/") return math_func<double>::div_2; else

    if (symbol == "%") return math_func<double>::mod_2; else
    if (symbol == "^") return math_func<double>::pow_2; else

    if (symbol == "==") return math_func<double>::equal_2; else
    if (symbol == "!=") return math_func<double>::not_equal_2; else
    if (symbol == "<") return math_func<double>::smaller_2; else
    if (symbol == "<=") return math_func<double>::smaller_equal_2; else
    if (symbol == ">") return math_func<double>::greater_2; else
    if (symbol == ">=") return math_func<double>::greater_equal_2; else

    if (symbol == "&") return math_func<double>::and_2; else
    if (symbol == "|") return math_func<double>::or_2; else
    if (symbol == "xor") return math_func<double>::xor_2; else

    if (symbol == "&&") return math_func<double>::logic_and_2; else
    if (symbol == "||") return math_func<double>::logic_or_2; else
    if (symbol == "^^") return math_func<double>::logic_xor_2; else

    return 0;
}

#endif



} // namespace PPP_NAMESPACE


#endif // FUNCTIONS_H_DEFINED
