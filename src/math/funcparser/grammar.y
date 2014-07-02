/* 	@brief function parser bison file

	@version 2013/29/09 started
	@author nerdibergi

	@note invoke with bison -o *output*.cpp
*/

%{
/* ------------- C declarations ------------- */

#include <cmath>
#include <cctype>
#include <cstdlib>
#include <cstdio>

#include <string>
#include <iostream>
#include <sstream>

#define DEBUG(str_stream_) { std::cout << "[" << str_stream_ << "] "; }
#define DATA_(data_) "(" << (void*)(((unsigned int)&data_)%361) << ")"
#define BRACE_(stream_arg_) "{" << stream_arg_ << "}"

#include "parser_defines.h"
#include "parser_program.h"

/* Parameter exchange between C++ Parser -> yyparse().
	unfortunately this ended up as a class that does a bit of parsing as well */
struct ParseParam
{
	const char * inp;
	size_t inp_pos;
	std::vector<size_t> pos_stack_;

	/** current output from yyparser */
	PPP_NAMESPACE::Program * prog;

	/** variables in Parser class */
	PPP_NAMESPACE::Variables * vars;

	/** installed functions in parser */
	PPP_NAMESPACE::Functions * funcs;

	int getchar()
	{
		int c = inp[inp_pos];
		if (c != 0) ++inp_pos;
		return c;
	}

	/** return next char after whitespace, might return 0 */
	int preview_next()
	{
		const char * i = &inp[inp_pos];
		while (*i && isspace(*i)) ++i;
		return *i;
	}

	bool goto_next(int c)
	{
		while (inp[inp_pos] && inp[inp_pos] != c) ++inp_pos;
		//std::cout << "goto-next " << (char)c << " = " <<  inp[inp_pos] << "\n";
		return (inp[inp_pos] == c);
	}

	void back() { if (inp_pos) --inp_pos; }

	void pushpos() { pos_stack_.push_back(inp_pos);
		//std::cout << "push: " << inp_pos << "\n";
		}
	void poppos() { if (!pos_stack_.empty()) { inp_pos = pos_stack_.back(); pos_stack_.pop_back();
		//std::cout << "pop: " << inp_pos << "\n";
		} }
};

/** all data types (instead of bison union) */
struct Data
{
	int type;

	PPP_NAMESPACE::Float num;
	std::string str;

	/** current program */
	PPP_NAMESPACE::Program * prog;
	PPP_NAMESPACE::Expression * expr;

	PPP_NAMESPACE::FuncDef funcdef;
};


#define YYSTYPE Data
#define YYPARSE_PARAM param
#define YYLEX_PARAM param

#define P ((ParseParam*)(param))

#define PPP_CONST_OP(T0__, T1__, T2__, T3__) \
	DEBUG(T1__ << " " << T2__ << /*"(" << typeid(T2__).name() << ")*/" " << T3__) \
	PPP_NAMESPACE::FuncPtr2 f = PPP_NAMESPACE::math_func<PPP_NAMESPACE::Float>::get_binary_op_func(T2__); \
	if (f) \
	{	PPP_NAMESPACE::Func ff(f, T1__, T3__); \
		ff.exec(); \
		T0__ = ff.res; \
	} \
	else \
	{	\
		std::cout << "unknown operator '" << T2__ << "'\n"; \
		YYABORT; \
	}

#define PPP_EXPR_OP(T0__, T1__, T2__, T3__) \
	DEBUG(T1__ << " " << T2__ << /*"(" << typeid(T2__).name() << ")*/" " << T3__) \
	PPP_NAMESPACE::FuncPtr2 f = PPP_NAMESPACE::math_func<PPP_NAMESPACE::Float>::get_binary_op_func(T2__); \
	if (f) \
	{	P->prog->add( T0__ = new PPP_NAMESPACE::Func(f, T1__, T3__) ); \
	} \
	else \
	{	\
		std::cout << "unknown operator '" << T2__ << "'\n"; \
		YYABORT; \
	}


int yylex (YYSTYPE * data, YYLTYPE * loc, void * param);
void yyerror (const char * s);


%}



/* ------------- Bison declarations ------------- */

/** request threadsafe parser */
%pure_parser

/* --- non-single-character literals --- */

%token <num> NUM
/** general string */
%token <str> STR;
/* binary operator string */
%token <str> OP2_STR;
/** string of a variable name (which is no constant) */
%token <str> VAR_STR;
/** string of a function name */
%token <str> FUNC_STR;
/** string of a function name in definition */
%token <str> FUNC_DEF_STR;


/** --- typed rules --- */

/** a function definition */
%type <funcdef> FUNC_DEF;
%type <str> 	MAYBE_VAR;
/** basic expression (tree) */
%type <expr> 	vexpr
/** expression with conditional execution */
/*%type <expr> 	condexpr*/

