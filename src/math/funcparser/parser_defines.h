/**	@file

	@brief

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef PARSER_DEFINES_H_INCLUDED
#define PARSER_DEFINES_H_INCLUDED

#include <functional>

#define PPP_DOUBLE

// use tables in release mode (takes a while to compile)
#ifdef NDEBUG
#   define PPP_USE_NDIV_TABLE
#   define PPP_USE_DIVISORS_TABLE
#endif

namespace PPP_NAMESPACE
{
	class Variables;
	class Functions;

	typedef double Float;
    typedef long long Int;
    struct cast
    {
        static Int toInt (const Float& f) { return static_cast<Int>(f); }
    };

    typedef std::function<void(Float**)> FuncPtr;
    typedef std::function<void(FuncPtr, FuncPtr, Float**)> LambdaFuncPtr;

} // namespace PPP_NAMESPACE

#endif // PARSER_DEFINES_H_INCLUDED
