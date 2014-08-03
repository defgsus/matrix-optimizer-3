#include <set>

#include "parser.h"
#include "math/constants.h"

#include "grammar.parser.cc"

#if 0
#	define MF_DEBUG_(stream_arg__) std::cout << stream_arg__ << "\n";
#else
#	define MF_DEBUG_(stream_arg__)
#endif

namespace PPP_NAMESPACE
{


// --------------------------- Variable ------------------------------------------------------

/*Variable::Variable()
	: 	value(0),
		owner(0)
{
	MF_DEBUG_("Variable("<<this<<")::Variable()");
}*/

Variable::Variable(const std::string &name, Float *value)
	:	name_	(name),
		value_	(value),
		owner_	(false),
		const_	(false)
{
	MF_DEBUG_("Variable("<<this<<")::Variable(\""<<name<<"\", "<<value<<")");
}

Variable::Variable(const std::string &name, Float value)
	:	name_	(name),
		value_	(new Float),
		owner_	(true),
		const_	(true)
{
	MF_DEBUG_("Variable("<<this<<")::Variable(\""<<name<<"\", "<<value<<")");

	*value_ = value;
}

Variable::Variable(const Variable& var)
	:	value_	(0),
		owner_	(false),
		const_	(false)
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


Variables::Variables()
{
    add("PI", PI);
    add("TWO_PI", TWO_PI);
    add("TAU", TWO_PI);
    add("HALF_PI", HALF_PI);
    add("E", 2.71828182845904523536);
    add("PHI", (1.0 + sqrt(5.0)) / 2.0);
}

bool Variables::add(const std::string& name, Float value)
{
	if (variable(name)) return false;

	//std::cout << v.name() << " " << v.value() << "\n";
	map_.insert( Map::value_type( name, Variable(name, value) ) );
	return true;
}

bool Variables::add(const std::string& name, Float* value)
{
	if (variable(name)) return false;

	map_.insert( std::make_pair( name, Variable(name, value) ) );
	return true;
}

Variable * Variables::variable(const std::string& name)
{
	Map::iterator i = map_.find(name);

	return (i == map_.end())? 0 : &(i->second);
}

const Variable * Variables::variable(const std::string& name) const
{
	Map::const_iterator i = map_.find(name);

	return (i == map_.end())? 0 : &(i->second);
}

std::vector<std::string> Variables::variableNames() const
{
    std::vector<std::string> vec;
    for (auto &i : map_)
        vec.push_back(i.second.name());

    return vec;
}



// -------------------------- Functions ----------------------------

std::vector<std::string> Functions::functionNames() const
{
    std::set<std::string> set;
    for (auto &i : map_)
        set.insert(i.second.name());

    std::vector<std::string> vec;
    for (auto &i : set)
        vec.push_back(i);

    return vec;
}

bool Functions::add(Function::Type type, int num_param, const std::string& name, FuncPtr func_ptr)
{
	if (function(name, num_param))
	{
		std::cerr << "Functions: try to add duplicate function '" << name << "' (" << num_param << ")\n";
		return false;
	}
	map_.insert( std::make_pair( name, Function(name, func_ptr, num_param, type) ) );
	return true;
}

Function * Functions::function(const std::string& name, int num_params)
{
	iterator_pair range = map_.equal_range(name);
	// name not found
	if (range.first == range.second) return 0;
	// check for parameters
	if (num_params < 0) return &range.first->second;
	for (; range.first != range.second; ++range.first)
	{
		if (range.first->second.num_param() == num_params) return &range.first->second;
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
	if (num_params < 0) return &range.first->second;
	for (; range.first != range.second; ++range.first)
	{
		if (range.first->second.num_param() == num_params) return &range.first->second;
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
	if (range.first->second.type() == type && num_params < 0) return &range.first->second;
	for (; range.first != range.second; ++range.first)
	{
		if (range.first->second.type() == type
			&& range.first->second.num_param() == num_params) return &range.first->second;
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
	if (range.first->second.type() == type && num_params < 0) return &range.first->second;
	for (; range.first != range.second; ++range.first)
	{
		if (range.first->second.type() == type
			&& range.first->second.num_param() == num_params) return &range.first->second;
	}
	// parameters don't match
	return 0;
}


bool Functions::match_params(const std::string& name, int num_params) const
{
	return function(name, num_params);
}









// ------------------ Parser ---------------------------------------


struct Parser::Detail
{
    std::string str;
	/* YYPARSE_PARAM */
	ParseParam param;
};


Parser::Parser()
	:	d_	(new Detail),
		ok_	(false)
{
	create_functions(funcs_);
	d_->param.prog = new Program(&funcs_);
}


Parser::~Parser()
{
	delete d_->param.prog;
	delete d_;
}



// -------------- variables ----------------





bool Parser::parse(const std::string& str)
{
    d_->str = str;
	d_->param.prog->clear();

    d_->param.inp = d_->str.c_str();
	d_->param.inp_pos = 0;
	d_->param.vars = &var_;
	d_->param.funcs = &funcs_;

	ok_ = (yyparse( (void*)&d_->param ) == 0);
	if (!ok_) { std::cerr << "parsing failed.\n"; return false; }

	ok_ = d_->param.prog->compile();
	if (!ok_) { std::cerr << "compilation failed.\n"; }
	return ok_;
}

Float Parser::eval()
{
	if (!ok_) return 0.0;

	return d_->param.prog->eval();
}









// ---------------------------- test --------------------------

int test_parser_()
{
	using namespace std;

	int errors = 0;
	Parser p;

	Float x = 23.0, y = 42.0;

	p.variables().add("x", &x);
	p.variables().add("y", &y);

	#define PPP_COMPARE_2(cexpression__, expression__) \
	{	\
		std::cout << #expression__ << " "; \
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
			if (fabs(rc-rp) > fabs(rc)/1000000.0) \
			{ \
				std::cout << "[result differs c++ = " << rc << " / ppp = " << rp << "]\n"; \
				++errors; \
			} \
			else \
				std::cout << "= " << rp << " OK\n"; \
		} \
	}

	#define PPP_COMPARE(expression__) PPP_COMPARE_2(expression__, expression__)

	PPP_COMPARE( 1+2-3+4-5+6-7+8 )
	PPP_COMPARE( 1+2*3 )
	PPP_COMPARE( (1+2)*3 )
	PPP_COMPARE( 1.0/2+3*4 )

	PPP_COMPARE( (((x+1)*x+2)*x+3) )

	PPP_COMPARE( x > 20 && y < 50 )
	PPP_COMPARE( 1 xor 3 xor 5 xor 8 )

	PPP_COMPARE( sin(PI) )
	PPP_COMPARE( cos(2*PI) )
	PPP_COMPARE( sin(cos(tan(0.5))) )

	PPP_COMPARE( x - exp(-pow(x*(50-x), x/(1+x))/(x*(3+(x*x)))) )

	#undef PPP_COMPARE

	return errors;
}






} // namespace PPP_NAMESPACE