/*
%type <num> Float_
%type <num> Integer_
%type <num> Digit_
*/

/* associativity */
%left 			'=' /* must be lowest */
%left <str>		'?' ':'
%left <str>		LOR_STR
%left <str>		LXOR_STR
%left <str>		LAND_STR
%left <str>		OR_STR
%left <str>		XOR_STR
%left <str>		AND_STR
%left <str> 	EQUAL_STR
%left <str>		COMP_STR
%left 			'+' '-'
%left 			'*' '/' '%'

%left 			USIGN
%right			'^'
%left			'!'

%start input

%%
/* ------------- Grammar rules ------------- */

input:		vexprseq 			{ 	YYACCEPT; }


/* ------- function definition ------- */

MAYBE_VAR:  VAR_STR | STR;

FUNC_DEF:
			FUNC_DEF_STR '(' MAYBE_VAR ')'
									{ $$ = PPP_NAMESPACE::FuncDef(@$, $1, @1, {$3}, {@3}); }

		|	FUNC_DEF_STR '(' MAYBE_VAR ',' MAYBE_VAR ')'
									{ $$ = PPP_NAMESPACE::FuncDef(@$, $1, @1, {$3, $5}, {@3, @5}); }
	;

FUNC_CREATE:
			FUNC_DEF '{'			{	P->prog->potential_functions.insert( $1.name );
										PPP_PROG_DEBUG(P->prog->debug_prefix << "creating sub-program {");
										P->prog = P->prog->getSubProgram();
										if (!P->prog->addFuncDef($1)) YYERROR;
									}

				vexprseq '}'		{	PPP_NAMESPACE::Program * fprog = P->prog;
										PPP_PROG_DEBUG(P->prog->debug_prefix << "} leaving sub-program");
										if (!P->prog->parent) { PPP_PROG_ERROR(@5, "internal logic: defined program not pushed!?!"); YYABORT; }
										P->prog = P->prog->parent;
										if (!P->prog->createFunction($1, fprog, @4)) YYERROR;
									}
;

/* ------ expression ----- */

vexprseq:
			vexprseq ';' vexpr 		{ }
		|	vexpr 					{ }
	;

vexpr:
			FUNC_CREATE				{ }

		|	VAR_STR					{ 	if (PPP_NAMESPACE::Variable * v = P->prog->has_variable($1))
											$$ = P->prog->add(@$, "NOP", v, @1);
										else PPP_ERR(@1, "internal logic: VAR_STR '" << $1 << "' for unknown variable."); }

		|	NUM						{ $$ = P->prog->add(@$, "NOP", $1, @1); }
/*
		|	Integer_				{ $$ = P->prog->add(@$, "NOP", $1, @1); }
		|	Float_					{ $$ = P->prog->add(@$, "NOP", $1, @1); }
*/
	/* new variable */
		|	STR '=' vexpr			{ $$ = P->prog->assignment(@$, $1, @1, $3, @3); }
	/* or overwrite */
		|	VAR_STR '=' vexpr		{ PPP_PROG_WARN(@1, "overwriting non-local variable '"<<$1<<"'");
										$$ = P->prog->assignment(@$, $1, @1, $3, @3); }

		|	vexpr '+' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }
		|	vexpr '-' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }
		|	vexpr '*' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }
		|	vexpr '/' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }
		|	vexpr '%' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }
		|	vexpr '^' vexpr			{ $$ = P->prog->add(@$, $<str>2, $1, @1, $3, @3); }

		|	'-' vexpr %prec USIGN	{ $$ = P->prog->add(@$, $<str>1, $2, @2 ); }
		|	'+' vexpr %prec USIGN	{ $$ = P->prog->add(@$, $<str>1, $2, @2 ); }

