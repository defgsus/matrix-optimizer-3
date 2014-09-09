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

Variable::Variable(const std::string &name, Float *value, const std::string & desc)
    :	name_	(name),
        desc_   (desc),
        value_	(value),
        owner_	(false),
        const_	(false),
        temp_	(false),
        prog_local_	(0)
{
    MF_DEBUG_("Variable("<<this<<")::Variable(\""<<name<<"\", "<<value<<")");
}

Variable::Variable(const std::string &name, Float value, const std::string& desc)
    :	name_	(name),
        desc_   (desc),
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
    desc_ = var.desc_;
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
    add("PI", PI, QObject::tr("the famous pi constant").toStdString());
    add("TWO_PI", TWO_PI, QObject::tr("two times pi").toStdString());
    add("TAU", TWO_PI, QObject::tr("same as TWO_PI").toStdString());
    add("HALF_PI", HALF_PI, QObject::tr("pi divided by 2").toStdString());
    add("E", 2.71828182845904523536, QObject::tr("\"Euler's constant\"").toStdString());
    add("PHI", (1.0 + sqrt(5.0)) / 2.0, QObject::tr("the \"golden ratio\"").toStdString());
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

bool Variables::add(const std::string& name, Float value, const std::string & desc)
{
    if (variable(name)) return false;

    //std::cout << v.name() << " " << v.value() << "\n";
    map_.insert( Map::value_type( name, new Variable(name, value, desc) ) );
    return true;
}

bool Variables::add(const std::string& name, Float* value, const std::string & desc)
{
    if (variable(name)) return false;

    map_.insert( std::make_pair( name, new Variable(name, value, desc) ) );
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

std::vector<std::string> Variables::variableDescriptions() const
{
    std::vector<std::string> vec;
    for (auto &i : map_)
        vec.push_back(i.second->description());

    return vec;
}

void Variables::copyFrom(const Variables &other)
{
    clear();

    for (Map::const_iterator i = other.map_.begin(); i != other.map_.end(); ++i)
    if (!i->second->temp_)
    {
        if (i->second->owner_)
            add(i->second->name_, i->second->value_, i->second->desc_);
        else
            add(i->second->name_, *i->second->value_, i->second->desc_);
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
    map_.insert( std::make_pair( name, f = new Function(name, curGroup_, func_ptr, num_param, type) ) );
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
                                    curGroup_,
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

std::vector<const Function*> Functions::getFunctions() const
{
    std::vector<const Function*> f;
    for (auto &i : map_)
        f.push_back(i.second);
    return f;
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

    //std::cout << d_->param.prog->string() << std::endl;
    //d_->param.prog->printSyntax(std::cout); std::cout << std::endl;
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






} // namespace PPP_NAMESPACE
