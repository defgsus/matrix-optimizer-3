/**	@file

	@brief Math Function Parser - private functions and classes

	@author def.gsus-
	@version 2013/09/30 started
*/
#ifndef PARSER_PROGRAM_H_INCLUDED
#define PARSER_PROGRAM_H_INCLUDED

#include <cassert>
#include <functional>
#include <set>
#include <iomanip>

#include "functions.h"
#include "grammar.parser.h"
#include "parser.h"

// qt specific debugging
#if 1
#include <QDebug>
#   define PPP_PRINT(stream_arg__) \
      { std::stringstream s__; \
        s__ << stream_arg__; \
        qDebug() << s__.str().c_str(); \
        }
#else
#   define PPP_PRINT(stream_arg__) { std::cerr << stream_arg__; }
#endif

#if !defined(NDEBUG) & (0)
#	define PPP_PROG_DEBUG(stream_arg__) { PPP_PRINT( stream_arg__ << "\n" ); }
#	define PPP_WHERE "\n at: " << __LINE__ << ":" << __FILE__
#else
#	define PPP_PROG_DEBUG(unused__) { }
#	define PPP_WHERE ""
#endif


#define PPP_PROG_WARN(loc__, stream_arg__) \
    PPP_PRINT( "parser: warning: " << PPP_NAMESPACE::location_string(loc__) << ": " << stream_arg__ << PPP_WHERE );
#define PPP_PROG_ERROR(loc__, stream_arg__) \
    PPP_PRINT( "parser: error: " << PPP_NAMESPACE::location_string(loc__) << ": " << stream_arg__ << PPP_WHERE );

/** used to signal erros from inside yyparse(). only compiles in bison! */
#define PPP_ERR(loc__, stream_arg__) \
{	PPP_PRINT( "ERROR: " << PPP_NAMESPACE::location_string(loc__) << ": " << stream_arg__ << PPP_WHERE << "\n" ); \
    YYERROR; }

/** prints a nicer pointer address */
#define PPPS_EXPR(expr__) "e" << std::setfill('0') << std::setw(3) << (((size_t)(expr__))%521+1)
#define PPPS_PROG(expr__) "p" << std::setfill('0') << std::setw(3) << (((size_t)(expr__))%907+1)

namespace PPP_NAMESPACE
{

