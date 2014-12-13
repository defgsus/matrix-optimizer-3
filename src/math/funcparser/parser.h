/**	@file

	@brief Math FUnction Parser - public interface

	@author def.gsus-
	@version 2013/09/29 started
*/
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "parser_defines.h"

namespace PPP_NAMESPACE {

    class Program;

    /** basic variable type */
    class Variable
    {
        friend class Program;
        friend class Variables;

        public:

        const std::string& name() const { return name_; }
        const std::string& description() const { return desc_; }

        Float value() const { return *value_; }

        long long int valueAsInt() const;

        Float * value_ptr() { return value_; }

        bool isConst() const { return const_ && !temp_; }
        bool isTemp() const { return temp_; }

        /* the following stuff is for handyness and to
            manage ownership and stuff */

        /** constructor for non-owning float */
        Variable(const std::string &name_, Float *value_, const std::string& desc = "");

        /** constructor for own allocated float */
        Variable(const std::string &name_, Float value_, const std::string& desc = "");

        /** copy constructor.
            if 'var' owns it's float, this Variable will own it's own float */
        Variable(const Variable& var);

        /** copies settings.
            if 'var' owns it's float, this Variable will own it's own float */
        Variable& operator=(const Variable& var);

        /** destructor to release the float when owned */
        ~Variable();

    private:

        /** name, f.e. "x" */
        std::string name_,
        /** Description */
            desc_;

        /** pointer to the float value */
        Float *value_;

        /** tells if Variable is responsible for the pointed-to float */
        bool owner_,
        /** is this variable constant (can it be substituted by a float).
            by default owner are const, except temporaries */
            const_,
        /** this is a temporary? the Program will create and delete them as needed. */
            temp_;
        /** temporary is belonging to this program when not 0 */
        Program * prog_local_;
    };



    class Variables
    {
        friend class Parser;
        friend class Program;

        typedef std::map<std::string, Variable*> Map;
        Map map_;

        /** creates and returns a temp, or returns the variable that exists.
            if prog != 0, the variable is constructed for prog only. */
        Variable * add_temp_(const std::string& name, Float value, Program * prog = 0);
        /** return the local variable of program, or NULL if there is no local. */
        Variable * variable_(const std::string& name, Program * prog);

        void clear_temps_();

        public:

        /** Creates some default variables */
        Variables();

        /** Ignores temporaries in other! */
        Variables(const Variables& other) { copyFrom(other); }

        ~Variables() { clear(); }

        /** Ignores temporaries in other! */
        Variables& operator = (const Variables& other) { copyFrom(other); return *this; }

        /** add a non-owning variable */
        bool add(const std::string& name, Float * value, const std::string& desc);

        /** add an owning variable */
        bool add(const std::string& name, Float value, const std::string& desc);

        /** return the installed variable or NULL */
        Variable * variable(const std::string& name);

        /** return the installed variable or NULL */
        const Variable * variable(const std::string& name) const;

        void clear();

        /** Ignores temporaries in other! */
        void copyFrom(const Variables& other);

        /** add all variables that match the paramaters to 'vec'. */
        bool getVariables(std::vector<Variable*>& vec, bool temp);

        std::vector<std::string> variableNames() const;
        std::vector<std::string> variableDescriptions() const;
    };




    class Function
    {
        friend class Functions;
        friend class Expression;
        friend class Program;

        public:

        enum Type
        {
            FUNCTION,
            UNARY_LEFT_OP,
            UNARY_RIGHT_OP,
            BINARY_OP,
            LAMBDA
        };

        Function(const std::string& name, const std::string& groupname,
                 FuncPtr func_ptr, int nparam, Type type)
            : name_(name), groupName_(groupname), nparam_(nparam),
              func_(func_ptr), lambda_func_(0), type_(type), temp_(false)

        { }

