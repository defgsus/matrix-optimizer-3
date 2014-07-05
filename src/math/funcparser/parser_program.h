/**	@file

	@brief Math Function Parser - private functions and classes

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef PARSER_PROGRAM_H_INCLUDED
#define PARSER_PROGRAM_H_INCLUDED

#include <cassert>
#include "functions.h"
#include <functional>

#include "grammar.parser.hh"
#include "parser.h"

#if 0
#	define PPP_PROG_DEBUG(stream_arg__) std::cout << stream_arg__ << "\n";
#else
#	define PPP_PROG_DEBUG(unused__)
#endif

#define PPP_PROG_ERROR(stream_arg__) std::cerr << stream_arg__ << "\n";

/** used to signal erros from inside yyparse() */
#define PPP_ERR(loc__, stream_arg__) \
{	std::cerr << "ERROR: " << PPP_NAMESPACE::location_string(loc__) << ": " << stream_arg__ << "\n"; \
	YYERROR; }


namespace PPP_NAMESPACE
{

	static inline void create_functions(Functions & f)
	{
		f.add(Function::BINARY_OP,		2, "+",			math_func<Float>::add_2			);
		f.add(Function::BINARY_OP,		2, "-",			math_func<Float>::sub_2			);
		f.add(Function::BINARY_OP,		2, "*",			math_func<Float>::mul_2			);
		f.add(Function::BINARY_OP,		2, "/",			math_func<Float>::div_2			);
		f.add(Function::BINARY_OP,		2, "^",			math_func<Float>::pow_2			);
		f.add(Function::BINARY_OP,		2, "%",			math_func<Float>::mod_2			);

		f.add(Function::UNARY_LEFT_OP,	1, "-",			math_func<Float>::neg_assign_1	);
		f.add(Function::UNARY_RIGHT_OP,	1, "!",			math_func<Float>::factorial_1	);

		f.add(Function::BINARY_OP,		2, "&",			math_func<Float>::and_2			);
		f.add(Function::BINARY_OP,		2, "|",			math_func<Float>::or_2			);
		f.add(Function::BINARY_OP,		2, "xor",		math_func<Float>::xor_2			);
		f.add(Function::BINARY_OP,		2, "&&",		math_func<Float>::logic_and_2	);
		f.add(Function::BINARY_OP,		2, "||",		math_func<Float>::logic_or_2	);
		f.add(Function::BINARY_OP,		2, "^^",		math_func<Float>::logic_xor_2	);
		f.add(Function::BINARY_OP,		2, "==",		math_func<Float>::equal_2		);
		f.add(Function::BINARY_OP,		2, "!=",		math_func<Float>::not_equal_2	);
		f.add(Function::BINARY_OP,		2, "<",			math_func<Float>::smaller_2		);
		f.add(Function::BINARY_OP,		2, "<=",		math_func<Float>::smaller_equal_2);
		f.add(Function::BINARY_OP,		2, ">",			math_func<Float>::greater_2		);
		f.add(Function::BINARY_OP,		2, ">=",		math_func<Float>::greater_equal_2);


		f.add(Function::FUNCTION,		1, "abs",		math_func<Float>::abs_1			);
		f.add(Function::FUNCTION,		1, "sign",		math_func<Float>::sign_1		);
		f.add(Function::FUNCTION,		1, "floor",		math_func<Float>::floor_1		);
		f.add(Function::FUNCTION,		1, "ceil",		math_func<Float>::ceil_1		);
		f.add(Function::FUNCTION,		1, "round",		math_func<Float>::round_1		);

		f.add(Function::FUNCTION,		1, "sin",		math_func<Float>::sin_1			);
		f.add(Function::FUNCTION,		1, "sinh",		math_func<Float>::sinh_1		);
		f.add(Function::FUNCTION,		1, "asin",		math_func<Float>::asin_1		);
		f.add(Function::FUNCTION,		1, "cos",		math_func<Float>::cos_1			);
		f.add(Function::FUNCTION,		1, "cosh",		math_func<Float>::cosh_1		);
		f.add(Function::FUNCTION,		1, "acos",		math_func<Float>::acos_1		);
		f.add(Function::FUNCTION,		1, "sinc",		math_func<Float>::sinc_1		);
		f.add(Function::FUNCTION,		1, "tan",		math_func<Float>::tan_1			);
		f.add(Function::FUNCTION,		1, "tanh",		math_func<Float>::tanh_1		);
		f.add(Function::FUNCTION,		1, "atan",		math_func<Float>::atan_1		);
		f.add(Function::FUNCTION,		2, "atan",		math_func<Float>::atan_2		);

		f.add(Function::FUNCTION,		1, "beta",		math_func<Float>::beta_1		);
		f.add(Function::FUNCTION,		2, "beta",		math_func<Float>::beta_2		);
		f.add(Function::FUNCTION,		3, "beta",		math_func<Float>::beta_3		);
		f.add(Function::FUNCTION,		4, "beta",		math_func<Float>::beta_4		);

		f.add(Function::FUNCTION,		2, "mag",		math_func<Float>::mag_2			);
		f.add(Function::FUNCTION,		3, "mag",		math_func<Float>::mag_3			);
		f.add(Function::FUNCTION,		4, "mag",		math_func<Float>::mag_4			);
		f.add(Function::FUNCTION,		4, "dist",		math_func<Float>::dist_4		);

		f.add(Function::FUNCTION,		2, "mod",		math_func<Float>::mod_2			);
		f.add(Function::FUNCTION,		2, "smod",		math_func<Float>::smod_2		);

		f.add(Function::FUNCTION,		1, "exp",		math_func<Float>::exp_1			);
		f.add(Function::FUNCTION,		1, "ln",		math_func<Float>::ln_1			);
		f.add(Function::FUNCTION,		2, "logistic",	math_func<Float>::logistic_1	);
		f.add(Function::FUNCTION,		2, "pow",		math_func<Float>::pow_2			);
		f.add(Function::FUNCTION,		1, "sqrt",		math_func<Float>::sqrt_1		);
		f.add(Function::FUNCTION,		2, "root",		math_func<Float>::root_2		);

		f.add(Function::FUNCTION,		0, "rnd",		math_func<Float>::rnd_0			);

		f.add(Function::FUNCTION,		1, "fac",		math_func<Float>::factorial_1	);
		f.add(Function::FUNCTION,		1, "fib",		math_func<Float>::fibonacci_1	);

		f.add(Function::FUNCTION,		1, "zeta",		math_func<Float>::zeta_1		);
		f.add(Function::FUNCTION,		2, "zeta",		math_func<Float>::zetap_2		);

		f.add(Function::FUNCTION,		2, "harmo",		math_func<Float>::harmo_2		);
		f.add(Function::FUNCTION,		3, "harmo",		math_func<Float>::harmo_3		);
		f.add(Function::FUNCTION,		1, "prime",		math_func<Float>::prime_1		);

		f.add(Function::FUNCTION,		1, "ndiv",		math_func<Float>::numdiv_1		);
		f.add(Function::FUNCTION,		1, "digits",	math_func<Float>::digits_1		);

	}


