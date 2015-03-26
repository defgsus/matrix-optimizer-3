/** @file commandlineparser.cpp

    @brief Command line parameters parser

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.10.2014</p>
*/

#include <QObject> // for tr()
#include <QTextStream>

#include "commandlineparser.h"
#include "io/error.h"
#include "tool/stringmanip.h"

namespace MO {
namespace IO {


CommandLineParameter::CommandLineParameter(
        const QString &id, const QString &name, const QString &description)
    : t_    (T_NONE),
      id_   (id),
      desc_ (description),
      pres_ (false)
{
    names_ = name.split(',', QString::SkipEmptyParts);
    MO_ASSERT(names_.size(), "No name given for commandline parameter '" << id_ << "'");
}



const QString& CommandLineParameter::typeName() const
{
    switch (t_)
    {
        case T_NONE: break;
        case T_INT: { static QString s=QObject::tr("signed integer"); return s; }
        case T_UINT: { static QString s=QObject::tr("unsigned integer"); return s; }
        case T_DOUBLE: { static QString s=QObject::tr("floating-point number"); return s; }
        case T_STRING: { static QString s=QObject::tr("string"); return s; }
    }

    static QString s = QObject::tr("none");
    return s;
}




// ############################ CommandLineParser ####################################

CommandLineParser::CommandLineParser()
    : opt_      (0)
{
}

CommandLineParser::~CommandLineParser()
{
    for (auto p : params_)
        delete p;
}

QString CommandLineParser::tr(const char * t)
{
    return QObject::tr(t);
}

void CommandLineParser::setOption(Option o, bool enable)
{
    // clear the option
    opt_ = (opt_ & (~o));
    // and set
    if (enable)
        opt_ |= o;
}

CommandLineParameter * CommandLineParser::parameter(const QString &id) const
{
    for (auto p : params_)
        if (p->id() == id)
            return p;
    return 0;
}

bool CommandLineParser::contains(const QString &id) const
{
    auto p = parameter(id);
    return p && p->isPresent();
}

QVariant CommandLineParser::value(const QString &id) const
{
    auto p = parameter(id);
    return p ? p->value() : QVariant();
}

void CommandLineParser::removeParameter(const QString &id)
{
    for (int i=0; i<params_.size(); ++i)
    if (params_[i]->id() == id)
    {
        delete params_[i];
        params_.removeAt(i);
        break;
    }
}

bool CommandLineParser::addParameter(CommandLineParameter * param)
{
    for (auto p : params_)
    if (param->id() == p->id())
    {
        MO_WARNING("Duplicate commandline parameter id '" << param->id() << "' ignored");
        return false;
    }

    params_.append(param);
    return true;
}

CommandLineParameter * CommandLineParser::addParameter(
        const QString &id, const QString &name, const QString &description)
{
    auto p = new CommandLineParameter(id, name, description);
    if (!addParameter(p))
    {
        delete p;
        return 0;
    }
    return p;
}


QString CommandLineParser::helpString(int max_width) const
{
    QString help;
    QTextStream s(&help);

    for (const CommandLineParameter * p : params_)
    {
        // name(s)
        for (int i=0; i<p->names().size(); ++i)
        {
            if (i>0)
                s << ", ";
            s << "-" << p->names()[i];
        }
        // type and default value
        if (p->type() != CommandLineParameter::T_NONE)
        {
            s << " " << p->typeName()
              << " (default = ";
            for (int j=0; j<p->numValues(); ++j)
            {
                if (j > 0)
                    s << " ";
                s << p->defaultValue(j).toString();
            }
            s << ")";
        }
        s << "\n";

        // description
        s << fit_text_block(p->description(), max_width, "   ");

        s << "\n\n";
    }

    return help;
}

QString CommandLineParser::helpStringHtml() const
{
    QString help;
    QTextStream s(&help);

    s << "<table>";
    for (const CommandLineParameter * p : params_)
    {
        s << "<tr>\n <td>";
        // name(s)
        for (int i=0; i<p->names().size(); ++i)
        {
            if (i>0)
                s << ", ";
            s << "<b>-" << p->names()[i].toHtmlEscaped() << "</b>";
        }
        s << "</td>\n <td>";
        // type and default value
        if (p->type() != CommandLineParameter::T_NONE)
            s << p->typeName()
              << "<br/>default = <b>" << p->defaultValue().toString().toHtmlEscaped() << "</b>";

        s << "</td>\n";

        // description
        s << " <td>" << p->description().toHtmlEscaped().replace('\n', "<br/>") << "</td>\n";

        s << "</tr>\n";
    }
    s << "</table>\n";

    return help;
}

QString CommandLineParser::valueString(bool use_id_as_name) const
{
    QString string;
    QTextStream s(&string);

    s.setFieldAlignment(QTextStream::AlignLeft);

    for (const CommandLineParameter * p : params_)
    if (p->type() != CommandLineParameter::T_NONE)
    {
        s.setFieldWidth(20);
        if (use_id_as_name)
            s << p->id();
        else
            s << p->names()[0];
        s.setFieldWidth(0);

        s << " = ";
        for (int i=0; i<p->numValues(); ++i)
        {
            if (i>0)
                s << " ";
            s << p->value(i).toString();
        }
        s << "\n";
    }

    return string;
}


bool CommandLineParser::parse(int argc, char *argv[], int skip)
{
    QStringList l;
    for (int i=skip; i<argc; ++i)
        l << argv[i];

    return parse(l);
}

bool CommandLineParser::parse(const QStringList & list)
{
    // clear error log
    error_.clear();

    // initialize parameters
    for (auto p : params_)
    {
        p->pres_ = false;
        p->v_ = p->d_;
    }

    // clear arguments
    arguments_.clear();

    // scan each token
    for (int i=0; i<list.size(); ++i)
    {
        QString name = list[i];

        // check if parameter
        bool isparam = false;
        while (name.startsWith('-'))
        {
            isparam = true;
            name = name.mid(1);
        }

        // no parameter? so it's an 'argument'
        if (!isparam)
        {
            arguments_.append(name);
            continue;
        }

        // find parameter
        CommandLineParameter * par = 0;
        for (auto p : params_)
        if (p->hasName(name))
        {
            par = p;
            break;
        }

        // unknown?
        if (!par)
        {
            if (opt_ & O_SKIP_UNKNOWN_PARAMETERS)
            {
                error_ += tr("skipped unknown parameter '%1'\n").arg(name);
                continue;
            }
            else
            {
                error_ += tr("unknown parameter '%1'\n").arg(name);
                return false;
            }
        }

        // flag as present
        par->pres_ = true;

        // no value?
        if (par->type() == CommandLineParameter::T_NONE)
            continue;

        for (int j=0; j<par->numValues(); ++j)
        {
            // read value
            ++i;
            if (i >= list.size())
            {
                error_ += tr("missing %2th value after parameter '%1'\n").arg(name).arg(j+1);
                return false;
            }

            const QString& value = list[i];

            // translate value
            switch (par->type())
            {
                case CommandLineParameter::T_NONE: break;

                case CommandLineParameter::T_STRING:
                    par->v_[j] = value;
                break;

                #define MO__PARSE_PARAM(type__, command__)                                      \
                    case CommandLineParameter::type__:                                          \
                    {                                                                           \
                        bool ok;                                                                \
                        command__;                                                              \
                        if (ok)                                                                 \
                            par->v_[j] = v;                                                     \
                        else                                                                    \
                        {                                                                       \
                            error_ += tr("expected %1 after parameter '%2' but found '%3'\n")   \
                                    .arg(par->typeName()).arg(name).arg(value);                 \
                            if (!(opt_ & O_SKIP_ILLEGAL_VALUES))                                \
                                return false;                                                   \
                        }                                                                       \
                    }                                                                           \
                    break;

                MO__PARSE_PARAM(T_INT,    auto v = value.toInt(&ok) );
                MO__PARSE_PARAM(T_UINT,   auto v = value.toUInt(&ok) );
                MO__PARSE_PARAM(T_DOUBLE, auto v = value.toDouble(&ok) );

                #undef MO__PARSE_PARAM
            }
        }
    }

    return true;
}


} // namespace IO
} // namespace MO