        Function(const std::string& name, const std::string& groupname,
                 LambdaFuncPtr lambda_func, int nparam)
            :	name_(name), groupName_(groupname), nparam_(nparam), func_(0),
                lambda_func_(lambda_func), type_(LAMBDA), temp_(false)
        { }

        const std::string& name() const { return name_; }
        const std::string& groupName() const { return groupName_; }

        int num_param() const { return nparam_; }

        FuncPtr func() const { return func_; }

        Type type() const { return type_; }

        bool isTemp() const { return temp_; }

        bool is_lambda() const { return lambda_func_ != 0; }

        private:

        std::string name_, groupName_;
        int nparam_;
        FuncPtr func_;
        LambdaFuncPtr lambda_func_;
        Type type_;
        bool temp_;
    };



    class Functions
    {
        friend class Program;

        typedef std::multimap<std::string, Function*> Map;
        typedef std::pair<Map::iterator, Map::iterator> iterator_pair;
        typedef std::pair<Map::const_iterator, Map::const_iterator> const_iterator_pair;
        Map map_;

        public:

        Functions() { }
        Functions(const Functions& other) { copyFrom(other); }

        ~Functions() { clear(); }

        Functions& operator = (const Functions& other) { copyFrom(other); return *this; }

        void setCurrentGroup(const std::string& groupName) { curGroup_ = groupName; }

        /** add a function */
        Function * add(Function::Type type, int num_param, const std::string& name, FuncPtr func_ptr);

        /** add a lambda function in the form of: <br/>
            lambda_func( exec_func, combine_func, params**) */
        Function * add(int num_param, const std::string& name, LambdaFuncPtr lambda_func);
        //				FuncPtr lambda_func, FuncPtr exec_func, FuncPtr combine_func);

        /** return the installed function or NULL.
            num_params may be negative which returns the first name match
            regardless of number of parameters. */
        Function * function(const std::string& name, int num_params);

        /** return the installed function or NULL.
            num_params may be negative which returns the first name match
            regardless of number of parameters. */
        const Function * function(const std::string& name, int num_params) const;

        /** return the installed function of specific type or NULL.
            num_params may be negative which returns the first name match
            regardless of number of parameters. */
        Function * function(const std::string& name, Function::Type type, int num_params);

        /** return the installed function of specific type or NULL.
            num_params may be negative which returns the first name match
            regardless of number of parameters. */
        const Function * function(const std::string& name, Function::Type type, int num_params) const;

        /** check if there exists a function that matches the given number of parameters. */
        bool match_params(const std::string& name, int num_params) const;

        void clear();

        void copyFrom(const Functions& other);

        // --------------- info ----------------

        void print(std::ostream& out = std::cout) const;
        std::string string() const { std::stringstream s; print(s); return s.str(); }

        /** Returns all functions */
        std::vector<const Function*> getFunctions() const;

        /** Returns all (unique) function names to the vector of strings */
        std::vector<std::string> functionNames() const;

        private:

        void clear_temps_();

        std::string curGroup_;
    };



    class Parser
    {
        struct Detail;

        // disable copy
        Parser(const Parser& );
        Parser& operator = (const Parser&);

        public:

        // ---------- ctor -------------

        Parser();
        ~Parser();


        // -------- variables -------------

        Variables & variables() { return var_; }

        const Variables & variables() const { return var_; }

        // --------- functions ------------

        Functions & functions() { return funcs_; }

        const Functions & functions() const { return funcs_; }

        // ---------- run --------------

        /** returns true when ready to eval() */
        bool ok() const { return ok_; }

        bool parse(const std::string& str);

        /** (re-)calculate and return result */
        Float eval();

        // -------- info ---------------

        /** internal representation back to human readable */
        std::string syntax() const;

        std::string dot_graph() const;

        bool save_dot_graph(const std::string& filename) const;

        // __________ OUT ______________
        private:

        Detail * d_;

        bool ok_;

        Variables var_;

        Functions funcs_;
    };

} // namespace PPP_NAMESPACE


#endif // PARSER_H_INCLUDED
