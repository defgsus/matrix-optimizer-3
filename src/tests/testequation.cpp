/** @file testequation.cpp

    @brief Math equation tester

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <iostream>

#include "testequation.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"

namespace MO {

TestEquation::TestEquation()
{
}

int TestEquation::run()
{
    return compareTests_();
}

int TestEquation::compareTests_()
{
    using namespace PPP_NAMESPACE;

    int errors = 0, tests = 0;
    Parser p;

    Float x = 23.0, y = 42.0;
    double x_ = 23.0, y_ = 42.0;

    p.variables().add("x", &x, "");
    p.variables().add("y", &y, "");

#ifndef PPP_TTMATH
#	define Abs abs
#	define Sin sin
#	define Cos cos
#	define Tan tan
#	define Pow pow
#endif

    #define PPP_COMPARE_2(cexpression__, expression__) \
    {	\
        std::cout << #expression__ << "\n"; \
        if (!p.parse(#expression__)) \
        { \
            std::cout << "[parsing failed]\n"; \
            ++errors; \
        } \
        else \
        { \
            Float \
                rc = (cexpression__), \
                rp = p.eval(); \
            if (Abs(rc-rp) > 0.001) \
            { \
                std::cout << "[result differs c++ = " << rc << " / ppp = " << rp << "]\n"; \
                ++errors; \
            } \
            else \
                std::cout << "= " << rp << " OK\n"; \
        } \
        ++tests; \
        std::cout << std::endl; \
    }

    #define PPP_COMPARE(expression__) PPP_COMPARE_2(expression__, expression__)

    std::cout << "x = " << x << "\ny = " << y << "\n\n";

    PPP_COMPARE(   1.7 )
    PPP_COMPARE(   1+2-3+4-5+6-7+8 )
    PPP_COMPARE(   1.5+2*3 )
    PPP_COMPARE(   (1.1+2.2)*3.3 )
    PPP_COMPARE(   1.6/2+3*4 )

    PPP_COMPARE(   (((x+1)*x+2)*x+3) )

    PPP_COMPARE(   x > 20 && y < 50 )
#ifndef PPP_GMP
    PPP_COMPARE(   1 xor 3 xor 5 xor 8 )
#endif

#ifndef PPP_GMP
    PPP_COMPARE_2( Sin(PI), sin(PI) )
    PPP_COMPARE_2( Cos(PI*2), cos(PI*2) )
    PPP_COMPARE_2( Sin(Cos(Tan((Float)0.5))), sin(cos(tan(0.5))) )
#endif

    PPP_COMPARE_2( -x*x,         foo(c) { -c }; bar(d) { d*d }; foo(bar(x)) )
#ifndef PPP_GMP
    PPP_COMPARE_2( x*x+pow(y_,y_), foo(c) { c*c }; bar(c) { c^c }; foo(x) + bar(y) )
    PPP_COMPARE_2( x*y+pow(x_,x_), foo(a) { a^a }; foo(a,b) { a*b }; foo(x) + foo(x,y) )
#endif
    PPP_COMPARE_2( -x*x,         foo(a) { bar(a) { a*a }; -bar(a) }; foo(x) )
    PPP_COMPARE_2( 1.5,          foo(a) { prime(a)? a/2 : (a>1? foo(a-1) : 1) }; foo(4)	)
    PPP_COMPARE_2( 1,            Q(x) { v=quer(x); v<10? v : Q(v) }; Q(19) )
    PPP_COMPARE_2( 17,           next_prime(a) { useless=1; prime(a)? a : next_prime(a+1) }; next_prime(14) )
    PPP_COMPARE_2( 2*3*4*5*6*7,  faculty(a) { a>1? a*faculty(a-1) : 1 }; faculty(7) )

    if (errors)
        std::cout << errors << " of " << tests << " failed\n";
    else
        std::cout << "no errors in " << tests << " tests :D\n";

    return errors;
}

} // namespace MO