	std::string location_string(const YYLTYPE& loc)
	{
		std::stringstream s;
		if (loc.first_line != loc.last_line)
			s << loc.first_line << "-" << loc.last_line << ":";
		else
		if (loc.first_line != 1)
			s << loc.first_line << ":";

		s << loc.first_column;
		if (loc.first_column != loc.last_column)
			s << "-" << loc.last_column;

		return s.str();
	}

	static inline int num_digits(long int number)
	{
		int r = 1;
		while (number >= 10) { ++r; number /= 10; };
		return r;
	}

	struct Expression
	{
		/** type of the parameters */
		enum ParamType
		{
			P_EXPRESSION,
			P_VARIABLE,
			P_CONSTANT
		};

		Expression(const std::string func_name, int num_params, YYLTYPE loc)
			:	func_name(func_name), num_params(num_params),
				location(loc),
				type	(num_params+1),
				expr	(num_params+1),
				var		(num_params+1),
				value	(num_params+1),
				loca 	(num_params+1)
		{ }

		// -------- interface ---------------------

		/** set parameter 'which' to Expression result */
		void setParam(int which, Expression * pexpr, YYLTYPE loc)
		{
			type[which] = P_EXPRESSION;
			expr[which] = pexpr;
			loca[which] = loc;
		}

		/** set parameter 'which' to Variable */
		void setParam(int which, Variable * v, YYLTYPE loc)
		{
			type[which] = P_VARIABLE;
			var[which] = v;
			loca[which] = loc;
		}

