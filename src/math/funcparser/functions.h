/**	@file

	@brief

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef FUNCTIONS_H_DEFINED
#define FUNCTIONS_H_DEFINED

#include <cmath>

namespace PPP_NAMESPACE {

template <typename F>
struct math_func
{
};

template <>
struct math_func<double>
{
	// ---------------------- logic  -----------------------------

	static void assign_1		(double ** v) { *v[0] = *v[1]; }
	static void neg_assign_1	(double ** v) { *v[0] = -*v[1]; }

	static void abs_1			(double ** v) { *v[0] = std::abs(*v[1]); }
	static void sign_1			(double ** v) { *v[0] = *v[1] > 0.0? -1.0 : *v[1] < 0.0? -1.0 : 0.0; }
	static void floor_1			(double ** v) { *v[0] = std::floor(*v[1]); }
	static void ceil_1			(double ** v) { *v[0] = std::ceil(*v[1]); }
	static void round_1			(double ** v) { *v[0] = std::floor(*v[1] + 0.5); }

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

	// ----------------- oscillator ------------------------------

	static void ramp_1			(double ** v)
		{ *v[0] = (*v[1]>=0.f) ? fmodf(*v[1], 1.f) : (1.f - fmodf(-*v[1], 1.f)); }

	static void square_1		(double ** v)
		{ *v[0] = (*v[1]>=0.) ?
			(-1. + 2. * floor(fmodf(*v[1], 1.) + 0.5))
			: (1. - 2. * floor(fmodf(-*v[1], 1.) + 0.5)); }

	static void tri_1			(double ** v)
	{
		Float p;
		p = (*v[1]>=0.) ?	fmodf(*v[1] + 0.25, 1.) : (1. - fmodf(-*v[1] + 0.75, 1.));
		*v[0] = (p<0.5) ? (4. * p - 1.) : (3. - 4. * p);
	}

	static void saw_1			(double ** v)
	{ *v[0] = (*v[1]>=0.) ?
		(-1. + 2. * fmodf(*v[1], 1.))
			: (1. - 2. * fmodf(-*v[1], 1.)); }

	// -------------- random ------------------------------------

	static void rnd_0			(double ** v) { *v[0] = (double)std::rand() / RAND_MAX; }

	// ---------------- number theory ---------------

	static void prime_1			(double ** v)
	{
		if (*v[1] < 2) { *v[1] = 0.0; return; }
		const int k = (int)*v[1];
		// even?
		if (!(k & 1)) { *v[0] = 0.0; return; }
		const int m = k>>1;
		// test divisors
		for (int i=2; i<m; ++i)
			if (k % i == 0) { *v[0] = 0.0; return; }

		*v[0] = 1.0;
	}

	static void numdiv_1			(double ** v)
	{
		typedef long int I;
		const I k = std::abs((long int)*v[1]);
		if (k==0) { *v[0] = 0.0; return; }
		if (k==1) { *v[0] = 1.0; return; }
		*v[0] = 2;
		const I k2 = k >> 1;
		for (I i=2; i<=k2; ++i)
			if (k % i == 0) ++*v[0];
	}

	static void factorial_1		(double ** v)
	{
		*v[0] = 1.0;
		int k = *v[1];
		for (int i=2; i<=k; ++i)
			*v[0] *= i;
	}

	static void fibonacci_1		(double ** v)
	{
		int64_t k = *v[1], f = 1, f0 = 0, f1;
		for (int64_t i=1; i<k; ++i, f1 = f, f += f0, f0 = f1);
		*v[0] = f;
	}

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
		*v[0] = 1.0;
		long int number = *v[1];
		while (number >= 10) { ++*v[0]; number /= 10; };
	}

	static void harmo_2			(double ** v)
	{
		if (*v[1] == 0.0 || *v[2] == 0.0) { *v[0] = 0.0; return; }
		Float f = *v[1] / *v[2];
		if (f == floor(f)) { *v[0] = f; return; }
		f = *v[2] / *v[1];
		*v[0] = (f == floor(f))? f : 0.0;
	}

	static void harmo_3			(double ** v)
	{
		if (*v[1] == 0.0 || *v[2] == 0.0 || *v[3] == 0.0) { *v[0] = 0.0; return; }
		Float
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


} // namespace PPP_NAMESPACE


#endif // FUNCTIONS_H_DEFINED
