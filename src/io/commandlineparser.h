/** @file commandlineparser.h

    @brief Command line parameters parser

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#ifndef MOSRC_IO_COMMANDLINEPARSER_H
#define MOSRC_IO_COMMANDLINEPARSER_H

#include <QString>
#include <QVariant>
#include <QList>
#include <QStringList>

namespace MO {
namespace IO {


class CommandLineParameter
{
    friend class CommandLineParser;
public:

    /** Types of parameters.
        To extend this list, also extend
        typeName(),
        CommandLineParser::parse()
        and CommandLineParameterTraits */
    enum Type
    {
        T_NONE,
        T_INT,
        T_UINT,
        T_DOUBLE,
        T_STRING
    };

    /** Creates a valueless parameter.
        @p name can be a comma separated list of aliases. */
    explicit CommandLineParameter(const QString& id, const QString& name, const QString& description);

    /** Creates a valued parameter.
        @p name can be a comma separated list of aliases. */
    template <typename T>
    explicit CommandLineParameter(const QString& id, const QString& name, const QString& description,
                                  T default_value);

    /** Creates a parameter with multiple values.
        @p name can be a comma separated list of aliases. */
    template <typename T>
    explicit CommandLineParameter(const QString& id, const QString& name, const QString& description,
                                  const QList<T> & default_value);

    // ----- getter -------

    /** Returns the type of the parameter */
    Type type() const { return t_; }
    /** Returns a nice name of the type of the parameter */
    const QString& typeName() const;
    /** Returns the number of values (>1 for multi-value-parameters) */
    int numValues() const { return v_.size(); }

    /** Returns the identifier of the parameter */
    const QString& id() const { return id_; }
    /** Returns the list of all alias names of the parameter */
    const QStringList& names() const { return names_; }
    /** Returns the description/help text defined for this parameter */
    const QString& description() const { return desc_; }

    /** Returns true when @p name is in the list of aliases */
    bool hasName(const QString& name) const { return names_.contains(name); }

    /** Returns true when the parameter was parsed by CommandLineParser::parse() */
    bool isPresent() const { return pres_; }
    /** Returns the parsed value */
    const QVariant & value(uint index = 0) const { return v_[index]; }
    /** Returns the default value set for this parameter */
    const QVariant & defaultValue(uint index = 0) const { return d_[index]; }

private:

    Type t_;
    QString id_, desc_;
    QStringList names_;
    QList<QVariant> v_, d_;
    bool pres_;
};



class CommandLineParser
{
public:

    enum Option
    {
        /** If set, unknown parameters will be skipped,
            otherwise parsing fails. */
        O_SKIP_UNKNOWN_PARAMETERS = 1,
        /** If set, illegal values will be skipped,
            otherwise parsing fails. */
        O_SKIP_ILLEGAL_VALUES = 1<<1
    };


    CommandLineParser();
    ~CommandLineParser();

    // ----- getter -------

    /** Returns the currently set options */
    int options() const { return opt_; }

    /** Returns installed parameters */
    const QList<CommandLineParameter*> parameters() const { return params_; }

    /** Returns the error text when parse() failed */
    const QString& error() const { return error_; }

    /** Returns the whole parameter description */
    QString helpString(int max_width = 70) const;

    /** Returns the whole parameter description */
    QString helpStringHtml() const;

    /** Returns a list of all value parameters and their values */
    QString valueString(bool use_id_as_name = true) const;

    /** Returns the parameter with the given id, or NULL */
    CommandLineParameter * parameter(const QString& id) const;

    /** Returns true when the parameter matching @p id was parsed */
    bool contains(const QString& id) const;

    /** Returns the value of the parameter matching @p id,
        or an empty QVariant if not found. */
    QVariant value(const QString& id) const;

    /** Returns the arguments that where given on the commandline.
        An 'argument' is everything that is no parameter (does not start with -)
        and is no value to a previous parameter. */
    const QStringList& arguments() const { return arguments_; }

    // ----- setter -------