		/** set parameter 'which' to Float constant */
		void setParam(int which, Float constant, YYLTYPE loc)
		{
			type[which] = P_CONSTANT;
			value[which] = constant;
			loca[which] = loc;
		}

		void exec()
		{
			assert(func && "call of Expression::exec() without function pointer");
			func(&params[0]);
		}

		// ------------ info -------------

		/** return if whole expression is const */
		bool is_const() const
		{
			// expression without parameters can't be const
			if (num_params<1) return false;

			for (int i=0; i<num_params; ++i)
			if (type[i+1] != P_CONSTANT) return false;
			return true;
		}

		void print(std::ostream& out = std::cout) const
		{
			#define PPP_PRINT_PAR(which_) \
			{	if (type[which_] == P_EXPRESSION) \
				{ \
					out << "E" << expr[which_]; \
					if (expr[which_]->params.size()) out << "(" << *expr[which_]->params[0] << ")"; \
				} else \
				if (type[which_] == P_VARIABLE) out << "'" << var[which_]->name() << "'(" << var[which_]->value() << ")"; else \
				if (type[which_] == P_CONSTANT) out << value[which_]; else \
				out << "?"; }

			out << "[" << func_name;
			for (int i=1; i<=num_params; ++i)
				{ out << ", "; PPP_PRINT_PAR(i); }
			out << "]";

			#undef PPP_PRINT_PAR
		}

		std::string string() const { std::stringstream s; print(s); return s.str(); }

		/** return pointer to Float of each input parameter (which>0) */
		Float * get_pfloat(int which)
		{
			assert(which <= num_params && "Expression::get_pfloat() which out of range");

			if (type[which] == P_VARIABLE)
				return var[which]->value_ptr();
			else
			if (type[which] == P_CONSTANT)
				return &value[which];
			else
			if (type[which] == P_EXPRESSION)
				return expr[which]->params[0];

			assert(0 && "Expression::get_pfloat() type[which] unknown");
            return 0;
		}

		// ----------- member ------------

		/** name of the function */
		std::string	func_name;
		int num_params;
		/** location of this expression in string */
		YYLTYPE location;

		/** type of each parameter */
		std::vector<ParamType>		type;
		/** link to each expression */
		std::vector<Expression*>	expr;
		/** link to each variable */
		std::vector<Variable*>		var;
		/** each constant */
		std::vector<Float>	 		value;
		/** location in string of each parameter */
		std::vector<YYLTYPE>		loca;

		/** function pointer */
		FuncPtr 					func;
		/** place for result of the computation */
		Float 						result;
		/** pointer to Float parameters used for function call,
			first is result. */
		std::vector<Float*>			params;
	};




	class Program
	{
		public:

		typedef std::vector<Expression*> PExpressions;

		// --------- member ----------

		Functions * functions;
		PExpressions expr;

		// ---------- ctor -----------

		Program(Functions * functions)
			:	functions(functions)
		{ }

		~Program() { clear(); }

		void clear()
		{
			for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
				delete *i;
			expr.clear();
		}

		bool compile();

		Float eval()
		{
			if (expr.empty()) { std::cerr << "empty "; return 0.0; }
			PExpressions::const_iterator i;
			for (i=expr.begin(); i!=expr.end(); ++i)
			{
				// call if not NOP
				if ((*i)->func)
				{
					PPP_PROG_DEBUG("executing " << (*i)->string())
					(*i)->exec();
				}
			}
			return *(*(--i))->params[0];
		}

		void print(std::ostream& out = std::cout) const
		{
			for (PExpressions::const_iterator i=expr.begin(); i!=expr.end(); ++i)
			{
				(*i)->print(out);
				out << "\n";
			}
		}

		std::string string() const { std::stringstream s; print(s); return s.str(); }

		/** execute const expression and remove */
		void reduce_expression(Expression * e);

		// ------------------------ add expressions ------------------------

		Expression * add(YYLTYPE l0, const std::string& func_name)
		{
			Expression * e = new Expression(func_name, 0, l0);
			expr.push_back(e);
			return e;
		}

		template <class Float_pVariable_or_pExpression>
		Expression * add(YYLTYPE l0, const std::string& func_name, Float_pVariable_or_pExpression p1, YYLTYPE l1)
		{
			Expression * e = new Expression(func_name, 1, l0);
			e->setParam(1, p1, l1);
			expr.push_back(e);
			return e;
		}

