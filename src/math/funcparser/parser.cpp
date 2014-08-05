/**	@file

    @brief Math Function Parser

    @author def.gsus-
    @version 2013/09/30 started
*/
#include <set>
#include <fstream>

#include "parser.h"
#include "math/constants.h"

#include "grammar.parser.cpp"

#if 0
#	define MF_DEBUG_(stream_arg__) std::cout << stream_arg__ << "\n";
#else
#	define MF_DEBUG_(stream_arg__)
#endif

namespace PPP_NAMESPACE
{



// --------------------------- Variable ------------------------------------------------------

Variable::Variable(const std::string &name, Float *value)
    :	name_	(name),
        value_	(value),
        owner_	(false),
        const_	(false),
        temp_	(false),
        prog_local_	(0)
{
    MF_DEBUG_("Variable("<<this<<")::Variable(\""<<name<<"\", "<<value<<")");
}

Variable::Variable(const std::string &name, Float value)
    :	name_	(name),
        value_	(new Float),
        owner_	(true),
        const_	(true),
        temp_	(false),
        prog_local_	(0)
{
    MF_DEBUG_("Variable("<<this<<")::Variable(\""<<name<<"\", "<<value<<")");

    *value_ = value;
}

Variable::Variable(const Variable& var)
    :	value_	(0),
        owner_	(false),
        const_	(false),
        temp_	(false),
        prog_local_	(0)
{
    MF_DEBUG_("Variable("<<this<<")::Variable("<<&var<<")");

    *this = var;
}

Variable& Variable::operator=(const Variable& var)
{
    MF_DEBUG_("Variable("<<this<<")::operator=("<<&var<<")");

    if (owner_ && value_) delete value_;

    name_ = var.name_;
    value_ = var.value_;
    owner_ = var.owner_;
    const_ = var.const_;
    temp_ = var.temp_;
    prog_local_ = var.prog_local_;

    if (owner_ && value_)
    {
        value_ = new Float;
        *value_ = *var.value_;
    }

    return *this;
}

Variable::~Variable()
{
    MF_DEBUG_("Variable("<<this<<")::~Variable()");

    if (owner_ && value_) delete value_;
}

long long int Variable::valueAsInt() const
{
#ifdef PPP_TTMATH
    ttmath::sint res;
    value_->ToInt(res);
    return res;
#elif defined(PPP_GMP)
    return value_->get_si();
#else
    return (long long int)*value_;
#endif
}

// -------------------------- variables --------------------------------

Variables::Variables()
{
    add("PI", PI);
    add("TWO_PI", TWO_PI);
    add("TAU", TWO_PI);
    add("HALF_PI", HALF_PI);
    add("E", 2.71828182845904523536);
    add("PHI", (1.0 + sqrt(5.0)) / 2.0);
}

Variable * Variables::add_temp_(const std::string& name, Float value, Program * prog)
{
    Variable * v;
    if ((v = variable(name))) return v;

    map_.insert( Map::value_type( name, v = new Variable(name, value) ) );
    v->temp_ = true;
    v->prog_local_ = prog;

    return v;
}

void Variables::clear_temps_()
{
    for (Map::iterator i=map_.begin();
            i!=map_.end(); ++i)
    if (i->second->temp_)
    {
        delete i->second;
        map_.erase(i);
        i = map_.begin();
    }
}

void Variables::clear()
{
    for (Map::iterator i=map_.begin();
            i!=map_.end(); ++i)
        delete i->second;
    map_.clear();
}

bool Variables::getVariables(std::vector<Variable*>& vec, bool temp)
{
    bool r = false;
    for (Map::iterator i=map_.begin(); i!=map_.end(); ++i)
    {
        if (i->second->temp_ == temp)
        {
            vec.push_back( i->second );
            r = true;
        }
    }
    return r;
}

bool Variables::add(const std::string& name, Float value)
{
    if (variable(name)) return false;

    //std::cout << v.name() << " " << v.value() << "\n";
    map_.insert( Map::value_type( name, new Variable(name, value) ) );
    return true;
}

bool Variables::add(const std::string& name, Float* value)
{
    if (variable(name)) return false;

    map_.insert( std::make_pair( name, new Variable(name, value) ) );
    return true;
}

Variable * Variables::variable(const std::string& name)
{
    Map::iterator i = map_.find(name);

    return (i == map_.end())? 0 : (i->second);
}

const Variable * Variables::variable(const std::string& name) const
{
    Map::const_iterator i = map_.find(name);

    return (i == map_.end())? 0 : (i->second);
}

Variable * Variables::variable_(const std::string& name, Program * prog)
{
    Map::iterator i = map_.find(name);

    return (i == map_.end() || i->second->prog_local_ != prog)? 0 : (i->second);
}

std::vector<std::string> Variables::variableNames() const
{
    std::vector<std::string> vec;
    for (auto &i : map_)
        vec.push_back(i.second->name());

    return vec;
}

void Variables::copyFrom(const Variables &other)
{
    clear();

    for (Map::const_iterator i = other.map_.begin(); i != other.map_.end(); ++i)
    if (!i->second->temp_)
    {
        if (i->second->owner_)
            add(i->second->name_, i->second->value_);
        else
            add(i->second->name_, *i->second->value_);
    }
}


// -------------------------- Functions ----------------------------

void Functions::copyFrom(const Functions &other)
{
    clear();
    for (Map::const_iterator f = other.map_.begin(); f != other.map_.end(); ++f)
    {
        if (f->second->type() == Function::LAMBDA)
            add(f->second->num_param(),
                       f->second->name(), f->second->lambda_func_);
        else
            add(f->second->type(), f->second->num_param(),
                   f->second->name(), f->second->func_);
    }
}

void Functions::print(std::ostream& out) const
{
    for (Map::const_iterator i=map_.begin(); i!=map_.end(); ++i)
    {
        out << i->second->name() << "(" << i->second->num_param() << ")";
        if (i->second->temp_) out << " TEMP";
        out << "\n";
    }
}

Function * Functions::add(Function::Type type, int num_param, const std::string& name, FuncPtr func_ptr)
{
    if (function(name, num_param))
    {
        std::cerr << "Functions: try to add duplicate function '" << name << "' (" << num_param << ")\n";
        return 0;
    }
    Function * f;
    map_.insert( std::make_pair( name, f = new Function(name, func_ptr, num_param, type) ) );
    return f;
}

Function * Functions::add(int num_param, const std::string& name, LambdaFuncPtr lambda_func)
//				FuncPtr lambda_func, FuncPtr exec_func, FuncPtr combine_func)
{
    if (function(name, num_param))
    {
        std::cerr << "Functions: (lambda) try to add duplicate function '" << name << "' (" << num_param << ")\n";
        return 0;
    }

    Function * f;
    map_.insert( std::make_pair( name,
                        f = new Function(
                                    name,
                                    lambda_func,
                                    num_param) ) );
    return f;
}

Function * Functions::function(const std::string& name, int num_params)
{
    iterator_pair range = map_.equal_range(name);
    // name not found
    if (range.first == range.second) return 0;
    // check for parameters
    if (num_params < 0) return range.first->second;
    for (; range.first != range.second; ++range.first)
    {
        if (range.first->second->num_param() == num_params) return range.first->second;
    }
    // parameters don't match
    return 0;
}

const Function * Functions::function(const std::string& name, int num_params) const
{
    const_iterator_pair range = map_.equal_range(name);
    // name not found
    if (range.first == range.second) return 0;
    // check for parameters
    if (num_params < 0) return range.first->second;
    for (; range.first != range.second; ++range.first)
    {
        if (range.first->second->num_param() == num_params) return range.first->second;
    }
    // parameters don't match
    return 0;
}

Function * Functions::function(const std::string& name, Function::Type type, int num_params)
{
    iterator_pair range = map_.equal_range(name);
    // name not found
    if (range.first == range.second) return 0;
    // check for parameters
    if (range.first->second->type() == type && num_params < 0) return range.first->second;
    for (; range.first != range.second; ++range.first)
    {
        if (range.first->second->type() == type
            && range.first->second->num_param() == num_params) return range.first->second;
    }
    // parameters don't match
    return 0;
}

const Function * Functions::function(const std::string& name, Function::Type type, int num_params) const
{
    const_iterator_pair range = map_.equal_range(name);
    // name not found
    if (range.first == range.second) return 0;
    // check for parameters
    if (range.first->second->type() == type && num_params < 0) return range.first->second;
    for (; range.first != range.second; ++range.first)
    {
        if (range.first->second->type() == type
            && range.first->second->num_param() == num_params) return range.first->second;
    }
    // parameters don't match
    return 0;
}



bool Functions::match_params(const std::string& name, int num_params) const
{
    return function(name, num_params);
}

void Functions::clear()
{
    for (Map::iterator i=map_.begin(); i!=map_.end(); ++i)
        delete i->second;

    map_.clear();
}

void Functions::clear_temps_()
{
    for (Map::iterator i=map_.begin();
            i!=map_.end(); ++i)

    if (i->second->temp_)
    {
        delete i->second;
        map_.erase(i);
        i = map_.begin();
        if (i==map_.end()) return;
    }
}

std::vector<std::string> Functions::functionNames() const
{
    std::set<std::string> set;
    for (auto &i : map_)
        set.insert(i.second->name());

    std::vector<std::string> vec;
    for (auto &i : set)
        vec.push_back(i);

    return vec;
}





// ------------------ Parser ---------------------------------------


struct Parser::Detail
{
    /* YYPARSE_PARAM */
    ParseParam param;
};


Parser::Parser()
    :	d_	(new Detail),
        ok_	(false)
{
    create_functions(funcs_);
    d_->param.prog = new Program(&funcs_, &var_);
}


Parser::~Parser()
{
    delete d_->param.prog;
    delete d_;
}


// -------------- variables ----------------



bool Parser::parse(const std::string& str)
{
    if (str.empty()) return false;
    std::string str_(str);
    //if (str[str.size()-1] != ';') str_ += ';';

    d_->param.prog->clear();
    var_.clear_temps_();

    d_->param.inp = str_.c_str();
    d_->param.inp_pos = 0;
    d_->param.vars = &var_;
    d_->param.funcs = &funcs_;

    d_->param.prog->clear();

    ok_ = (yyparse( (void*)&d_->param ) == 0);

    if (d_->param.prog->root() != d_->param.prog)
    {
        PPP_PROG_DEBUG( "parser did not return to root program!" );
        d_->param.prog = d_->param.prog->root();
    }

    if (!ok_)
    {
        PPP_PROG_DEBUG( "parsing failed..." );
        /* IMPortant to clear the gathered expressions */
        d_->param.prog->clear();
        return false;
    }
    else
        PPP_PROG_DEBUG("parsing finished\n");

    PPP_PROG_DEBUG( "--- after parsing:");
    PPP_PROG_DEBUG( d_->param.prog->string() )
    ok_ = d_->param.prog->compile();
    if (!ok_) { PPP_PROG_DEBUG( "compilation failed." ); }
        else { PPP_PROG_DEBUG("compiled."); }

    PPP_PROG_DEBUG( "--- after compilation:\n" << d_->param.prog->string() )
    PPP_PROG_DEBUG( d_->param.prog->num_visible_expr() << " functions" );

    return ok_;
}

Float Parser::eval()
{
    if (!ok_) return 0.0;

    if (d_->param.prog->parent) { std::cout << "wtf?\n"; return 0.0; }

    d_->param.prog->clearCallCount();

    MO_EXTEND_EXCEPTION(
        return d_->param.prog->eval() ,
        "failed in Parser, equation text = '" << d_->param.inp << "'"
    );

}


std::string Parser::syntax() const
{
    std::stringstream s;
    d_->param.prog->printSyntax(s, 0);
    return s.str();
}

std::string Parser::dot_graph() const
{
    std::stringstream s;
    d_->param.prog->printDot(s);
    return s.str();
}

bool Parser::save_dot_graph(const std::string& filename) const
{
    std::ofstream f;
    f.open(filename);
//	if (!f.ok()) return false;

    d_->param.prog->printDot(f);

    f.close();
    return true;
}





// ---------------------------- test --------------------------

int test_parser_()
{
    using namespace std;

    int errors = 0, tests = 0;
    Parser p;

    Float x = 23.0, y = 42.0;
    double x_ = 23.0, y_ = 42.0;

    p.variables().add("x", &x);
    p.variables().add("y", &y);
    p.variables().add("PI", PI);

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
            if (Abs(rc-rp) > Abs(rc)/1000000.0) \
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

    PPP_COMPARE(   1+2-3+4-5+6-7+8 )
    PPP_COMPARE(   1+2*3 )
    PPP_COMPARE(   (1+2)*3 )
    PPP_COMPARE(   1.0/2+3*4 )

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
    PPP_COMPARE_2( -x*x,         foo(x) { bar(x) { x*x }; -bar(x) }; foo(x) )
    PPP_COMPARE_2( 1.5,          foo(a) { prime(a)? a/2 : (a>1? foo(a-1) : 1) }; foo(4)	)
    PPP_COMPARE_2( 1,            Q(x) { v=quer(x); v<10? v : Q(v) }; Q(19) )
    PPP_COMPARE_2( 17,           next_prime(a) { useless=1; prime(a)? a : next_prime(a+1) }; next_prime(14) )
    PPP_COMPARE_2( 2*3*4*5*6*7,  faculty(a) { a>1? a*faculty(a-1) : 1 }; faculty(7) )

    if (errors)
        std::cout << errors << " of " << tests << " failed\n";
    else
        std::cout << "no errors in " << tests << " tests :D\n";

//	PPP_COMPARE( x - exp(-pow(x*(50-x), x/(1+x))/(x*(3+(x*x)))) )

    /*
        end = 10; x=x-1; k = 4^4^4^4; digcount(k,x)

        foo(c) { -c }; bar(d) { d*d }; foo(bar(x))
        foo(c) { c*c }; bar(c) { c^c }; foo(x) + bar(y)
        foo(a) { a^a }; foo(a,b) { a*b }; foo(x) + foo(x,x)

        foo(x) { bar(x) { x*x }; -bar(x) }; foo(x)

        faculty(a) { a>1? a*faculty(a-1) : 1 }; faculty(x) == fac(x)

        foo(a) { prime(a)? a/2 : foo(a*1.5) }; foo(x)

        next_prime(a) { prime(a)? a : next_prime(a+1) }; next_prime(10)

        // WTF?
        foo(a) { prime(a)? a/2 : (a>1? foo(a-1) : 1) }; foo(3)							OK
        foo(a) { prime(a)? x : (a==1? y : z) }; foo(4)									OK
        Q(x) { v=quer(x); v<10? v : Q(v) }; Q(19) 										OK
        c=0; next_prime(a) { c=c+1; prime(a)? a : next_prime(a+1) }; next_prime(10)		OK

        end=3; x>2? x+30 : x>1? x+20 : x+10

        // optimization
        a=x; b=a; c=b

        // harmonic series
        f(a) { 1/fac(a) }; series(f,+,1,100)

        // how to count 3x+1 iterations?
        collatz(a) { a==1? 1 : (a%2==0? collatz(a/2) : collatz(a*3+1)) }; collatz(n)


        // nice tables
        end=29; M=2^x-1; dM=ndiv(M); px=prime(x)*10; prime(M)*10;

        // -- 2d --
        // x,y, c = count (from top-left to bottom-right)
        // if ndiv(c)==3 then ndiv(c^2)==5, ndiv(c^3)==7, ndiv(c^4)==9, ...
        n=ndiv(c); n2=ndiv(c^2); (n==3) + (n2==5)
        // ndiv(c)==5, no c^2, ndiv(c^3)==13, ndiv(c^4)==17

        ndiv(x) == ndiv(x*y+1)
        ndiv(x+y)/2 == sqrt(ndiv(x*y))

        prime(x) && prime(fib(y))
        // ^ when prime(x) then prime(fib(x))

        // sine curve
        floor(sin(x/10)*11+13) == y

        // Ulam Prime Spiral
        n=uspiral(x-40,y-20); prime(n)

        -color 8 -plot 41 20 'u = n^2-n+41; prime(u)'

        -plot 80 40 'n=uspiral(x-40,y-20); prime(ndiv(n)) + 2*prime(n)'

        //
        ./func_parser -plot 150 66 -sym '.*'   'N=7; X=floor(x/N); Y=floor(y/N); cong(y^X+y,x^Y-x, N)'
    */

    #undef PPP_COMPARE

    return errors;
}






} // namespace PPP_NAMESPACE
