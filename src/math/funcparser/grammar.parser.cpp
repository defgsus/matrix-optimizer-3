/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 7 "grammar.y"

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

    int get_the_char()
	{
		skip_comment();
		int c = inp[inp_pos];
		if (c != 0) ++inp_pos;
		return c;
	}

	/** return next char after whitespace, might return 0 */
	int preview_next()
	{
		skip_comment();
		const char * i = &inp[inp_pos];
		while (*i && isspace(*i)) { ++i; i += skip_comment(); }
		return *i;
	}

	bool goto_next(int c)
	{
		skip_comment();
		while (inp[inp_pos] && inp[inp_pos] != c) { ++inp_pos; skip_comment(); }
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

	private:

	/** returns steps made */
	int skip_comment()
	{
		int oldpos = inp_pos;
		
		while (    (inp[inp_pos] == '/' && inp[inp_pos+1] == '/') 
			|| (inp[inp_pos] == '/' && inp[inp_pos+1] == '*') )
		{
			// single line comment
			if (inp[inp_pos] == '/' && inp[inp_pos+1] == '/')
			{
				inp_pos+=2;
				// goto next line
				while (inp[inp_pos] != '\n' && inp[inp_pos] != 0)
					++inp_pos;
				if (!inp[inp_pos]) return inp_pos - oldpos;
				++inp_pos;
			}
						
			// multiline comment
			if (inp[inp_pos] == '/' && inp[inp_pos+1] == '*')
			{
				inp_pos+=2;
				while (inp[inp_pos] && inp[inp_pos] != '*' && inp[inp_pos] != '/' ) 
					++inp_pos;
				if (!inp[inp_pos]) return inp_pos - oldpos;
				++inp_pos;
				if (!inp[inp_pos]) return inp_pos - oldpos;
				++inp_pos;
			}
		}
		return inp_pos - oldpos;
	}

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




/* Line 268 of yacc.c  */
#line 235 "grammar.parser.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUM = 258,
     STR = 259,
     OP2_STR = 260,
     VAR_STR = 261,
     FUNC_STR = 262,
     LFUNC_STR = 263,
     FUNC_DEF_STR = 264,
     LOR_STR = 265,
     LXOR_STR = 266,
     LAND_STR = 267,
     OR_STR = 268,
     XOR_STR = 269,
     AND_STR = 270,
     EQUAL_STR = 271,
     COMP_STR = 272,
     USIGN = 273
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 307 "grammar.parser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   358

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  35
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  11
/* YYNRULES -- Number of rules.  */
#define YYNRULES  59
/* YYNRULES -- Number of states.  */
#define YYNSTATES  116

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    28,     2,     2,     2,    25,     2,     2,
      29,    30,    23,    21,    31,    22,     2,    24,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    12,    34,
       2,    10,     2,    11,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    27,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    32,     2,    33,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    13,    14,    15,    16,    17,
      18,    19,    20,    26
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    14,    21,    30,    41,
      42,    48,    50,    52,    54,    56,    58,    60,    62,    64,
      66,    68,    70,    72,    74,    76,    78,    80,    84,    86,
      88,    90,    92,    96,   100,   104,   108,   112,   116,   120,
     124,   127,   130,   134,   138,   142,   146,   150,   154,   158,
     162,   166,   167,   168,   176,   187,   191,   196,   203,   212
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      36,     0,    -1,    42,    -1,     6,    -1,     4,    -1,     9,
      29,    37,    30,    -1,     9,    29,    37,    31,    37,    30,
      -1,     9,    29,    37,    31,    37,    31,    37,    30,    -1,
       9,    29,    37,    31,    37,    31,    37,    31,    37,    30,
      -1,    -1,    38,    32,    40,    42,    33,    -1,     7,    -1,
      13,    -1,    14,    -1,    15,    -1,    16,    -1,    17,    -1,
      18,    -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,
      23,    -1,    24,    -1,    25,    -1,    27,    -1,    28,    -1,
      42,    34,    43,    -1,    43,    -1,    39,    -1,     6,    -1,
       3,    -1,     4,    10,    43,    -1,     6,    10,    43,    -1,
      43,    21,    43,    -1,    43,    22,    43,    -1,    43,    23,
      43,    -1,    43,    24,    43,    -1,    43,    25,    43,    -1,
      43,    27,    43,    -1,    22,    43,    -1,    21,    43,    -1,
      43,    17,    43,    -1,    43,    16,    43,    -1,    43,    18,
      43,    -1,    43,    14,    43,    -1,    43,    13,    43,    -1,
      43,    15,    43,    -1,    43,    19,    43,    -1,    43,    20,
      43,    -1,    29,    43,    30,    -1,    -1,    -1,    43,    11,
      44,    43,    12,    45,    43,    -1,     8,    29,    41,    31,
      41,    31,    43,    31,    43,    30,    -1,     7,    29,    30,
      -1,     7,    29,    43,    30,    -1,     7,    29,    43,    31,
      43,    30,    -1,     7,    29,    43,    31,    43,    31,    43,
      30,    -1,     7,    29,    43,    31,    43,    31,    43,    31,
      43,    30,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   235,   235,   240,   240,   243,   246,   249,   252,   257,
     257,   273,   274,   275,   276,   277,   278,   279,   280,   280,
     281,   282,   283,   284,   285,   286,   287,   293,   294,   298,
     300,   304,   310,   312,   315,   316,   317,   318,   319,   320,
     322,   323,   328,   329,   330,   331,   332,   333,   337,   338,
     340,   343,   345,   343,   353,   375,   381,   386,   392,   398
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUM", "STR", "OP2_STR", "VAR_STR",
  "FUNC_STR", "LFUNC_STR", "FUNC_DEF_STR", "'='", "'?'", "':'", "LOR_STR",
  "LXOR_STR", "LAND_STR", "OR_STR", "XOR_STR", "AND_STR", "EQUAL_STR",
  "COMP_STR", "'+'", "'-'", "'*'", "'/'", "'%'", "USIGN", "'^'", "'!'",
  "'('", "')'", "','", "'{'", "'}'", "';'", "$accept", "input",
  "MAYBE_VAR", "FUNC_DEF", "FUNC_CREATE", "$@1", "FUNC_PARAM", "vexprseq",
  "vexpr", "$@2", "$@3", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      61,    63,    58,   265,   266,   267,   268,   269,   270,   271,
     272,    43,    45,    42,    47,    37,   273,    94,    33,    40,
      41,    44,   123,   125,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    35,    36,    37,    37,    38,    38,    38,    38,    40,
      39,    41,    41,    41,    41,    41,    41,    41,    41,    41,
      41,    41,    41,    41,    41,    41,    41,    42,    42,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    44,    45,    43,    43,    43,    43,    43,    43,    43
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     4,     6,     8,    10,     0,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     0,     0,     7,    10,     3,     4,     6,     8,    10
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    31,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,    29,     2,    28,     0,     0,     0,     0,     0,
      41,    40,     0,     1,     9,     0,    51,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    32,    33,    55,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,     0,     4,     3,     0,    50,     0,    27,     0,    46,
      45,    47,    43,    42,    44,    48,    49,    34,    35,    36,
      37,    38,    39,    56,     0,     0,     5,     0,     0,     0,
       0,     0,     0,    10,    52,    57,     0,     0,     6,     0,
       0,     0,     0,     0,    53,    58,     0,     0,     7,     0,
       0,     0,     0,    59,    54,     8
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    10,    64,    11,    12,    66,    61,    13,    14,    68,
     100
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -44
static const yytype_int16 yypact[] =
{
      43,   -44,    -4,    28,    19,    47,    58,    43,    43,    43,
      53,    59,   -44,    60,   259,    43,    43,    33,    88,    39,
      65,    65,   188,   -44,   -44,    43,   -44,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,   259,   259,   -44,   106,   -44,   -44,   -44,   -44,   -44,
     -44,   -44,   -44,   -44,   -44,   -44,   -44,   -44,   -44,   -44,
     -44,    66,   -44,   -44,     4,   -44,    43,   259,    43,   288,
     301,   313,    61,   323,    -8,   331,    46,   -20,   -20,    65,
      65,    65,    65,   -44,    43,    88,   -44,    39,    24,   244,
     127,    67,    29,   -44,   -44,   -44,    43,    43,   -44,    39,
      43,   148,   169,    44,   274,   -44,    43,    43,   -44,    39,
     206,   224,    84,   -44,   -44,   -44
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -44,   -44,   -43,   -44,   -44,   -44,    11,    52,    -7,   -44,
     -44
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      20,    21,    22,    37,    38,    39,    15,    40,    41,    42,
      44,    33,    34,    35,    36,    37,    38,    39,    67,    40,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    86,    87,     1,     2,    16,     3,
       4,     5,     6,    62,    92,    63,     1,     2,    17,     3,
       4,     5,     6,    23,     7,     8,   103,    93,    25,    98,
      99,    89,     9,    43,     7,     8,   112,    35,    36,    37,
      38,    39,     9,    40,   108,   109,    18,    90,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    19,    40,   101,
     102,    24,    40,   104,    25,    45,    91,    85,    97,   110,
     111,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,   115,    59,    60,    26,    88,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,     0,    40,     0,     0,    83,    84,    26,     0,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,     0,    40,     0,     0,    95,    96,    26,
       0,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,     0,    40,     0,     0,   105,   106,
      26,     0,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,     0,    40,     0,     0,    26,
     107,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,     0,    40,     0,    26,    65,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,     0,    40,     0,    26,   113,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
       0,    40,     0,     0,   114,    26,    94,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      26,    40,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,     0,    40,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
       0,    40,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,     0,    40,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,     0,    40,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,     0,
      40,    32,    33,    34,    35,    36,    37,    38,    39,     0,
      40,    34,    35,    36,    37,    38,    39,     0,    40
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-44))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int8 yycheck[] =
{
       7,     8,     9,    23,    24,    25,    10,    27,    15,    16,
      17,    19,    20,    21,    22,    23,    24,    25,    25,    27,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    30,    31,     3,     4,    10,     6,
       7,     8,     9,     4,    87,     6,     3,     4,    29,     6,
       7,     8,     9,     0,    21,    22,    99,    33,    34,    30,
      31,    68,    29,    30,    21,    22,   109,    21,    22,    23,
      24,    25,    29,    27,    30,    31,    29,    84,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    29,    27,    96,
      97,    32,    27,   100,    34,     7,    85,    31,    31,   106,
     107,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    30,    27,    28,    11,    66,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    27,    -1,    -1,    30,    31,    11,    -1,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    27,    -1,    -1,    30,    31,    11,
      -1,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    27,    -1,    -1,    30,    31,
      11,    -1,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    27,    -1,    -1,    11,
      31,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    27,    -1,    11,    30,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    27,    -1,    11,    30,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    27,    -1,    -1,    30,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      11,    27,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    27,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    27,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    27,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    -1,    27,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      27,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      27,    20,    21,    22,    23,    24,    25,    -1,    27
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     6,     7,     8,     9,    21,    22,    29,
      36,    38,    39,    42,    43,    10,    10,    29,    29,    29,
      43,    43,    43,     0,    32,    34,    11,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      27,    43,    43,    30,    43,     7,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    27,
      28,    41,     4,     6,    37,    30,    40,    43,    44,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    30,    31,    31,    30,    31,    42,    43,
      43,    41,    37,    33,    12,    30,    31,    31,    30,    31,
      45,    43,    43,    37,    43,    30,    31,    31,    30,    31,
      43,    43,    37,    30,    30,    30
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 235 "grammar.y"
    { 	YYACCEPT; }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 244 "grammar.y"
    { (yyval.funcdef) = PPP_NAMESPACE::FuncDef((yyloc), (yyvsp[(1) - (4)].str), (yylsp[(1) - (4)]), {(yyvsp[(3) - (4)].str)}, {(yylsp[(3) - (4)])}); }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 247 "grammar.y"
    { (yyval.funcdef) = PPP_NAMESPACE::FuncDef((yyloc), (yyvsp[(1) - (6)].str), (yylsp[(1) - (6)]), {(yyvsp[(3) - (6)].str), (yyvsp[(5) - (6)].str)}, {(yylsp[(3) - (6)]), (yylsp[(5) - (6)])}); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 250 "grammar.y"
    { (yyval.funcdef) = PPP_NAMESPACE::FuncDef((yyloc), (yyvsp[(1) - (8)].str), (yylsp[(1) - (8)]), {(yyvsp[(3) - (8)].str), (yyvsp[(5) - (8)].str), (yyvsp[(7) - (8)].str)}, {(yylsp[(3) - (8)]), (yylsp[(5) - (8)]), (yylsp[(7) - (8)])}); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 253 "grammar.y"
    { (yyval.funcdef) = PPP_NAMESPACE::FuncDef((yyloc), (yyvsp[(1) - (10)].str), (yylsp[(1) - (10)]), {(yyvsp[(3) - (10)].str), (yyvsp[(5) - (10)].str), (yyvsp[(7) - (10)].str), (yyvsp[(9) - (10)].str)}, {(yylsp[(3) - (10)]), (yylsp[(5) - (10)]), (yylsp[(7) - (10)]), (yylsp[(9) - (10)])}); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 257 "grammar.y"
    {	P->prog->potential_functions.insert( (yyvsp[(1) - (2)].funcdef).name );
										PPP_PROG_DEBUG(P->prog->debug_prefix << "creating sub-program {");
										P->prog = P->prog->getSubProgram();
										if (!P->prog->addFuncDef((yyvsp[(1) - (2)].funcdef))) YYERROR;
									}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 263 "grammar.y"
    {	PPP_NAMESPACE::Program * fprog = P->prog;
										PPP_PROG_DEBUG(P->prog->debug_prefix << "} leaving sub-program");
										if (!P->prog->parent) { PPP_PROG_ERROR((yylsp[(5) - (5)]), "internal logic: defined program not pushed!?!"); YYABORT; }
										P->prog = P->prog->parent;
										if (!P->prog->createFunction((yyvsp[(1) - (5)].funcdef), fprog, (yylsp[(4) - (5)]))) YYERROR;
									}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 273 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 274 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 275 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 276 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 277 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 278 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 279 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 280 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 281 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 282 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 283 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 284 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 285 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 286 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 287 "grammar.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 293 "grammar.y"
    { }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 294 "grammar.y"
    { }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 298 "grammar.y"
    { }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 300 "grammar.y"
    { 	if (PPP_NAMESPACE::Variable * v = P->prog->has_variable((yyvsp[(1) - (1)].str)))
											(yyval.expr) = P->prog->add((yyloc), "NOP", v, (yylsp[(1) - (1)]));
										else PPP_ERR((yylsp[(1) - (1)]), "internal logic: VAR_STR '" << (yyvsp[(1) - (1)].str) << "' for unknown variable."); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 304 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), "NOP", (yyvsp[(1) - (1)].num), (yylsp[(1) - (1)])); }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 310 "grammar.y"
    { (yyval.expr) = P->prog->assignment((yyloc), (yyvsp[(1) - (3)].str), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 312 "grammar.y"
    { //PPP_PROG_WARN((yylsp[(1) - (3)]), "overwriting non-local variable '"<<(yyvsp[(1) - (3)].str)<<"'");
        (yyval.expr) = P->prog->assignment((yyloc), (yyvsp[(1) - (3)].str), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 315 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 316 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 317 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 318 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 319 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 320 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 322 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].expr), (yylsp[(2) - (2)]) ); }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 323 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (2)].str), (yyvsp[(2) - (2)].expr), (yylsp[(2) - (2)]) ); }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 328 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 329 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 330 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 331 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 332 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 333 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 337 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 338 "grammar.y"
    { (yyval.expr) = P->prog->add((yyloc), (yyvsp[(2) - (3)].str), (yyvsp[(1) - (3)].expr), (yylsp[(1) - (3)]), (yyvsp[(3) - (3)].expr), (yylsp[(3) - (3)])); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 340 "grammar.y"
    { (yyval.expr) = (yyvsp[(2) - (3)].expr); /* pass the pointer, don't create new expression
												XXX probably spoils location!*/ }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 343 "grammar.y"
    { P->prog->startConditionLeft((yyvsp[(1) - (2)].expr)); }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 345 "grammar.y"
    { P->prog->startConditionRight((yyvsp[(1) - (5)].expr)); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 346 "grammar.y"
    { P->prog->stopCondition();
									  (yyval.expr) = P->prog->add((yyloc), "?", (yyvsp[(1) - (7)].expr), (yylsp[(1) - (7)]), (yyvsp[(4) - (7)].expr), (yylsp[(4) - (7)]), (yyvsp[(7) - (7)].expr), (yylsp[(7) - (7)]));
									  (yyvsp[(1) - (7)].expr)->setConditionArgs((yyvsp[(4) - (7)].expr), (yyvsp[(7) - (7)].expr));
									}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 354 "grammar.y"
    {
									  if (P->funcs->function((yyvsp[(1) - (10)].str), PPP_NAMESPACE::Function::LAMBDA, 4)/* || P->prog->has_potential_function($1)*/)
									  {
										(yyval.expr) = P->prog->addLambda((yyloc), (yyvsp[(1) - (10)].str), (yyvsp[(3) - (10)].str), (yylsp[(3) - (10)]), (yyvsp[(5) - (10)].str), (yylsp[(5) - (10)]), (yyvsp[(7) - (10)].expr), (yylsp[(7) - (10)]), (yyvsp[(9) - (10)].expr), (yylsp[(9) - (10)]));
									  }
									  else
										PPP_ERR((yylsp[(1) - (10)]), "no lambda function '" << (yyvsp[(1) - (10)].str) << "' with matching paramaters");
									}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 375 "grammar.y"
    { if (P->funcs->function((yyvsp[(1) - (3)].str), 0) || P->prog->has_potential_function((yyvsp[(1) - (3)].str)))
										(yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (3)].str));
									  else
										PPP_ERR((yylsp[(1) - (3)]), "number of parameters not matching function '" << (yyvsp[(1) - (3)].str) << "'");
									}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 381 "grammar.y"
    { if (P->funcs->function((yyvsp[(1) - (4)].str), 1) || P->prog->has_potential_function((yyvsp[(1) - (4)].str)))
										(yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (4)].str), (yyvsp[(3) - (4)].expr), (yylsp[(3) - (4)]));
									  else
										PPP_ERR((yylsp[(1) - (4)]), "number of parameters not matching function '" << (yyvsp[(1) - (4)].str) << "'");
									}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 387 "grammar.y"
    { if (P->funcs->function((yyvsp[(1) - (6)].str), 2) || P->prog->has_potential_function((yyvsp[(1) - (6)].str)))
										(yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (6)].str), (yyvsp[(3) - (6)].expr), (yylsp[(3) - (6)]), (yyvsp[(5) - (6)].expr), (yylsp[(5) - (6)]));
									  else
										PPP_ERR((yylsp[(1) - (6)]), "number of parameters not matching function '" << (yyvsp[(1) - (6)].str) << "'");
									}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 393 "grammar.y"
    { if (P->funcs->function((yyvsp[(1) - (8)].str), 3) || P->prog->has_potential_function((yyvsp[(1) - (8)].str)))
										(yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (8)].str), (yyvsp[(3) - (8)].expr), (yylsp[(3) - (8)]), (yyvsp[(5) - (8)].expr), (yylsp[(5) - (8)]), (yyvsp[(7) - (8)].expr), (yylsp[(7) - (8)]));
									  else
										PPP_ERR((yylsp[(1) - (8)]), "number of parameters not matching function '" << (yyvsp[(1) - (8)].str) << "'");
									}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 399 "grammar.y"
    { if (P->funcs->function((yyvsp[(1) - (10)].str), 4) || P->prog->has_potential_function((yyvsp[(1) - (10)].str)))
										(yyval.expr) = P->prog->add((yyloc), (yyvsp[(1) - (10)].str), (yyvsp[(3) - (10)].expr), (yylsp[(3) - (10)]), (yyvsp[(5) - (10)].expr), (yylsp[(5) - (10)]), (yyvsp[(7) - (10)].expr), (yylsp[(7) - (10)]), (yyvsp[(9) - (10)].expr), (yylsp[(9) - (10)]));
									  else
										PPP_ERR((yylsp[(1) - (10)]), "number of parameters not matching function '" << (yyvsp[(1) - (10)].str) << "'");
									}
    break;



