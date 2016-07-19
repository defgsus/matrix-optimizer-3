/**	@file funcparser/functions.h

    @brief static functions for use with math parser

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef FUNCTIONS_H_DEFINED
#define FUNCTIONS_H_DEFINED

#include "parser_defines.h"
#include "math/functions.h"
#include "math/NoisePerlin.h"
#include "math/interpol.h"
#include "math/constants.h"

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
#define RES (*v[0])
#define A (*v[1])
#define B (*v[2])
#define C (*v[3])
#define D (*v[4])

    // ---------------------- logic  -----------------------------

    static void assign_1		(double ** v) { RES = A; }
    static void neg_assign_1	(double ** v) { RES = -A; }

    /** special case for setting variables. v[0] = v[1] = v[2] */
    static void assign_2		(double ** v) { RES = A = B; }

    /** v[0] = v[1]!=0? v[2] : v[3] */
    static void assign_if_3		(double ** v) { RES = A != 0? B : C; }

    static void abs_1			(double ** v) { RES = std::abs(A); }
    static void sign_1			(double ** v) { RES = A > 0.0? 1.0 : A < 0.0? -1.0 : 0.0; }
    static void floor_1			(double ** v) { RES = std::floor(A); }
    static void ceil_1			(double ** v) { RES = std::ceil(A); }
    static void round_1			(double ** v) { RES = std::floor(A + 0.5); }
    static void frac_1			(double ** v) { RES = A - std::floor(A); }

    static void min_2			(double ** v) { RES = std::min(A, B); }
    static void max_2			(double ** v) { RES = std::max(A, B); }
    static void clamp_3			(double ** v) { RES = std::min(std::max(A, B), C); }
    static void quant_2			(double ** v) { RES = std::floor(A / B) * B; }
    static void mod_2			(double ** v) { RES = std::fmod(A, B); }
    static void smod_2			(double ** v)
        { RES = (A >= 0.0)? std::fmod(A, B) : B - std::fmod(-A, B); }

    // ----------------- binary operator -------------------------

    static void add_2			(double ** v) { RES = A + B; }
    static void sub_2			(double ** v) { RES = A - B; }
    static void mul_2			(double ** v) { RES = A * B; }
    static void div_2			(double ** v) { RES = A / B; }

    static void add_1			(double ** v) { RES += A; }
    static void sub_1			(double ** v) { RES -= A; }
    static void mul_1			(double ** v) { RES *= A; }
    static void div_1			(double ** v) { RES /= A; }

    static void equal_2			(double ** v) { RES = A == B; }
    static void not_equal_2		(double ** v) { RES = A != B; }
    static void smaller_2		(double ** v) { RES = std::isless(A, B); }
    static void smaller_equal_2	(double ** v) { RES = std::islessequal(A, B); }
    static void greater_2		(double ** v) { RES = std::isgreater(A, B); }
    static void greater_equal_2	(double ** v) { RES = std::isgreaterequal(A, B); }

    static void and_2			(double ** v) { RES = ((int)(A)) & ((int)(B)); }
    static void or_2			(double ** v) { RES = ((int)(A)) | ((int)(B)); }
    static void xor_2			(double ** v) { RES = ((int)(A)) ^ ((int)(B)); }

    static void logic_and_2		(double ** v) { RES = ((int)(A)) && ((int)(B)); }
    static void logic_or_2		(double ** v) { RES = ((int)(A)) || ((int)(B)); }
    static void logic_xor_2		(double ** v) { RES = (A>0.0) ^ (B>0.0); }

    typedef void (*FuncPtr)(double **);
    /** return operator call or NULL */
    static FuncPtr get_binary_op_func(const std::string& symbol);

    // ------------------- 'scientific' --------------------------

    static void sin_1			(double ** v) { RES = std::sin(A); }
    static void sinh_1			(double ** v) { RES = std::sinh(A); }
    static void asin_1			(double ** v) { RES = std::asin(A); }
    static void asinh_1			(double ** v) { RES = std::asinh(A); }
    static void cos_1			(double ** v) { RES = std::cos(A); }
    static void cosh_1			(double ** v) { RES = std::cosh(A); }
    static void acos_1			(double ** v) { RES = std::acos(A); }
    static void acosh_1			(double ** v) { RES = std::acosh(A); }
    static void tan_1			(double ** v) { RES = std::tan(A); }
    static void tanh_1			(double ** v) { RES = std::tanh(A); }
    static void atan_1			(double ** v) { RES = std::atan(A); }
    static void atan_2			(double ** v) { RES = std::atan2(A, B); }
    static void atanh_1			(double ** v) { RES = std::atanh(A); }
    static void sinc_1			(double ** v) { RES = std::sin(A) / A; }

    static void exp_1			(double ** v) { RES = std::exp(A); }
    static void log_1			(double ** v) { RES = std::log(A); }
    static void log2_1			(double ** v) { RES = std::log2(A); }
    static void log10_1			(double ** v) { RES = std::log10(A); }

    static void pow_2			(double ** v) { RES = std::pow(A, B); }
    static void sqrt_1			(double ** v) { RES = std::sqrt(A); }
    static void root_2			(double ** v) { RES = std::pow(A, 1.0 / B); }

    // ------------------- statistics ----------------------------

    static void logistic_1		(double ** v) { RES = 1.0 / (1.0 + std::exp(-A)); }
    static void erf_1       	(double ** v) { RES = std::erf(A); }
    static void erfc_1       	(double ** v) { RES = std::erfc(A); }
    // gauss(x, dev)
    static void gauss_2       	(double ** v) { RES = std::exp(-((A*A)/(2.0*B*B))); }
    // gauss(x, dev, center)
    static void gauss_3       	(double ** v) { RES = std::exp(-(std::pow(A-C,2)/(2.0*B*B))); }
    // cauchy(x, dev)
    static void cauchy_2       	(double ** v) { RES = (1.0/PI) * (B/(A*A + B*B)); }
    // cauchy(x, dev, center)
    static void cauchy_3       	(double ** v) { RES = (1.0/PI) * (B/(std::pow(A-C,2)+ B*B)); }


    // -------------------- common gfx ---------------------------

    static void mix_3           (double ** v) { RES = A + C * (B - A); }

    static void smoothstep_3    (double ** v) { RES = MO::MATH::smoothstep(A, B, C); }
    static void smootherstep_3  (double ** v) { RES = MO::MATH::smootherstep(A, B, C); }

    static void smoothladder_2  (double ** v)
    { RES = MO::MATH::quant(A, B) + B * MO::MATH::smoothstep(0.0, B,
                MO::MATH::modulo(A, B)); }

    static void smootherladder_2(double ** v)
    { RES = MO::MATH::quant(A, B) + B * MO::MATH::smootherstep(0.0, B,
                MO::MATH::modulo(A, B)); }
    //quant(x,f)+f*smstep(0,f,x%f)

    // -------------------- geometric ----------------------------

    static void beta_1			(double ** v) { double a = 1.0 - A * A;
                                                RES = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_2			(double ** v) { double a = 1.0 - A * A - B * B;
                                                RES = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_3			(double ** v) { double a = 1.0 - A * A - B * B - C * C;
                                                RES = a > 0.0? std::sqrt(a) : 0.0; }
    static void beta_4			(double ** v) { double a = 1.0 - A * A - B * B - C * C - D * D;
                                                RES = a > 0.0? std::sqrt(a) : 0.0; }

    static void mag_2			(double ** v) { RES = std::sqrt(A * A + B * B); }
    static void mag_3			(double ** v) { RES = std::sqrt(A * A + B * B + C * C); }
    static void mag_4			(double ** v) { RES = std::sqrt(A * A + B * B + C * C + D * D); }

    static void dist_4			(double ** v) { RES = std::sqrt(std::pow(C - A, 2.0) + std::pow(D - B, 2.0)); }

    /** rotate(x,y,radians) */
    static void rotater_3       (double ** v)
    {
        const double ca = cos(C), sa = sin(C);
        RES = A * ca - B * sa;
    }

    /** rotate(x,y,degree) */
    static void rotate_3        (double ** v)
    {
        const double r = C / 180.0 * PI, ca = cos(r), sa = sin(r);
        RES = A * ca - B * sa;
    }

    // ----------------- oscillator ------------------------------

    static void ramp_1			(double ** v)
        { RES = MO::MATH::moduloSigned( A, 1.0 ); }

    static void saw_1			(double ** v)
        { RES = -1.0 + 2.0 * MO::MATH::moduloSigned( A, 1.0 ); }

    static void square_1		(double ** v)
        { RES = (MO::MATH::moduloSigned( A, 1.0 ) >= 0.5) ? -1.0 : 1.0 ; }

    static void tri_1			(double ** v)
    {
        double p;
        p = MO::MATH::moduloSigned(A, 1.0);
        RES = (p<0.5) ? (p * 4. - 1.) : (3. - p * 4.);
    }

    // with pulsewidth
    static void square_2		(double ** v)
        { RES = (MO::MATH::moduloSigned( A, 1.0 ) >= B) ? -1.0 : 1.0 ; }

    static void tri_2			(double ** v)
    {
        double p;
        p = MO::MATH::moduloSigned(A, 1.0);
        RES = (p<B)? p * 2.0/ B - 1.0 : (1.0-p) * 2.0/(1.0-B) - 1.0;
    }

    static void note2freq_1     (double ** v)
    {
        const double o = std::pow(2.0, 1.0 / 12.0);
        RES = std::pow(o, A) * 16.35155;
    }

    // -------------- random ------------------------------------

    static void rnd_0			(double ** v) { RES = (double)std::rand() / RAND_MAX; }

    static void noise_1         (double ** v) { RES = noise_.noise(A); }
    static void noise_2         (double ** v) { RES = noise_.noise(A, B); }
    static void noise_3         (double ** v) { RES = noise_.noise(A, B, C); }
    static void noiseoct_2      (double ** v)
        { RES = noise_.noiseoct(A, std::min((uint)10, (uint)B)); }
    static void noiseoct_3      (double ** v)
        { RES = noise_.noiseoct(A, B, std::min((uint)10, (uint)C)); }
    static void noiseoct_4      (double ** v)
        { RES = noise_.noiseoct(A, B, C, std::min((uint)10, (uint)D)); }

    // -------------- fractal ----------------------------------

    // takes x,y position, returns z
    static void mandel_2        (double ** v)
    {
        double
            x0 = A,
            y0 = B,
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
        RES = sqrt(z);
    }

    // takes x,y position, returns iteration
    static void mandeli_2        (double ** v)
    {
        double
            x0 = A,
            y0 = B,
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
        RES = iter;
    }

    static void mandel_3        (double ** v)
    {
        double
            x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0,
            z = 0.0;
        int iter = 0, maxiter = C;
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
        RES = std::sqrt(z);
    }

    // takes x,y position and maxiter, returns iteration
    static void mandeli_3        (double ** v)
    {
        double
            x0 = A,
            y0 = B,
            x = 0.0,
            y = 0.0,
            tmp = 0.0;
        int iter = 0, maxiter = C;
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
        RES = iter;
    }

    // takes xstart, ystart, x, y and returns value
    static void julia_4        (double ** v)
    {
        double
            x0 = C,
            y0 = D,
            x = A,
            y = B,
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
        RES = std::sqrt(z);
    }

    // takes xstart, ystart, x, y and returns iteration
    static void juliai_4        (double ** v)
    {
        double
            x0 = C,
            y0 = D,
            x = A,
            y = B,
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
        RES = iter;
    }

    // takes xstart, ystart, x, y and returns value
    static void duckball_4   (double ** v)
    {
        double
            x0 = C,
            y0 = D,
            x = A,
            y = B,
            z = 0.0;
        int iter = 0;
        while (iter < 200)
        {
            z = x*x + y*y;
            if (z < 0.0001) break;
            x = std::abs(x) / z + x0;
            y = std::abs(y) / z + y0;
            ++iter;
        }
        RES = std::sqrt(z);
    }

    // ---------------- number theory ---------------

    static void odd_1			(double ** v) { RES = int(A) & 1; }
    static void even_1			(double ** v) { RES = !(int(A) & 1); }

    static void quer_1			(double ** v)
    {
        int k = A;
        int q = 0;
        do
        {
            if (k<10)
            {
                q += k;
                RES = q;
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
        RES = generic_int<Int>::isPrime(std::abs(static_cast<Int>(A)));
        /*
        typedef long int I;
        const I k = std::abs(static_cast<I>(A));
        if (k < 2 || k == 4) { RES = 0.0; return; } else
        if (k == 2) { RES = 1.0; return; } else
        // even?
        if (!(k & 1)) { RES = 0.0; return; }
        const I m = sqrt(k)+1;
        // test divisors
        for (I i=2; i<m; ++i)
            if (k % i == 0) { RES = 0.0; return; }

        RES = 1.0;
        */
    }

    static void sprime_1			(double ** v)
    {
        typedef long int I;
        const I k = A;
        if (k < 2 || k == 4) { RES = 0.0; return; } else
        if (k == 2) { RES = 1.0; return; } else
        // even?
        if (!(k & 1)) { RES = 0.0; return; }
        const I m = sqrt(k)+1;
        // test divisors
        for (I i=2; i<m; ++i)
            if (k % i == 0) { RES = 0.0; return; }

        RES = 1.0;
    }


    static void divisor_2			(double ** v)
    {
        RES = generic_int<Int>::divisor( std::abs((Int)A), std::abs((Int)B) );
    }

    static void numdiv_1			(double ** v)
    {
        RES = generic_int<Int>::num_div( std::abs((Int)A) );
    }

    static void sumdiv_1			(double ** v)
    {
        RES = generic_int<Int>::sum_div( std::abs((Int)A) );
    }

    static void proddiv_1			(double ** v)
    {
        RES = generic_int<Int>::prod_div( std::abs((Int)A) );
    }

    static void nextdiv_2			(double ** v)
    {
        RES = generic_int<Int>::next_div( A, B );
    }

    static void gcd_2				(double ** v)
    {
        RES = generic_int<Int>::gcd( A, B );
    }

    static void congruent_3			(double ** v)
    {
        RES = generic_int<Int>::congruent( A, B, C );
    }

    static void factorial_1			(double ** v)
    {
        RES = generic_int<Int>::factorial( A );
    }

    static void gamma_1			(double ** v)
    {
        RES = std::tgamma( A );
    }

    static void fibonacci_1		(double ** v)
    {
        RES = generic_int<Int>::fibonacci( A );
    }

    static void ulam_spiral_2	(double ** v)
    {
        RES = generic_int<Int>::ulam_spiral(A, B);
    }

    static void ulam_spiral_3	(double ** v)
    {
        RES = generic_int<Int>::ulam_spiral(A, B, C);
    }

    static void tri_spiral_2	(double ** v)
    {
        RES = generic_int<Int>::tri_spiral(A, B);
    }

    // ------------ smoothed number theory --------------

    static void s_prime_1			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::isPrime( std::abs((Int)A) ),
                    (double)generic_int<Int>::isPrime( std::abs((Int)A + 1) ));
    }

    static void s_numdiv_1			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::num_div( std::abs((Int)A) ),
                    (double)generic_int<Int>::num_div( std::abs((Int)A+1) ));
    }

    static void s_divisor_2			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
            MO::MATH::fract(A),
            (double)generic_int<Int>::divisor( std::abs((Int)A), std::abs((Int)B) ),
            (double)generic_int<Int>::divisor( std::abs((Int)A+1), std::abs((Int)B) ));
    }

    static void s_sumdiv_1			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::sum_div( std::abs((Int)A) ),
                    (double)generic_int<Int>::sum_div( std::abs((Int)A + 1) ));
    }

    static void s_proddiv_1			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::prod_div( std::abs((Int)A) ),
                    (double)generic_int<Int>::prod_div( std::abs((Int)A + 1) ));
    }

    static void s_nextdiv_2			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::next_div( A, B ),
                    (double)generic_int<Int>::next_div( A + 1, B ) );
    }

    static void s_gcd_2				(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::gcd( A, B ),
                    (double)generic_int<Int>::gcd( A + 1, B ) );
    }

    static void s_congruent_3			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::congruent( A, B, C ),
                    (double)generic_int<Int>::congruent( A + 1, B, C ) );
    }

    static void s_factorial_1			(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::factorial( A ),
                    (double)generic_int<Int>::factorial( A + 1 ));
    }

    static void s_digits_1      		(double ** v)
    {
        RES = MO::MATH::interpol_smooth(
                    MO::MATH::fract(A),
                    (double)generic_int<Int>::num_digits( A ),
                    (double)generic_int<Int>::num_digits( A + 1 ));
    }

    // ------------- float number theory ----------------

    static void zeta_1			(double ** v)
    {
        RES = 0.0;
        double s = -A;
        for (double i=1.0; i<70.0; ++i)
            RES += std::pow(i, s);
    }

    static void zetap_2			(double ** v)
    {
        RES = 0.0;
        double i = 1.0, k, s = -A;
        do
        {
            k = std::pow(i, s);
            RES += k;
        } while (k > B && ++i <= 200000);
    }


    static void digits_1 		(double ** v)
    {
        RES = generic_int<Int>::num_digits( A );
    }

    static void harmo_2			(double ** v)
    {
        if (A == 0.0 || B == 0.0) { RES = 0.0; return; }
        double f = A / B;
        if (f == floor(f)) { RES = f; return; }
        f = B / A;
        RES = (f == floor(f))? f : 0.0;
    }

    static void harmo_3			(double ** v)
    {
        if (A == 0.0 || B == 0.0 || C == 0.0) { RES = 0.0; return; }
        double
        f = A / B / C;
        if (f == floor(f)) { RES = f; return; }
        f = A / C / B;
        if (f == floor(f)) { RES = f; return; }
        f = B / A / C;
        if (f == floor(f)) { RES = f; return; }
        f = B / C / A;
        if (f == floor(f)) { RES = f; return; }
        f = C / A / B;
        if (f == floor(f)) { RES = f; return; }
        f = C / B / A;
        RES = (f == floor(f))? f : 0.0;
    }

#undef RES
#undef A
#undef B
#undef C
#undef D
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