/** @todo vexpr '!' keeps parsing forever... */
/*		|	vexpr '!' 				{ $$ = P->prog->add(@$, $<str>2, $1, @1 ); } */

		|	vexpr XOR_STR vexpr		{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr OR_STR vexpr		{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr AND_STR vexpr		{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr LXOR_STR vexpr	{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr LOR_STR vexpr		{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr LAND_STR vexpr	{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }

/*		|	vexpr '=' '=' vexpr %prec EQUAL_STR 	{ $$ = P->prog->add(@$, "==", $1, @1, $4, @4); }*/

		|	vexpr EQUAL_STR vexpr	{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }
		|	vexpr COMP_STR vexpr	{ $$ = P->prog->add(@$, $2, $1, @1, $3, @3); }

		|	'(' vexpr ')'			{ $$ = $2; /* pass the pointer, don't create new expression
												XXX probably spoils location!*/ }

		|	vexpr '?' 				{ P->prog->startConditionLeft($1); }
					vexpr
				':' 				{ P->prog->startConditionRight($1); }
					vexpr			{ P->prog->stopCondition();
									  $$ = P->prog->add(@$, "?", $1, @1, $4, @4, $7, @7);
									  $1->setConditionArgs($4, $7);
									}

	/* catch unknown function calls */
		|	STR '(' ')'				{ if (!P->prog->has_potential_function($1)) PPP_ERR(@1, "unknown function '" << $1 << "'"); }
		|	STR '(' vexpr ')'
									{ if (!P->prog->has_potential_function($1)) PPP_ERR(@1, "unknown function '" << $1 << "'"); }
		|	STR '(' vexpr ',' vexpr ')'
									{ if (!P->prog->has_potential_function($1)) PPP_ERR(@1, "unknown function '" << $1 << "'"); }
		|	STR '(' vexpr ',' vexpr ',' vexpr ')'
									{ if (!P->prog->has_potential_function($1)) PPP_ERR(@1, "unknown function '" << $1 << "'"); }
		|	STR '(' vexpr ',' vexpr ',' vexpr ',' vexpr ')'
									{ if (!P->prog->has_potential_function($1)) PPP_ERR(@1, "unknown function '" << $1 << "'"); }

	/* function calls */
		| 	FUNC_STR '(' ')'		{ if (P->funcs->function($1, 0) || P->prog->has_potential_function($1))
										$$ = P->prog->add(@$, $1);
									  else
										PPP_ERR(@1, "number of parameters not matching function '" << $1 << "'");
									}

		| 	FUNC_STR '(' vexpr ')'	{ if (P->funcs->function($1, 1) || P->prog->has_potential_function($1))
										$$ = P->prog->add(@$, $1, $3, @3);
									  else
										PPP_ERR(@1, "number of parameters not matching function '" << $1 << "'");
									}
		| 	FUNC_STR '(' vexpr ',' vexpr ')'
									{ if (P->funcs->function($1, 2) || P->prog->has_potential_function($1))
										$$ = P->prog->add(@$, $1, $3, @3, $5, @5);
									  else
										PPP_ERR(@1, "number of parameters not matching function '" << $1 << "'");
									}
		| 	FUNC_STR '(' vexpr ',' vexpr ',' vexpr ')'
									{ if (P->funcs->function($1, 3) || P->prog->has_potential_function($1))
										$$ = P->prog->add(@$, $1, $3, @3, $5, @5, $7, @7);
									  else
										PPP_ERR(@1, "number of parameters not matching function '" << $1 << "'");
									}
		| 	FUNC_STR '(' vexpr ',' vexpr ',' vexpr ',' vexpr ')'
									{ if (P->funcs->function($1, 4) || P->prog->has_potential_function($1))
										$$ = P->prog->add(@$, $1, $3, @3, $5, @5, $7, @7, $9, @9);
									  else
										PPP_ERR(@1, "number of parameters not matching function '" << $1 << "'");
									}
/*
Float_:		Integer_ '.' 			{ $$ = $1; }
		|	'.' Integer_ 			{ $$ = $2 / std::pow(10, PPP_NAMESPACE::num_digits($2)); }
		|	Integer_ '.' Integer_ 	{ $$ = $1 + $3 / std::pow(10, PPP_NAMESPACE::num_digits($3)); }
	;

Integer_:	Digit_					{ $$ = $1; }
		|	Integer_ Digit_			{ $$ = $1 * 10 + $2; }
	;

Digit_ :	'0'						{ $$ = 0; }
		|	'1'						{ $$ = 1; }
		|	'2'						{ $$ = 2; }
		|	'3'						{ $$ = 3; }
		|	'4'						{ $$ = 4; }
		|	'5'						{ $$ = 5; }
		|	'6'						{ $$ = 6; }
		|	'7'						{ $$ = 7; }
		|	'8'						{ $$ = 8; }
		|	'9'						{ $$ = 9; }
	;
*/

%%


/* ------------- additional C code ------------- */

bool is_alpha_char(int c)
{
	return isalpha(c) || c == '_';
}

bool is_operator_char(int c)
{
	return	   c == '+' || c == '-'
			|| c == '*' || c == '/'
			|| c == '^' || c == '!'
			|| c == '&' || c == '|'
			|| c == '%' || c == '='
			|| c == '<' || c == '>';
}

/* returns operator type.
	returns STR for unknown multicharacter strings.
	asciicode for unknown single characters. */
int operator_type(const std::string& s)
{
	if (s == "&&") return LAND_STR; else
	if (s == "||") return LOR_STR; else
	if (s == "^^") return LXOR_STR; else
	if (s == "&") return AND_STR; else
	if (s == "|") return OR_STR; else
	if (s == "=="
	 || s == "!=") return EQUAL_STR; else
	if (s == "<"
	 || s == "<="
	 || s == ">"
	 || s == ">=") return COMP_STR; else
	 // unknown string
	 if (s.size()>1) return STR; else
	 // return single char
	 return s[0];
}

#if 0

/** reentrant lexer */
int yylex (YYSTYPE * lhs, YYLTYPE * loc, void * YYPARSE_PARAM)
{
	int c;

	/* Skip white space.  */
	while ((c = P->getchar()) == ' ' || c == '\t')
		++loc->last_column;

	/* step here */
	loc->first_line = loc->last_line;
	loc->first_column = loc->last_column;

	/* Return end-of-input.  */
	if (c == 0 || c == EOF) return 0;

//	++loc->last_column;

    if (c == '\n')
    {
		loc->first_column = loc->last_column = 1;
		++loc->last_line;
	}

	/* Return a single whatever.  */
	lhs->str = c;
	return lhs->type = c;
}

#else
/** reentrant lexer */
int yylex (YYSTYPE * lhs, YYLTYPE * loc, void * YYPARSE_PARAM)
{
	int c;

	/* Skip white space.  */
	while ((c = P->getchar()) == ' ' || c == '\t')
		++loc->last_column;

	/* step here */
	loc->first_line = loc->last_line;
	loc->first_column = loc->last_column;

	/* Return end-of-input.  */
	if (c == 0 || c == EOF) return 0;

//	++loc->last_column;

	/* operator? */
	if (is_operator_char(c))
	{
		lhs->str = (char)c;
//		std::cout << "first " << lhs->str << "\n";
		// second char?
		c = P->getchar();
		if (is_operator_char(c))
		{
			++loc->last_column;
			std::string str = lhs->str + (char)c;
			int t = operator_type(str);
			if (t != STR)
			{
				// known two-char operator
				lhs->str = str;
				return lhs->type = t;
			}
			// only single char?
		}
		P->back();
		--loc->last_column;
//		std::cout << "return " << lhs->str << "\n";
		// single char operator / or unknown
		return lhs->type = operator_type(lhs->str);
	}

	/* Process numbers. */
	if (c == '.' || isdigit (c))
    {
		lhs->str.clear();
		bool hasexp=false;
		do
		{
			lhs->str += c;
			c = P->getchar();
			++loc->last_column;
			if (c == 'e') hasexp = true;
		}
		while (isdigit(c) || c == '.' || (hasexp && (c=='+' || c=='-')));
		P->back();

	#ifdef PPP_TTMATH
		lhs->num = lhs->str;
	#else
		lhs->num = atof(lhs->str.c_str());
	#endif
		return lhs->type = NUM;
    }

	/* Process letters */
    if (is_alpha_char(c))
    {
		lhs->str.clear();
		do
		{
			lhs->str += c;
			c = P->getchar();
			++loc->last_column;
		}
		while (is_alpha_char(c) || isdigit(c));
		P->back();
	#ifndef NDEBUG
		std::cout << P->prog->debug_prefix << "lexxed: \"" << lhs->str << "\" -> " << PPPS_PROG(P->prog) << "\n";
	#endif

		if (lhs->str == "xor")
			return lhs->type = XOR_STR;

		// check for variable
		PPP_NAMESPACE::Variable * v = P->prog->has_variable(lhs->str);

		// return constants as floats
		if (v && v->isConst())
		{
			lhs->num = v->value();
			return lhs->type = NUM;
		}
		// other variables as strings
		if (v) return lhs->type = VAR_STR;

		// see if this is meant as a function declaration/definition
		P->pushpos();
		/* XXX hacky */
		bool is_def = (P->preview_next()=='(' && P->goto_next('(') && P->goto_next(')') && P->getchar() && P->preview_next()=='{');
		P->poppos();

		if (is_def)
		{
			PPP_PROG_DEBUG(P->prog->debug_prefix << "function definition '" << lhs->str << "'");
			return lhs->type = FUNC_DEF_STR;
		}

		// check for function name
		PPP_NAMESPACE::Function * fu = P->funcs->function(lhs->str, PPP_NAMESPACE::Function::FUNCTION, -1);
		if (fu || P->prog->has_potential_function(lhs->str))
		{
			PPP_PROG_DEBUG(P->prog->debug_prefix << "function call '" << lhs->str << "'");
			return lhs->type = FUNC_STR;
		}

		/*if (lhs->str == "let")
			return lhs->type = LET_STR;*/

		// return unkown string
		return STR;
    }

    if (c == '\n')
    {
		loc->first_column = loc->last_column = 1;
		++loc->last_line;
	}

	/* Return a single whatever.  */
	lhs->str = c;
	return lhs->type = c;
}
#endif


void yyerror (const char * s)
{
	printf("yy: %s\n", s);
}

#undef P