/* Line 1806 of yacc.c  */
#line 2146 "grammar.parser.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 429 "grammar.y"



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
    while ((c = P->get_the_char()) == ' ' || c == '\t')
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

#define PPP_LEX_DEBUG(stream_arg__) \
	PPP_PROG_DEBUG(P->prog->debug_prefix << "lexed: " << stream_arg__ << " -> " << PPPS_PROG(P->prog) )

/** reentrant lexer */
int yylex (YYSTYPE * lhs, YYLTYPE * loc, void * YYPARSE_PARAM)
{
	int c;

	/* Skip white space.  */
    while ((c = P->get_the_char()) == ' ' || c == '\t' || c == '\n')
		++loc->last_column;

	/* step here */
	loc->first_line = loc->last_line;
	loc->first_column = loc->last_column;

	/* Return end-of-input.  */
	if (c == 0 || c == EOF)
	{
		PPP_LEX_DEBUG("eof");
		return 0;
	}

//	++loc->last_column;

	/* operator? */
	if (is_operator_char(c))
	{
		lhs->str = (char)c;
//		std::cout << "first " << lhs->str << "\n";
		// second char?
        c = P->get_the_char();
		if (is_operator_char(c))
		{
			++loc->last_column;
			std::string str = lhs->str + (char)c;
			int t = operator_type(str);
			if (t != STR)
			{
				// known two-char operator
				lhs->str = str;
				PPP_LEX_DEBUG("(two-char op) " << lhs->str)
				return lhs->type = t;
			}
			// only single char?
		}
		P->back();
		--loc->last_column;
//		std::cout << "return " << lhs->str << "\n";
		// single char operator / or unknown
		PPP_LEX_DEBUG("(op) " << lhs->str)
		return lhs->type = operator_type(lhs->str);
	}

	/* Process numbers. */
	if (c == '.' || isdigit (c))
    {
		lhs->str.clear();
		bool hasexp=false;
		do
		{
            if (c == 'e') hasexp = true;
            lhs->str += char(c);
            c = P->get_the_char();
			++loc->last_column;
        }
        while (isdigit(c) || c == '.' || (c == 'e' && !hasexp) || (hasexp && (c=='+' || c=='-')));
		P->back();

	#ifdef PPP_TTMATH
		lhs->num = lhs->str;
	#else
        lhs->num = QString::fromStdString(lhs->str).toDouble();
                // XXX below seems to return ints only in some cases
                //std::strtod(lhs->str.c_str(), 0);
	#endif
		PPP_LEX_DEBUG("(num) " << lhs->str)
        //std::cout << "CONV: [" << lhs->str << "] to " << lhs->num << std::endl;
		return lhs->type = NUM;
    }

	/* Process letters */
    if (is_alpha_char(c))
    {
		lhs->str.clear();
		do
		{
			lhs->str += c;
            c = P->get_the_char();
			++loc->last_column;
		}
		while (is_alpha_char(c) || isdigit(c));
		P->back();

		if (lhs->str == "xor")
		{
			PPP_LEX_DEBUG("(op) " << lhs->str)
			return lhs->type = XOR_STR;
		}

		// check for variable
		PPP_NAMESPACE::Variable * v = P->prog->has_variable(lhs->str);

		// return constants as floats
		if (v && v->isConst())
		{
			lhs->num = v->value();
			PPP_LEX_DEBUG("(const) " << lhs->str)
			return lhs->type = NUM;
		}
		// other variables as strings
		if (v)
		{
			PPP_LEX_DEBUG("(var) " << lhs->str)
			return lhs->type = VAR_STR;
		}

		// see if this is meant as a function declaration/definition
		P->pushpos();
		/* XXX hacky */
        bool is_def = (P->preview_next()=='(' && P->goto_next('(') && P->goto_next(')') && P->get_the_char() && P->preview_next()=='{');
		P->poppos();

		if (is_def)
		{
			PPP_PROG_DEBUG(P->prog->debug_prefix << "function definition '" << lhs->str << "'");
			PPP_LEX_DEBUG("(func def) " << lhs->str)
			return lhs->type = FUNC_DEF_STR;
		}

		// check for function name
		PPP_NAMESPACE::Function * fu = P->funcs->function(lhs->str, PPP_NAMESPACE::Function::FUNCTION, -1);
		if (fu || P->prog->has_potential_function(lhs->str))
		{
			PPP_LEX_DEBUG("(func name) " << lhs->str)
			return lhs->type = FUNC_STR;
		}

		// check for lambda function name
		fu = P->funcs->function(lhs->str, PPP_NAMESPACE::Function::LAMBDA, -1);
		if (fu)
		{
			PPP_LEX_DEBUG("(lfunc name) " << lhs->str)
			return lhs->type = LFUNC_STR;
		}

		/*if (lhs->str == "let")
			return lhs->type = LET_STR;*/

		// return unkown string
		PPP_LEX_DEBUG("(str) " << lhs->str)
		return STR;
    }

    if (c == '\n')
    {
		loc->first_column = loc->last_column = 1;
		++loc->last_line;
	}

	/* Return a single whatever.  */
	lhs->str = (char)c;
	PPP_LEX_DEBUG("(type " << c << ") " << lhs->str)
	return lhs->type = c;
}
#endif


void yyerror (const char * s)
{
	printf("yy: %s\n", s);
}

#undef P