    static inline void create_functions(Functions & f)
    {
        f.add(Function::BINARY_OP,		2, "=",			math_func<Float>::assign_2		);

        f.add(Function::FUNCTION,		3, "?",			math_func<Float>::assign_if_3	);

        f.add(Function::BINARY_OP,		2, "+",			math_func<Float>::add_2			);
        f.add(Function::BINARY_OP,		2, "-",			math_func<Float>::sub_2			);
        f.add(Function::BINARY_OP,		2, "*",			math_func<Float>::mul_2			);
        f.add(Function::BINARY_OP,		2, "/",			math_func<Float>::div_2			);
        f.add(Function::BINARY_OP,		2, "^",			math_func<Float>::pow_2			);
        f.add(Function::BINARY_OP,		2, "%",			math_func<Float>::mod_2			);

        f.add(Function::BINARY_OP,		2, "+=",		math_func<Float>::add_1			);
        f.add(Function::BINARY_OP,		2, "-=",		math_func<Float>::sub_1			);
        f.add(Function::BINARY_OP,		2, "*=",		math_func<Float>::mul_1			);
        f.add(Function::BINARY_OP,		2, "/=",		math_func<Float>::div_1			);


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
        f.add(Function::FUNCTION,		1, "frac",		math_func<Float>::frac_1		);

        f.add(Function::FUNCTION,		2, "min",		math_func<Float>::min_2			);
        f.add(Function::FUNCTION,		2, "max",		math_func<Float>::max_2			);
        f.add(Function::FUNCTION,		3, "clamp",		math_func<Float>::clamp_3		);

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

        f.add(Function::FUNCTION,		3, "rotate",	math_func<Float>::rotate_3		);
        f.add(Function::FUNCTION,		3, "rotater",	math_func<Float>::rotater_3		);

        f.add(Function::FUNCTION,		1, "exp",		math_func<Float>::exp_1			);
        f.add(Function::FUNCTION,		1, "ln",		math_func<Float>::ln_1			);
        f.add(Function::FUNCTION,		2, "logistic",	math_func<Float>::logistic_1	);


        f.add(Function::FUNCTION,		2, "mod",		math_func<Float>::mod_2			);
        f.add(Function::FUNCTION,		2, "smod",		math_func<Float>::smod_2		);

        f.add(Function::FUNCTION,		2, "pow",		math_func<Float>::pow_2			);
        f.add(Function::FUNCTION,		1, "sqrt",		math_func<Float>::sqrt_1		);
        f.add(Function::FUNCTION,		2, "root",		math_func<Float>::root_2		);

        f.add(Function::FUNCTION,		1, "ramp",		math_func<Float>::ramp_1		);
        f.add(Function::FUNCTION,		1, "saw",		math_func<Float>::saw_1         );
        f.add(Function::FUNCTION,		1, "square",	math_func<Float>::square_1		);
        f.add(Function::FUNCTION,		2, "square",	math_func<Float>::square_2		);
        f.add(Function::FUNCTION,		1, "tri",		math_func<Float>::tri_1         );
        f.add(Function::FUNCTION,		2, "tri",		math_func<Float>::tri_2         );

        f.add(Function::FUNCTION,		0, "rnd",		math_func<Float>::rnd_0			);
        f.add(Function::FUNCTION,		1, "noise",		math_func<Float>::noise_1		);
        f.add(Function::FUNCTION,		2, "noise",		math_func<Float>::noise_2		);
        f.add(Function::FUNCTION,		3, "noise",		math_func<Float>::noise_3		);

        f.add(Function::FUNCTION,		1, "fac",		math_func<Float>::factorial_1	);
        f.add(Function::FUNCTION,		1, "fib",		math_func<Float>::fibonacci_1	);

        f.add(Function::FUNCTION,		1, "zeta",		math_func<Float>::zeta_1		);
        f.add(Function::FUNCTION,		2, "zeta",		math_func<Float>::zetap_2		);

        f.add(Function::FUNCTION,		2, "harmo",		math_func<Float>::harmo_2		);
        f.add(Function::FUNCTION,		3, "harmo",		math_func<Float>::harmo_3		);

        f.add(Function::FUNCTION,		1, "prime",		math_func<Float>::prime_1		);
        f.add(Function::FUNCTION,		1, "sprime",	math_func<Float>::sprime_1		);

//        f.add(Function::FUNCTION,		1, "nextprime",	math_func<Float>::nextprime_1	);
        f.add(Function::FUNCTION,		1, "quer",		math_func<Float>::quer_1		);

        f.add(Function::FUNCTION,		2, "uspiral",	math_func<Float>::ulam_spiral_2	);
        f.add(Function::FUNCTION,		3, "uspiral",	math_func<Float>::ulam_spiral_3	);
        f.add(Function::FUNCTION,		2, "tspiral",	math_func<Float>::tri_spiral_2	);

        f.add(Function::FUNCTION,		1, "ndiv",		math_func<Float>::numdiv_1		);
        f.add(Function::FUNCTION,		2, "divisor",   math_func<Float>::divisor_2		);
        f.add(Function::FUNCTION,		1, "sumdiv",	math_func<Float>::sumdiv_1		);
        f.add(Function::FUNCTION,		1, "proddiv",	math_func<Float>::proddiv_1		);
        f.add(Function::FUNCTION,		2, "nextdiv",	math_func<Float>::nextdiv_2		);
        f.add(Function::FUNCTION,		2, "gcd",		math_func<Float>::gcd_2			);
        f.add(Function::FUNCTION,		3, "cong",		math_func<Float>::congruent_3	);

        f.add(Function::FUNCTION,		1, "digits",	math_func<Float>::digits_1		);


//        f.add(Function::FUNCTION,		2, "digcount",	math_func<Float>::digit_count_2	);
//        f.add(Function::FUNCTION,		2, "digfreq",	math_func<Float>::digit_freq_2	);

        f.add(Function::FUNCTION,       2, "mandel",    math_func<Float>::mandel_2     );
        f.add(Function::FUNCTION,       2, "mandeli",   math_func<Float>::mandeli_2     );
        f.add(Function::FUNCTION,       3, "mandel",    math_func<Float>::mandel_3     );
        f.add(Function::FUNCTION,       3, "mandeli",   math_func<Float>::mandeli_3     );

        f.add(							4, "series",	lambda_func<Float>::series_4	);
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

    class Program;

    class Expression
    {
        public:

        /** type of the parameters */
        enum ParamType
        {
            /** normally a function call */
            P_EXPRESSION,
            P_VARIABLE,
            P_CONSTANT,
            /** link to a subprogram */
            P_PROGRAM,
            /** function argument */
            P_LAMBDA
        };

        // ----------- member ------------

        /** name of the function */
        std::string	func_name;
        /** number of it's parameters */
        int num_params;
        /** location of this expression in string */
        YYLTYPE location;

        /** type of each parameter */
        std::vector<ParamType>		type;
        /** link to each expression */
        std::vector<Expression*>	expr;
        /** link to each function parameter */
        std::vector<Function*>		lambda;
        /** name of each function parameter */
        std::vector<std::string>	lambda_name;
        /** link to each program */
        std::vector<Program*>		prog;
        /** link to each variable */
        std::vector<Variable*>		var;
        /** each constant */
        std::vector<Float>	 		value;
        /** location in string of each parameter */
        std::vector<YYLTYPE>		loca;
        /** pointer to Float parameters used for function call,
            first is result. */
        std::vector<Float*>			params;

        /** function pointer class */
        Function 					*func;
        /** place for result of the computation */
        Float 						result;

        /** number of times this expression is used (set by compiler) */
        int num_needed;

        /** true if this expression is a condition in a ? statement */
        bool is_condition;
        /** normally true, can be switched of for conditioned expressions, e.g ? x : y */
        bool do_execute;
        /** used by Reduction. final is a NOP that is not needed. */
        bool final;

        /** expression that is conditionally controlling the
            execution of this. */
        Expression * cond_expr_left, * cond_expr_right;

        typedef std::vector<Expression*> PExpressions;
        /** list of expression that need to be turned on/off according
            to condition. only defined for Expression that have is_condition set. */
        PExpressions left_args, right_args;

        // ------------------- ctor -------------------------

        Expression(const std::string func_name, int num_params, YYLTYPE loc)
            :	func_name		(func_name),
                num_params		(num_params),
                location		(loc),
                type			(num_params+1),
                expr			(num_params+1),
                lambda			(num_params+1),
                lambda_name		(num_params+1),
                prog			(num_params+1),
                var				(num_params+1),
                value			(num_params+1),
                loca 			(num_params+1),
                func 			(0),
                result			((Float)0),
                num_needed		(0),
                is_condition	(false),
                do_execute		(true),
                final			(false),
                cond_expr_left	(0),
                cond_expr_right	(0),
                left_args		(0),
                right_args		(0)
        { }

        // -------- interface ---------------------

        /** set parameter 'which' to Expression result */
        void setParam(int which, Expression * pexpr, YYLTYPE loc)
        {
            type[which] = P_EXPRESSION;
            expr[which] = pexpr;
            loca[which] = loc;
        }

        /** set parameter 'which' to Function call */
        void setParam(int which, const std::string& lambda_name, YYLTYPE loc)
        {
            type[which] = P_LAMBDA;
            this->lambda_name[which] = lambda_name;
            lambda[which] = 0;
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

        /** set parameter 'which' to program constant */
        void setParam(int which, Program * prg, YYLTYPE loc)
        {
            type[which] = P_PROGRAM;
            prog[which] = prg;
            loca[which] = loc;
        }

        /** make this expression a condition and make 'e1' and 'e2' conditionally executed. */
        void setConditionArgs(Expression * , Expression * )
        {
            is_condition = true;
            //cond_expr1 = e1; e1->do_execute = false;
            //cond_expr2 = e2; e2->do_execute = false;
        }

        /** execute the assigned function */
        void exec()
        {
            assert(do_execute && "call of Expression::exec() while !do_execute");
            assert(func && "call of Expression::exec() without function pointer");

            if (func->is_lambda())
            {
                assert(func->lambda_func_ && "call of Expression::exec() with P_LAMBDA without function ptr");
                assert(lambda[1] && lambda[1]->func_ && "call of Expression::exec() with P_LAMBDA without function ptr");
                assert(lambda[2] && lambda[2]->func_ && "call of Expression::exec() with P_LAMBDA without function ptr");
                func->lambda_func_( lambda[1]->func_, lambda[2]->func_, &params[0] );
            }
            else
                func->func()(&params[0]);

            if (is_condition)
            {
                bool yes = *params[0] != 0;
                for (PExpressions::iterator i=left_args.begin(); i!=left_args.end(); ++i)
                    (*i)->do_execute = yes;
                yes = !yes;
                for (PExpressions::iterator i=right_args.begin(); i!=right_args.end(); ++i)
                    (*i)->do_execute = yes;
            }
        }

        // ------------ info -------------

        /** return if whole expression is const */
        bool is_const() const
        {
            // expression without parameters can't be const
            if (num_params<1) return false;

            for (int i=1; i<=num_params; ++i)
                if (type[i] != P_CONSTANT) return false;
            return true;
        }

        bool is_assignment() const { return func_name == "="; }

        bool is_funcdef() const { return func_name.empty() || func_name[0] == '@'; }

        bool is_top() const { return num_needed == 0 && !is_funcdef(); }

        bool is_conditional() const { return cond_expr_left != 0 || cond_expr_right != 0; }

        bool should_have_function() const
            { return !(is_funcdef() || func_name == "NOP"); }

        void print(std::ostream& out = std::cout) const
        {
            #define PPP_PRINT_PAR(which_) \
            {	if (type[which_] == P_EXPRESSION) \
                { \
                    out << PPPS_EXPR(expr[which_]); \
                    if (!expr[which_]) out << "*empty*"; else \
                    if (expr[which_]->params.size()) out << "(" << *expr[which_]->params[0] << ")"; \
                } else \
                if (type[which_] == P_VARIABLE) out << "'" << var[which_]->name() << "'(" << var[which_]->value() << ")"; else \
                if (type[which_] == P_CONSTANT) out << value[which_]; else \
                if (type[which_] == P_LAMBDA) out << "@" << lambda_name[which_]; else \
                if (type[which_] == P_PROGRAM) \
                { \
                    out << PPPS_PROG(prog[which_]); \
                } else \
                out << "?"; }

            out << PPPS_EXPR(this) << " = [" << func_name;
            if (func) out << "_" << func->num_param();
            for (int i=1; i<=num_params; ++i)
                { out << ", "; PPP_PRINT_PAR(i); }
            out << "]";

            if (is_condition)
            {
                out << "?";
                if (left_args.empty()) out << " none";
                for (PExpressions::const_iterator i=left_args.begin(); i!=left_args.end(); ++i)
                    out << " " << PPPS_EXPR(*i);
                out << " :";
                if (right_args.empty()) out << " none";
                for (PExpressions::const_iterator i=right_args.begin(); i!=right_args.end(); ++i)
                    out << " " << PPPS_EXPR(*i);
            }

            if (cond_expr_left)
                out << " " << PPPS_EXPR(cond_expr_left) << "? 1:0";
            if (cond_expr_right)
                out << " " << PPPS_EXPR(cond_expr_right) << "? 0:1";
            #undef PPP_PRINT_PAR
        }

        std::string string() const { std::stringstream s; print(s); return s.str(); }

        /** return pointer to Float of each input parameter (which>0) */
        Float * get_pfloat(int which)
        {
            assert(which <= num_params && "Expression::get_pfloat() which out of range");

            if (type[which] == P_VARIABLE)
                return var[which]? var[which]->value_ptr() : 0;
            else
            if (type[which] == P_CONSTANT)
                return &value[which];
            else
            if (type[which] == P_EXPRESSION)
                return expr[which]? expr[which]->params[0] : 0;
            else
            if (type[which] == P_LAMBDA)
            {
                assert(0 && "Expression::get_pfloat() called for lambda type");
                return 0;
            }

            assert(0 && "Expression::get_pfloat() type[which] unknown");
            return 000;
        }

        Float * get_rec_result()
        {
            if (func_name=="NOP") return get_pfloat(1);
            return params[0];
        }

    };



    class FuncDef
    {
        public:

        /** location of definition */
        YYLTYPE l0;

        /** name of function */
        std::string name;
        /** location of function name */
        YYLTYPE lname;

        /** name, var1, var2, ... */
        std::vector<std::string> var;
        /** location of name, var1, var2, ... */
        std::vector<YYLTYPE> lvar;

        /** this type must be copy constructable and assignable */
        FuncDef() { }

        FuncDef(YYLTYPE l0, const std::string& name, YYLTYPE lname,
                    const std::vector<std::string>& vars,
                    const std::vector<YYLTYPE>& loc_vars)
            :	l0(l0), name(name), lname(lname), var(vars), lvar(loc_vars)
        { }
    };



    class Program
    {
        public:

        typedef std::vector<Expression*> PExpressions;
        typedef std::vector<FuncDef*> PFuncDefs;
        typedef std::vector<Program*> PPrograms;
        typedef std::set<std::string> StringSet;

        // --------- member ----------

        Functions * functions;
        Variables * variables;
        StringSet potential_functions;
        bool empty;
        volatile size_t called;
        PExpressions expr;
        PFuncDefs funcdef;
        /** sub-programs */
        PPrograms sub;
        Program * parent;
        Program * root() { return parent==0 ? this : parent->root(); }

        std::vector<bool> cond_stack;
        std::vector<Float> param_stack;

        /** current condition expression */
        Expression * cond_expr_left, * cond_expr_right;

    #ifndef NDEBUG
        std::string debug_prefix;
    #endif

        // ---------- ctor -----------

        Program(Functions * functions, Variables * variables)
            :	functions(functions),
                variables(variables),
                empty	(true),
                called	(0),
                parent	(0),
                cond_expr_left	(0),
                cond_expr_right	(0)
        { }

        ~Program() { clear(); }

        void clear()
        {
            clearCallCount();

            for (PPrograms::iterator i=sub.begin(); i!=sub.end(); ++i)
                delete *i;
            sub.clear();

            for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
                delete *i;
            expr.clear();

            for (PFuncDefs::iterator i=funcdef.begin(); i!=funcdef.end(); ++i)
                delete *i;
            funcdef.clear();

            variables->clear_temps_();
            functions->clear_temps_();
            potential_functions.clear();
            empty = true;
            parent = 0;
            cond_stack.clear();
            param_stack.clear();
            cond_expr_left = cond_expr_right = 0;
        }

        Program * getSubProgram()
        {
            Program * p = new Program(functions, variables);
            p->parent = this;
        #ifndef NDEBUG
            p->debug_prefix = debug_prefix + " ";
        #endif
            sub.push_back( p );
            return p;
        }

        void pushExpressions();
        void popExpressions();


        // -------- compiler --------

        // compile all subprogs and this program
        bool compile();

        bool rec_compile();
        bool rec_reduce();
        bool assignprog();
        bool link();
        bool reduce();

        /** call this in parent! */
        void clearCallCount()
        {
            called = 0;
            for (PPrograms::iterator i=sub.begin(); i!=sub.end(); ++i)
                (*i)->clearCallCount();
        }

        Float eval()
        {
            if (empty) { std::cerr << "empty "; return 0.0; }

            for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
            {
                if ((*i)->is_conditional())
                    (*i)->do_execute = false;
            }

            PExpressions::const_iterator i;
            for (i=expr.begin(); i!=expr.end(); ++i)
            {
                // call if not NOP
                if ((*i)->func)
                {
                    if ((*i)->do_execute)
                    {
                        PPP_PROG_DEBUG(debug_prefix << "executing " << (*i)->string())
                        (*i)->exec();
                        PPP_PROG_DEBUG(debug_prefix << PPPS_EXPR(*i) << " := " << *(*i)->params[0]);
                    }
                    else
                        PPP_PROG_DEBUG(debug_prefix << "skipping  " << (*i)->string() << " on condition");
                }
            }
            return *(*(--i))->params[0];
        }

        /** execute const expression and remove */
        void reduce_expression(Expression * e);

        // ------------ info ---------------------------

        std::string id() const { std::stringstream s; s << std::hex << this; return s.str(); }

        /** looks for locals through whole parent tree */
        Variable * has_variable(const std::string name)
        {
            if (Variable * v = variables->variable_(name, this)) return v;
                else if (parent) return parent->has_variable(name);
                    else return variables->variable(name);
        }

        bool has_potential_function(const std::string name)
        {
            if (potential_functions.find(name) != potential_functions.end()) return true;
            return (parent && parent->potential_functions.find(name) != parent->potential_functions.end());
        }

        /** is this expression referenced somewhere */
        bool is_needed(Expression *ex);

        /** print as human-readable. if e == 0, print all TOPs */
        void printSyntax(std::ostream& out, Expression * e = 0) const;

        void print(std::ostream& out = std::cout) const
        {
            for (PPrograms::const_iterator i=sub.begin(); i!=sub.end(); ++i)
            {
                PPP_PROG_DEBUG(debug_prefix << "sub " << PPPS_PROG(*i) << ":");
                (*i)->print(out);
            }

            for (PExpressions::const_iterator i=expr.begin(); i!=expr.end(); ++i)
            {
            #ifndef NDEBUG
                out << debug_prefix;
            #endif
                (*i)->print(out);
                if ( (*i)->is_top() ) out << " TOP";
                out << "\n";
            }
        }

        std::string string() const { std::stringstream s; print(s); return s.str(); }
        std::string string(Expression * e) const { std::stringstream s; printSyntax(s,e); return s.str(); }

        void printDot(std::ostream& out = std::cout);
        void printDotExpressions(std::ostream& out = std::cout, int y = 0);

        void printVariables(std::ostream& out = std::cout) const
        {
            for (Variables::Map::const_iterator i=variables->map_.begin();
                        i!=variables->map_.end(); ++i)
            {
                out << i->second->name() << " = " << i->second->value() << "\n";
            }
        }

        void printExpressionContent(std::ostream& out = std::cout) const;

        int num_visible_expr() const
        {
            int k=0;
            for (PExpressions::const_iterator i=expr.begin(); i!=expr.end(); ++i)
                if (!(*i)->is_funcdef())
                    ++k;
            return k;
        }

        void find_connected_expressions(PExpressions& list);



        // ------------------------ add expressions ------------------------

        void startConditionLeft(Expression * cond_expr)
        {
            cond_expr_left = cond_expr;
            cond_expr_right = 0;
        }
        void startConditionRight(Expression * cond_expr)
        {
            cond_expr_left = 0;
            cond_expr_right = cond_expr;
        }
        void stopCondition()
        {
            cond_expr_left = cond_expr_right = 0;
        }

        void addExpr_(Expression * e)
        {
            //std::cout << cond_expr_right << "\n";
            e->cond_expr_left = cond_expr_left;
            e->cond_expr_right = cond_expr_right;
            expr.push_back( e );
        }

        Expression * add(YYLTYPE l0, const std::string& func_name)
        {
            Expression * e = new Expression(func_name, 0, l0);
            addExpr_(e);
            return e;
        }

        template <class Float_pVariable_or_pExpression>
        Expression * add(YYLTYPE l0, const std::string& func_name, Float_pVariable_or_pExpression p1, YYLTYPE l1)
        {
            Expression * e = new Expression(func_name, 1, l0);
            e->setParam(1, p1, l1);
            addExpr_(e);
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
            addExpr_(e);
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
            addExpr_(e);
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
            addExpr_(e);
            return e;
        }

        template <class Float_pVariable_or_pExpression1, class Float_pVariable_or_pExpression2>
        Expression * addLambda(YYLTYPE l0,
                                const std::string& func_name,
                                const std::string& exec_name, YYLTYPE l1,
                                const std::string& op_name, YYLTYPE l2,
                        Float_pVariable_or_pExpression1 p3, YYLTYPE l3,
                        Float_pVariable_or_pExpression2 p4, YYLTYPE l4)
        {
            PPP_PROG_DEBUG(debug_prefix << "lambda function " << func_name << "(" << exec_name << ", " << op_name << ", ...)" );
            Expression * e = new Expression(func_name, 4, l0);

            e->setParam(1, exec_name, l1);
            e->setParam(2, op_name, l2);
            e->setParam(3, p3, l3);
            e->setParam(4, p4, l4);
            addExpr_(e);
            return e;
        }

        /** create and attach an assignment expression */
        Expression * assignment(YYLTYPE l0,
                            const std::string& var_name, YYLTYPE l1,
                            Expression * expr, YYLTYPE l2);

        /** check definition and add variables and return true, or false if failure */
        bool addFuncDef(const FuncDef & f);

        /** create a function according to the definition with 'p' as it's function. */
        Expression * createFunction(const FuncDef & f, Program * p, YYLTYPE lp);
    };

    // ------------------- END class PROGRAM -------------------------




    Expression * Program::assignment(YYLTYPE l0,
                                    const std::string& var_name, YYLTYPE l1,
                                    Expression * rex, YYLTYPE l2)
    {
        // find or create variable
        Variable * v = has_variable(var_name);
        if (!v) v = variables->add_temp_(var_name, 0.0, this);

        // construct an assignment expression
        Expression * e = new Expression("=", 2, l0);
        e->setParam(1, v, l1);
        e->setParam(2, rex, l2);
        addExpr_(e);
        return e;
    }


    bool Program::addFuncDef(const FuncDef & f)
    {
        /*PPP_PROG_DEBUG("defining function '" << f.name << "(n="<<f.var.size()<<")'");

        if (functions->function(f.name, f.var.size()))
        {
            PPP_PROG_ERROR(f.l0, "redefinition of function '" << f.name << "'");
            return false;
        }
        */
        // create temporary variables
        for (size_t i=0; i<f.var.size(); ++i)
        {
            if (has_variable(f.var[i]))
            {
                PPP_PROG_WARN(f.lvar[i], "function parameter '" << f.var[i] << "' shadows variable");
                //PPP_PROG_ERROR(f.lvar[i], "function parameter '" << f.var[i] << "' must be unknown variable");
                //return false;
            }
            PPP_PROG_DEBUG(debug_prefix << "creating local variable '" << f.var[i] << "' for " << PPPS_PROG(this) );
            variables->add_temp_(f.var[i], 0, this);
        }

        return true;
    }


    Expression * Program::createFunction(const FuncDef & f, Program * p, YYLTYPE lp)
    {
        PPP_PROG_DEBUG(debug_prefix << "create function " << f.name << "(n="<<f.var.size()<<") " << PPPS_PROG(p) );

        // --- create the function ---

        if (functions->function(f.name, f.var.size()))
        {
            PPP_PROG_ERROR(f.l0, "redeclaration of function '" << f.name << "'");
            return 0;
        }

        // temporary to construct a function call that is a subprogram
        struct temp_
        {
            static void func(Float ** v, Program * p, Expression * e)
            {
                PPP_PROG_DEBUG(p->debug_prefix << "{} subprogram call " << PPPS_EXPR(e) << " "<< PPPS_PROG(p) );

                //std::cout << p->called << "\n";
                /*
                if (++p->called > 1000)
                {
                    PPP_PROG_ERROR(e->location, "recursive call > 1000");
                    *v[0] = 0;
                    return;
                }*/

                //p->printVariables();

                Float stack[e->num_params];

                p->pushExpressions();

                // copy input params to local vars (and save previous on stack)
                for (int i=1; i<=e->num_params; ++i)
                {
                    stack[i-1] = *e->var[i]->value_;
                    *e->var[i]->value_ = *v[i];
                }

                // execute program
                Float tmp = p->eval();

                p->popExpressions();

                *v[0] = tmp;

                PPP_PROG_DEBUG(p->debug_prefix << "{} result = " << *v[0])

                // restore local variables
                for (int i=1; i<=e->num_params; ++i)
                    *e->var[i]->value_ = stack[i-1];

            }
        };

        // --- create the expression ---

        Expression * e = new Expression("@"+f.name, f.var.size(), f.l0);

        // insert variables as parameters
        for (size_t i=0; i<f.var.size(); ++i)
            if (Variable * v = has_variable(f.var[i]))

                e->setParam(i+1, v, lp);

            else
            {
                PPP_PROG_ERROR(f.lvar[i], "unknown variable '" << f.var[i] << "' in function declaration");
                delete e;
                return 0;
            }

        /* XXX little hacky. prog parameters are not used otherwise right now */
        e->prog[0] = p;
        addExpr_(e);
        // add the function and link with program and expression */
        Function * fu = functions->add(Function::FUNCTION, f.var.size(), f.name, std::bind(temp_::func, std::placeholders::_1, p, e) );
        fu->temp_ = true;

        return e;
    }









    // -------------------------------- compiler code ----------------------------------------

    bool Program::compile()
    {
        cond_expr_left = cond_expr_right = 0;

        PPP_PROG_DEBUG(debug_prefix << "getting function pointers " << PPPS_PROG(this));
        if (!assignprog()) return false;

        // first compile subs
        for (PPrograms::iterator i=sub.begin(); i!=sub.end(); ++i)
        {
            if (!(*i)->compile()) return false;
        }

        PPP_PROG_DEBUG(debug_prefix << "linking " << PPPS_PROG(this));
        if (!link()) return false;
        PPP_PROG_DEBUG(debug_prefix << "reducing " << PPPS_PROG(this));
        return  reduce();

    //	if (!rec_compile()) return false;
    //	if (!rec_reduce()) return false;
    //	return true;
    }

    bool Program::rec_compile()
    {
        PPP_PROG_DEBUG(debug_prefix << "getting function pointers " << PPPS_PROG(this));
        if (!assignprog()) return false;

        for (PPrograms::iterator i=sub.begin(); i!=sub.end(); ++i)
        {
            if (!(*i)->rec_compile()) return false;
        }

        PPP_PROG_DEBUG(debug_prefix << "linking " << PPPS_PROG(this));
        if (!link()) return false;

        return true;
    }

    bool Program::rec_reduce()
    {
        for (PPrograms::iterator i=sub.begin(); i!=sub.end(); ++i)
        {
            if (!(*i)->rec_reduce()) return false;
        }

        PPP_PROG_DEBUG(debug_prefix << "reducing " << PPPS_PROG(this));
        return  reduce();
    }

    bool Program::assignprog()
    {
        #define PPP_QUIT { clear(); return false; }

        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        {
            Expression * e = *ei;

            e->params.clear();

//			std::cout << e->func_name << "::\n";

            // special case
            if (e->func_name == "NOP")
            {
                // simply pass through the source
                e->params.push_back( e->get_pfloat(1) );
                e->func = 0;
            }
            else
            {
                std::string name = e->func_name;

                // is only function definition?
                bool funcdef = (!name.empty() && name[0] == '@');
                if (funcdef) name.erase(0,1);

                // get function call
                Function * fu = functions->function(name, e->num_params);
                if (!fu)
                {
                    //PPP_PROG_DEBUG(functions->string());
                    PPP_PROG_ERROR(e->location, "linker: unknown function '" << name << "' (" << e->num_params << ")")
                    PPP_QUIT
                }
                e->func = funcdef? 0 : fu;

                // get function parameter functions
                for (int i=0; i<e->num_params; ++i)
                if (e->type[i] == Expression::P_LAMBDA)
                {
                    int n_args = 1;
                    if (i==2) n_args = 2;
                    fu = functions->function(e->lambda_name[i], n_args);
                    if (!fu)
                    {
                        PPP_PROG_ERROR(e->loca[i], "lambda function parameter '" << e->lambda_name[i] <<
                                    "' with " << n_args << " argument" << (n_args>1?"s":"") << " unknown");
                        PPP_QUIT
                    }
                    e->lambda[i] = fu;
                }

                // result of functions is always this
                e->params.push_back( &e->result );
            }
        }
        return true;
    }

    bool Program::link()
    {
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        {
            Expression * e = *ei;

            // collect sources
            for (int i=1; i<=e->num_params; ++i)
            {
                Float * src = 0;
                if (e->type[i] != Expression::P_LAMBDA)
                {
                    src = e->get_pfloat(i);
                    if (!src)
                    {
                        PPP_PROG_ERROR(e->loca[i], "internal logic: "
                                    "uninitialized parameter " << i << " in expression '" << e->func_name << "'");
                        return false;
                    }
                }
                e->params.push_back( src );
            }
        }

        // check assignment
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        {
            // check function assignment
            Expression * e = *ei;

            if (e->should_have_function() && e->func == 0)
            {
                PPP_PROG_ERROR( e->location, "internal logic: expression '" << e->func_name
                                << "' has no function" );
                return false;
            }

            // check parameters
            for (int j=1; j<=e->num_params; ++j)
            {
                bool ok =
                        ((size_t)j < e->type.size())
                    && 	(e->type[j] != Expression::P_EXPRESSION ||	e->expr[j])
                    &&	(e->type[j] != Expression::P_VARIABLE || 	e->var[j])
                    ;

                if (!ok)
                {
                    PPP_PROG_ERROR( e->location, "internal logic: parameter "
                                << j << " not initialized" );
                    return false;
                }

            }

            // assign NOP's result
            if (e->func_name == "NOP")
            {
                PPP_PROG_DEBUG(debug_prefix << "assigning result " << *e->params[1] << " to NOP " << PPPS_EXPR(e));
                e->params[0] = e->params[1];
            }
        }

        return true;
    }

    bool Program::reduce()
    {
        for (PExpressions::reverse_iterator ei=expr.rbegin(); ei!=expr.rend(); ++ei)
            (*ei)->final = false;
        /*
        // remove const expressions
        int k=expr.size(), k0 = 0;
    check_again:
        for (PExpressions::reverse_iterator ei=expr.rbegin(); ei!=expr.rend(); ++ei)
        {
            Expression * e = *ei;

            if (e->is_const() && !e->final)
            {
                reduce_expression(e);
                if (k0++ < k) goto check_again;
            }

        }
        */

        // count usage
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
            (*ei)->num_needed = 0;
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
            for (int i=0; i<(*ei)->num_params; ++i)
                if ((*ei)->type[i+1] == Expression::P_EXPRESSION)
                    ++(*ei)->expr[i+1]->num_needed;

        // check unneeded expressions
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
            if ((*ei)->num_needed==0 && !(*ei)->is_assignment()
                && *ei != expr.back() && !(*ei)->is_funcdef())
                PPP_PROG_WARN( (*ei)->location, "unused expression " << string(*ei) );

        // assign conditional expressions to condition
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        {
            if ((*ei)->cond_expr_left)
            {
                if (!(*ei)->cond_expr_left->is_condition)
                    PPP_PROG_ERROR((*ei)->cond_expr_left->location, PPPS_EXPR((*ei)->cond_expr_left)
                        << " is no condition but left conditional expression " << PPPS_EXPR(*ei) << " referes to it.");

                (*ei)->cond_expr_left->left_args.push_back( *ei );
            }
            if ((*ei)->cond_expr_right)
            {
                if (!(*ei)->cond_expr_right->is_condition)
                    PPP_PROG_ERROR((*ei)->cond_expr_right->location, PPPS_EXPR((*ei)->cond_expr_right)
                        << " is no condition but right conditional expression " << PPPS_EXPR(*ei) << " referes to it.");

                (*ei)->cond_expr_right->right_args.push_back( *ei );
            }
        }



        empty = num_visible_expr()==0;

        #undef PPP_QUIT
        return true;
    }

    bool Program::is_needed(Expression *ex)
    {
        if (ex->is_funcdef()) return false;

        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        if (*ei != ex)
        {
            Expression * e = *ei;
            for (int i=1; i<=e->num_params; ++i)
            if (e->type[i] == Expression::P_EXPRESSION && e->expr[i] == ex)
                return true;
        }
        return false;
    }

    void Program::reduce_expression(Expression * ex)
    {
        // run and get result
        if (ex->func) ex->exec();
        Float result = *ex->params[0];

        //if (ex->is_conditional()) return;

        // std::cout << ex->string() << " is" << (is_needed(ex)? "":" not") << " needed.\n";

        // don't remove the last expression
        if (num_visible_expr()<2 || !is_needed(ex))
        {
            //if (ex->func_name == "NOP") return;
            PPP_PROG_DEBUG(debug_prefix << "NOPing remaining expression " << ex->string());
            // simply NOP it (result is already calculated)
            ex->func = 0;
            ex->func_name = "NOP";
            // for correct debug output:
            ex->num_params = 1;
            ex->type[1] = Expression::P_CONSTANT;
            ex->value[1] = result;
            ex->final = true;
            return;
        }

        PPP_PROG_DEBUG(debug_prefix << "reducing " << ex->string() << " to " << result);

        // find references that need to be replaced
        for (PExpressions::iterator ei=expr.begin(); ei!=expr.end(); ++ei)
        {
            Expression * e = *ei;
            for (int i=1; i<=e->num_params; ++i)
            if (e->type[i] == Expression::P_EXPRESSION && e->expr[i] == ex)
            {
                e->type[i] = Expression::P_CONSTANT;
                e->value[i] = result;
            }

            // removed a condition for this expression ?
            if (e->cond_expr_left == ex) { e->cond_expr_left = 0;
                PPP_PROG_WARN(e->location, "condition " << PPPS_EXPR(ex) << " removed for left argument " << PPPS_EXPR(e)); }
            if (e->cond_expr_right == ex) { e->cond_expr_right = 0;
                PPP_PROG_WARN(e->location, "condition " << PPPS_EXPR(ex) << " removed for right argument " << PPPS_EXPR(e)); }
        }

        // delete from list
        for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
            if (*i == ex) { expr.erase(i); break; }
    }


    void Program::printSyntax(std::ostream& out, Expression * e) const
    {
        if (expr.empty()) return;

        /* print all top level expressions and their trees */
        if (e == 0)
        {
            bool first=true;
            for (PExpressions::const_iterator i=expr.begin(); i!=expr.end(); ++i)
            {
                if ((*i)->num_needed==0)
                {
                    if (!first) out << "; ";
                    first = false;
                    printSyntax(out, *i);
                }
            }
            return;
        }

        // config
        bool paran_op = true;

        // internal
        bool brace = false,
            printop = false;

        // all functions/operators
        if (e->func)
        {
            bool isbinop = (e->func->type() == Function::BINARY_OP);

            if (isbinop)
            {
                printop = true;
                brace = paran_op;
            }
            else
            {
                out << e->func_name;
                brace = true;
            }
        }

        // function definition
        if (e->is_funcdef())
        {
            out << &e->func_name[1];
            brace = paran_op;
        }

        if (brace) out << "(";
        for (int i=1; i<=e->num_params; ++i)
        {
            if (i>1)
            {
                if (printop)
                    //out << (paran_op? " (":" ") << e->func_name << (paran_op? ") ":" ");
                    out << " " << e->func_name << " ";
                else
                    out << ", ";
            }
            switch (e->type[i])
            {
                case Expression::P_CONSTANT:
                    out << e->value[i];
                break;
                case Expression::P_VARIABLE:
                    out << e->var[i]->name();
                break;
                case Expression::P_EXPRESSION:
                    printSyntax(out, e->expr[i]);
                break;
                case Expression::P_PROGRAM:
                    e->prog[i]->printSyntax(out);
                break;
                case Expression::P_LAMBDA:
                    out << e->lambda_name[i];
                break;
            }
        }
        if (brace) out << ")";

        if (e->is_funcdef())
        {
            out << " { ";
            e->prog[0]->printSyntax(out);
            out << " }";
        }
    }



    // --------------------- runtime --------------------------------

    void Program::printExpressionContent(std::ostream& out) const
    {
        for (PExpressions::const_iterator i=expr.begin(); i!=expr.end(); ++i)
            for (int j=0; j<=(*i)->num_params; ++j)
                out << PPPS_EXPR(*i) << ":" << j << " = " << *(*i)->params[j] << "\n";
    }

    void Program::pushExpressions()
    {
        PPP_PROG_DEBUG(debug_prefix << "*push stack*");
    #ifndef NDEBUG
        debug_prefix += ".";
    #endif

        for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
        {
            if ((*i)->is_conditional())
                cond_stack.push_back((*i)->do_execute);

            for (int j=0; j<=(*i)->num_params; ++j)
            {
        //		PPP_PROG_DEBUG(debug_prefix << "pushing " << PPPS_EXPR(*i) << ":" << j
        //							<< " " << *(*i)->params[j] );
                param_stack.push_back(*(*i)->params[j]);
            }
        }
    }

    void Program::popExpressions()
    {
        PPP_PROG_DEBUG(debug_prefix << "*pop stack*");
    #ifndef NDEBUG
        if (!debug_prefix.empty()) debug_prefix.resize(debug_prefix.size()-1);
    #endif


        for (PExpressions::reverse_iterator i=expr.rbegin(); i!=expr.rend(); ++i)
        {
            if ((*i)->is_conditional())
            {
                if (cond_stack.empty())
                {
                    PPP_PROG_ERROR((*i)->location,
                        "can not pop conditional execution state for expression " << PPPS_EXPR(*i));
                    return;
                }
                (*i)->do_execute = cond_stack.back();
                cond_stack.pop_back();
            }

            for (int j=(*i)->num_params; j>=0; --j)
            {
                if (param_stack.empty())
                {
                    PPP_PROG_ERROR((*i)->location,
                        "can not pop parameter " << j << " of expression " << PPPS_EXPR(*i));
                    return;
                }
        /*		if (*(*i)->params[j] != param_stack.back())
                {
                    PPP_PROG_DEBUG(debug_prefix << "popping " << PPPS_EXPR(*i) << ":" << j
                                    << " " << *(*i)->params[j] << " <- " << param_stack.back() );
                }
        */
                *(*i)->params[j] = param_stack.back();
                param_stack.pop_back();
            }
        }
    }

    void Program::printDot(std::ostream& out)
    {
        out << "graph\n{\n";
        out << "overlap = false; splines = true;\n";

        out << "the_input [label = \""; printSyntax(out); out << "\"; color = green]\n";
        printDotExpressions(out, 0);

        out << "}\n";
    }

    std::string exp_str(Expression * e, int i)
    {
        std::stringstream out;
        switch (e->type[i])
        {
            case Expression::P_CONSTANT: out << "const_" << e->value[i]; break;
            case Expression::P_VARIABLE: out << "var_" << e->var[i]->name(); break;
            case Expression::P_EXPRESSION: out << PPPS_EXPR(e->expr[i]); break;
            case Expression::P_PROGRAM: out << PPPS_PROG(e->prog[i]); break;
            case Expression::P_LAMBDA: out << "@" << e->lambda_name[i]; break;
        }
        return out.str();
    }

    std::string exp_name(Expression * e)
    {
        if (e->func_name == "NOP")
        {
            return "(" + exp_str(e, 1) + ")";
        }
        else
            return e->func_name;
    }

    bool is_exp(Expression * e)
    {
        return (!e->is_funcdef());
    }

    void Program::printDotExpressions(std::ostream& out, int y)
    {
        // definitions
        for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
        if (is_exp(*i))
        {
            Expression * e = *i;
            out << PPPS_EXPR(e) << " [label=\"" << exp_name(e) << "\", pos = \"" << y << ", " << "0" << "\" ]\n";
            y += 10;
        }

        // graph
        for (PExpressions::iterator i=expr.begin(); i!=expr.end(); ++i)
        if (is_exp(*i))
        {
            Expression * e = *i;

            for (int j=1; j<=e->num_params; ++j)
            {
                out << PPPS_EXPR(e) << " -- " << exp_str(e,j)
                    //<< " [label = \"" << *e->params[j] << "\"]"
                    << " [dir = back, label = \"#" << j << "\"]"
                    << ";\n";
            };
        }

        for (PPrograms::const_iterator i=sub.begin(); i!=sub.end(); ++i)
            (*i)->printDotExpressions(out, y);
    }

} // namespace PPP_NAMESPACE


#endif // PARSER_PROGRAM_H_INCLUDED