		template <class Float_pVariable_or_pExpression1, class Float_pVariable_or_pExpression2>
		Expression * add(YYLTYPE l0, const std::string& func_name,
						Float_pVariable_or_pExpression1 p1, YYLTYPE l1,
						Float_pVariable_or_pExpression2 p2, YYLTYPE l2)
		{
			Expression * e = new Expression(func_name, 2, l0);
			e->setParam(1, p1, l1);
			e->setParam(2, p2, l2);
			expr.push_back(e);
			return e;
		}


		template <class Float_pVariable_or_pExpression1, class Float_pVariable_or_pExpression2,
				class Float_pVariable_or_pExpression3>
		Expression * add(YYLTYPE l0, const std::string& func_name,
						Float_pVariable_or_pExpression1 p1, YYLTYPE l1,
						Float_pVariable_or_pExpression2 p2, YYLTYPE l2,
						Float_pVariable_or_pExpression3 p3, YYLTYPE l3)
		{
			Expression * e = new Expression(func_name, 3, l0);
			e->setParam(1, p1, l1);
			e->setParam(2, p2, l2);
			e->setParam(3, p3, l3);
			expr.push_back(e);
			return e;
		}

		template <class Float_pVariable_or_pExpression1, class Float_pVariable_or_pExpression2,
				class Float_pVariable_or_pExpression3, class Float_pVariable_or_pExpression4>
		Expression * add(YYLTYPE l0, const std::string& func_name,
						Float_pVariable_or_pExpression1 p1, YYLTYPE l1,
						Float_pVariable_or_pExpression2 p2, YYLTYPE l2,
						Float_pVariable_or_pExpression3 p3, YYLTYPE l3,
						Float_pVariable_or_pExpression4 p4, YYLTYPE l4)
		{
			Expression * e = new Expression(func_name, 4, l0);
			e->setParam(1, p1, l1);
			e->setParam(2, p2, l2);
			e->setParam(3, p3, l3);
			e->setParam(4, p4, l4);
			expr.push_back(e);
			return e;
		}

	};


	bool Program::compile()
	{
		PPP_PROG_DEBUG( "before compilation:\n" << string() )

		#define PPP_QUIT clear(); return false;
		for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
		{
			Expression * e = *ei;

			e->params.clear();

			// special case
			if (e->func_name == "NOP")
			{
				// simply pass through the source
				e->params.push_back( e->get_pfloat(1) );
				e->func = 0;
			}
			else
			{
				// get function call
				Function * fu = functions->function(e->func_name, e->num_params);
				if (!fu)
				{
					PPP_PROG_ERROR(location_string(e->location) << ": unknown function '" << e->func_name << "'")
					PPP_QUIT
				}
				e->func = fu->func();

				// result of functions is always this
				e->params.push_back( &e->result );

				// collect sources
				for (int i=0; i<e->num_params; ++i)
					e->params.push_back( e->get_pfloat(i+1) );
			}

		}

		// remove const expressions
		int k=expr.size(), k0 = 0;
	check_again:
		for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
		{
			Expression * e = *ei;

			if (e->is_const())
			{
				reduce_expression(e);
				if (++k0 < k) goto check_again;
			}
		}

		PPP_PROG_DEBUG( "after reduction:\n" << string() )

		#undef PPP_QUIT
		return true;
	}


	void Program::reduce_expression(Expression * ex)
	{
		// run and get result
		if (ex->func) ex->exec();
		Float result = *ex->params[0];

		// don't remove the last expression
		if (expr.size()<2)
		{
			PPP_PROG_DEBUG("NOPing remaining expression " << ex->string());
			// simply NOP it (result is already calculated)
			ex->func = 0;
			ex->func_name = "NOP";
			// for correct debug output:
			ex->num_params = 1;
			ex->type[1] = Expression::P_CONSTANT;
			ex->value[1] = result;
			return;
		}

		PPP_PROG_DEBUG("reducing " << ex->string() << " to " << result);

		// find references
		for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
		{
			Expression * e = *ei;
			for (int i=1; i<=e->num_params; ++i)
			if (e->type[i] == Expression::P_EXPRESSION && e->expr[i] == ex)
			{
				e->type[i] = Expression::P_CONSTANT;
				e->value[i] = result;
			}
		}

		// delete from list
		for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
			if (*i == ex) { expr.erase(i); break; }
	}

} // namespace PPP_NAMESPACE


#endif // PARSER_PROGRAM_H_INCLUDED