    /** Sets the parsing options as or-combination of the Option enum. */
    void setOptions(int options) { opt_ = options; }
    /** Enables or disables a specific option */
    void setOption(Option o, bool enable);

    /** Removes and destroys the given parameter, if found */
    void removeParameter(const QString& id);

    /** Adds a new parameter.
        Ownership is taken.
        If the CommandLineParameter::id() is already present,
        @p p is not added and false is returned. */
    bool addParameter(CommandLineParameter * p);

    /** Creates a valueless parameter and returns it.
        If the CommandLineParameter::id() is already present,
        no parameter is created and NULL returned. */
    CommandLineParameter * addParameter(
            const QString& id, const QString& name, const QString& description);

    /** Creates a parameter and returns it.
        If the CommandLineParameter::id() is already present,
        no parameter is created and NULL returned. */
    template <typename T>
    CommandLineParameter * addParameter(
            const QString& id, const QString& name, const QString& description,
            T default_value);

    /** Creates a multi-value parameter and returns it.
        If the CommandLineParameter::id() is already present,
        no parameter is created and NULL returned. */
    template <typename T>
    CommandLineParameter * addParameter(
            const QString& id, const QString& name, const QString& description,
            const QList<T>& default_values);

    // ------- io ---------

    /** Classic Qt tr() function */
    static QString tr(const char *);

    /** Parse the classic commandline parameter types.
        @p skip defines the number of parameters to skip at the beginning,
        which is usually 1. */
    bool parse(int argc, char * argv[], int skip = 0);

    /** Parse the list of strings as consecutive parameters */
    bool parse(const QStringList&);

private:

    int opt_;

    QList<CommandLineParameter*> params_;

    QStringList arguments_;

    QString error_;
};



// _______________________ TEMPLATE IMPL. _________________________

template <typename T>
struct CommandLineParameterTraits
{
    static const CommandLineParameter::Type type
        = T::_xxx_not_defined_as_commandline_parameter;
};

template <>
struct CommandLineParameterTraits<int>
{
    static const CommandLineParameter::Type type = CommandLineParameter::T_INT;
};

template <>
struct CommandLineParameterTraits<unsigned int>
{
    static const CommandLineParameter::Type type = CommandLineParameter::T_UINT;
};

template <>
struct CommandLineParameterTraits<double>
{
    static const CommandLineParameter::Type type = CommandLineParameter::T_DOUBLE;
};

template <>
struct CommandLineParameterTraits<QString>
{
    static const CommandLineParameter::Type type = CommandLineParameter::T_STRING;
};

template <>
struct CommandLineParameterTraits<const char*>
{
    static const CommandLineParameter::Type type = CommandLineParameter::T_STRING;
};


template <typename T>
CommandLineParameter::CommandLineParameter(
        const QString &id, const QString &name, const QString &description,
        T default_value)
    : CommandLineParameter(id, name, description)
{
    t_ = CommandLineParameterTraits<T>::type;
    v_.append(default_value);
    d_.append(default_value);
}

template <typename T>
CommandLineParameter::CommandLineParameter(
        const QString &id, const QString &name, const QString &description,
        const QList<T>& default_values)
    : CommandLineParameter(id, name, description)
{
    t_ = CommandLineParameterTraits<T>::type;
    for (auto & v : default_values)
    {
        v_.append(v);
        d_.append(v);
    }
}


template <typename T>
CommandLineParameter * CommandLineParser::addParameter(
        const QString &id, const QString &name, const QString &description,
        T default_value)
{
    auto p = new CommandLineParameter(id, name, description, default_value);
    if (!addParameter(p))
    {
        delete p;
        return 0;
    }
    return p;
}

template <typename T>
CommandLineParameter * CommandLineParser::addParameter(
        const QString &id, const QString &name, const QString &description,
        const QList<T>& default_values)
{
    auto p = new CommandLineParameter(id, name, description, default_values);
    if (!addParameter(p))
    {
        delete p;
        return 0;
    }
    return p;
}

} // namespace IO
} // namespace MO

#endif // COMMANDLINEPARSER_H
