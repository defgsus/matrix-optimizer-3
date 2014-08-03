/**	@file

	@brief Math FUnction Parser - public interface

	@author def.gsus-
	@version 2013/09/29 started
*/
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

/*#ifndef PPP_TYPE
#	error need to define PPP_TYPE
#endif*/
/*
#ifndef PPP_NAMESPACE
#	define PPP_NAMESPACE PPP
#	error need to define PPP_NAMESPACE
#endif
*/
#include <string>
#include <vector>
#include <map>

#include "parser_defines.h"

namespace PPP_NAMESPACE {

	/** basic variable type */
	class Variable
	{
		public:

		const std::string& name() const { return name_; }

		Float value() const { return *value_; }

		Float * value_ptr() { return value_; }

		bool isConst() const { return const_; }

		/* the following stuff is for handyness and to
			manage ownership and stuff */

		/* default constructor */
//		Variable();

		/** constructor for non-owning float */
		Variable(const std::string &name_, Float *value_);

		/** constructor for own allocated float */
		Variable(const std::string &name_, Float value_);

		/** copy constructor.
			if 'var' own's it's float, this Variable will own it's own float */
		Variable(const Variable& var);

		/** copies settings.
			if 'var' own's it's float, this Variable will own it's own float */
		Variable& operator=(const Variable& var);

		/** destructor to release the float when owned */
		~Variable();

	private:

		/** name, f.e. "x" */
		std::string name_;

		/** pointer to the float value */
		Float *value_;

		/** tells if Variable is responsible for the pointed-to float */
		bool owner_,
		/** is this variable constant (can it be substituted by a float).
			by default owner are const */
			const_;
	};


	class Variables
	{
		typedef std::map<std::string, Variable> Map;
		Map map_;

		public:

        /** Creates some default constants */
        Variables();

		/** add a non-owning variable */
		bool add(const std::string& name, Float * value);

		/** add an owning variable */
		bool add(const std::string& name, Float value);

		/** return the installed variable or NULL */
		Variable * variable(const std::string& name);

		/** return the installed variable or NULL */
		const Variable * variable(const std::string& name) const;

		void clear() { map_.clear(); }

        std::vector<std::string> variableNames() const;
	};


	class Function
	{
		public:

		enum Type
		{
			FUNCTION,
			UNARY_LEFT_OP,
			UNARY_RIGHT_OP,
			BINARY_OP
		};

		Function(const std::string& name, FuncPtr func_ptr, int nparam, Type type)
			:	name_(name), nparam_(nparam), func_(func_ptr), type_(type)
		{ }

		const std::string& name() const { return name_; }

		int num_param() const { return nparam_; }

		FuncPtr func() const { return func_; }

		Type type() const { return type_; }

		private:

		std::string name_;
		int nparam_;
		FuncPtr func_;
		Type type_;
	};


	class Functions
	{
		typedef std::multimap<std::string, Function> Map;
		typedef std::pair<Map::iterator, Map::iterator> iterator_pair;
		typedef std::pair<Map::const_iterator, Map::const_iterator> const_iterator_pair;
		Map map_;

		public:

        void clear() { map_.clear(); }

		/** add a function */
		bool add(Function::Type type, int num_param, const std::string& name, FuncPtr func_ptr);

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

        std::vector<std::string> functionNames() const;

	};


	class Parser
	{
		struct Detail;

		public:

		typedef double Float;

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

        bool parse(const std::string &str);

		/** (re-)calculate and return result */
		Float eval();

		// __________ OUT ______________
		private:

		Detail * d_;

		bool ok_;

		Variables var_;

		Functions funcs_;
	};


	/** test Parser and return errors */
	extern int test_parser_();

} // namespace PPP_NAMESPACE

#endif // PARSER_H_INCLUDED
