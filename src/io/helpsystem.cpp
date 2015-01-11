/** @file helpsystem.cpp

    @brief Pseudo-xhtml parser for resource help texts

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QImage>

#include "helpsystem.h"
#include "io/log.h"
#include "gui/widget/equationdisplaywidget.h"
#include "gui/util/viewspace.h"
#include "math/funcparser/parser.h"

#ifdef MO_DO_DEBUG_HELP
#   include <iomanip>
#endif

namespace MO {

HelpSystem::HelpSystem(QObject *parent) :
    QObject(parent)
{
    MO_DEBUG_HELP("HelpSystem::HelpSystem()");

    searchPaths_
            << ":/help"
            << ":/helpimg"
            << ":/img"
            << ":/texture";

}


QString HelpSystem::findResource(const QString &iurl, ResourceType type)
{
    // avoid these
    if (iurl.isEmpty() || iurl.startsWith("http"))
    {
        MO_DEBUG_HELP("HelpSystem::findResource(" << iurl << ") ingored");
        return QString();
    }

    QString url(iurl);

    if (type == HtmlResource)
    {
        // complete with extension
        if (!url.contains(".html", Qt::CaseInsensitive))
        {
            url.append(".html");
        }

        // strip anchor
        const int idx = url.indexOf("#");
        if (idx > 0)
            url = url.left(idx);
    }

#ifdef MO_DO_DEBUG_HELP
    if (url != iurl)
        MO_DEBUG_HELP("HelpSystem::findResource(" << iurl << ") reformed: " << url);
#endif

    if (QFileInfo(url).exists())
    {
        MO_DEBUG_HELP("HelpSystem::findResource(" << url << ") ok");
        return url;
    }

    // find in searchPaths_
    for (auto &s : searchPaths_)
    {
        QString newurl = s + QDir::separator() + url;
        if (QFileInfo(newurl).exists())
        {
            MO_DEBUG_HELP("HelpSystem::findResource(" << url << ") found " << newurl);
            return newurl;
        }
    }

    MO_DEBUG_HELP("HelpSystem::findResource(" << url << ") not found");
    return QString();
}

QVariant HelpSystem::loadResource(const QString &partial_url, ResourceType type)
{
    MO_DEBUG_HELP("HelpSystem::loadResource(" << partial_url << ")");

    if (type == HtmlResource || type == StyleSheetResource)
    {
        QString url = findResource(partial_url, type);
        if (url.isEmpty())
            return QVariant();

        // open the file
        QFile file(url);
        if (!file.open(QFile::ReadOnly))
        {
            MO_DEBUG_HELP("HelpSystem::loadResource() can't open '" << url << "'");
            return QVariant();
        }

        // return as text
        QTextStream stream(&file);
        return stream.readAll();
    }

    if (type == ImageResource)
    {
        if (partial_url.startsWith("_equ"))
            return getEquationImage(partial_url);

        QString url = findResource(partial_url, type);
        if (url.isEmpty())
            return QVariant();

        QImage img(url);
        if (img.isNull())
        {
            MO_DEBUG_HELP("HelpSystem::loadResource() can't open image '" << url << "'");
            return QVariant();
        }

        return img;
    }

    return QVariant();
}

bool HelpSystem::loadXhtml(const QString &partial_url, QDomDocument &doc)
{
    MO_DEBUG_HELP("HelpSystem::loadXhtml('" << partial_url << "')");

    QString url = findResource(partial_url, HtmlResource);
    if (url.isEmpty())
        return false;

    // open the file
    QFile file(url);
    if (!file.open(QFile::ReadOnly))
    {
        MO_DEBUG_HELP("HelpSystem::loadResource() can't open '" << url << "'");
        return false;
    }

    // get text
    QTextStream stream(&file);

    // put tags around
    QString xhtml
            = "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
              "<head>\n"
              "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n"
              "</head>\n"
              "<body>\n"
            + stream.readAll()
            + "</body></html>";

    // add runtime info to equations page
    if (url.contains("equation.html"))
        addEquationInfo_(xhtml);

    if (url.contains("angelscript.html"))
        addAngelScriptInfo_(xhtml);

    // get dom tree
    QString error;
    int line, column;
    if (!doc.setContent(xhtml, &error, &line, &column))
    {
#ifdef MO_DO_DEBUG_HELP
        QString errline = xhtml.section('\n', line-3, line-1);
        MO_DEBUG_HELP("HelpSystem::loadXhtml(" << url << ") parse error\n"
                      << line << ":" << column << " "
                      << error << "\n"
                      << errline << "\n" << std::setw(column) << "^");
#endif
        return false;
    }

    return true;
}



void HelpSystem::renderHtml(const QDomDocument & doc, QTextStream & stream)
{
    const QDomElement e = doc.documentElement();

    e.save(stream,0);
    //if (!e.isNull())
    //    renderHtml_(e, stream);
}


void HelpSystem::renderHtml_(const QDomElement & e, QTextStream & stream)
{
    if (e.tagName() == "img")
    {
        renderHtmlImg_(e, stream);
        return;
    }

    // start tag
    stream << "<" << e.tagName();

    // get attributes
    const QDomNamedNodeMap attr = e.attributes();
    for (int i=0; i<attr.count(); ++i)
    {
        const QDomAttr a = attr.item(i).toAttr();
        // write attribute
        stream << " " << a.name() << "=\""
               << a.value() << "\"";
    }

    stream << ">";

    if (e.tagName() != "html" && e.tagName() != "body")
        stream << e.nodeValue();

    // traverse childs
    const QDomNodeList childs = e.childNodes();

    for (int i=0; i<childs.count(); ++i)
    {
        const QDomElement ce = childs.at(i).toElement();
        if (!ce.isNull())
            renderHtml_(ce, stream);
    }

    // close tag
    stream << "</" << e.tagName() << ">\n";
}

void HelpSystem::renderHtmlImg_(const QDomElement & e, QTextStream & stream)
{
    stream << "<img";

    // get attributes
    const QDomNamedNodeMap attr = e.attributes();
    for (int i=0; i<attr.count(); ++i)
    {
        const QDomAttr a = attr.item(i).toAttr();
        // write attribute
        stream << " " << a.name() << "=\""
               << a.value() << "\"";
    }

    stream << "/>";

    if (e.hasChildNodes())
    {
        MO_DEBUG_HELP("HelpSystem::renderHtmlImg_() childs of img elements not supported");
    }
}

QImage HelpSystem::getEquationImage(const QString &url) const
{
    MO_DEBUG_HELP("HelpSystem::getEquationImage('" << url << "'");

    Double
            xstart = url.section(QChar('#'), 1, 1).toDouble(),
            xend = url.section(QChar('#'), 2, 2).toDouble(),
            ymin = url.section(QChar('#'), 3, 3).toDouble(),
            ymax = url.section(QChar('#'), 4, 4).toDouble();
    const QString
            equation = url.section(QChar('#'), 5, 5),
            widthstr = url.section(QChar('#'), 6, 6),
            heightstr = url.section(QChar('#'), 7, 7);

    // determine width and height
    int width = 400, height = 100;
    if (!widthstr.isEmpty())
    {
        width = widthstr.toInt();
        if (heightstr.isEmpty())
            height = width;
        else
            height = heightstr.toInt();
    }

    // expand the y-view a little
    ymax += (ymax - ymin) / height;

    QImage img(width, height, QImage::Format_RGB32);

    GUI::EquationDisplayWidget display;
    display.resize(width, height);
    display.setPaintMode(GUI::EquationDisplayWidget::PM_F_OF_X);
    display.setEquation(equation);
    display.setViewSpace(GUI::UTIL::ViewSpace(xstart, ymin, xend-xstart, ymax-ymin));
    display.render(&img);

    return img;
}

void HelpSystem::loadEquationFunctions_()
{
    QFile f(":/help/equationfunctions.html");
    if (!f.open(QFile::ReadOnly))
    {
        MO_DEBUG_HELP("HelpSystem::loadEquationFunctions_(): "
                      "equtionfunctions.html is missing from resources");
        return;
    }

    QTextStream stream(&f);
    QString text = stream.readAll();

    // find entry of each function description
    int x = text.indexOf("{{");
    while (x >= 0)
    {
        EquFunc_ equfunc;

        // read function signature
        x += 2;
        int end = text.indexOf("\n", x);
        equfunc.niceDisplay = text.mid(x, end-x).trimmed();

        // find end of description
        x = end + 1;
        end = text.indexOf("}}", x);

        // copy strings
        equfunc.help = text.mid(x, end-x);
        equfunc.numParams = equfunc.niceDisplay.count(',') + 1;

        // strip signature from function name
        x = equfunc.niceDisplay.indexOf('(');
        QString funcName = equfunc.niceDisplay.left(x);

        // put <i></i>s around parameters
        equfunc.niceDisplay.insert(x+1, "<i>");
        equfunc.niceDisplay.insert(
                    equfunc.niceDisplay.indexOf(')'), "</i>");

        funcMap_.insert(funcName, equfunc);

        // find next entry
        x = text.indexOf("{{", end);
    }
}

void HelpSystem::addEquationInfo_(QString& doc)
{
    // load the function descriptions
    if (funcMap_.isEmpty())
        loadEquationFunctions_();

    QString str;

    PPP_NAMESPACE::Parser p;

    // ----- constants -------

    std::vector<PPP_NAMESPACE::Variable*> vars;
    p.variables().getVariables(vars, false);

    str += "<table>\n";
    for (PPP_NAMESPACE::Variable * v : vars)
    {
        if (!v->isConst())
            continue;

        str += QString("<tr><td><b>%1&nbsp;&nbsp;</b></td> "
                       "<td>%2&nbsp;&nbsp;</td> <td>= %3</td></tr>\n")
                .arg(QString::fromStdString(v->name()))
                .arg(QString::fromStdString(v->description()))
                .arg(v->value());
    }
    str += "</table>\n";

    doc.replace("!CONSTANTS!", str);

    // ---- functions ----

    QStringList groups, groupsTr;
    groups  << "basic"
            << "transition"
            << "algebraic"
            << "trigonometry"
            << "geometry"
            << "statistical"
            << "number theory"
            << "oscillator"
            << "random"
            << "chaotic";
    groupsTr
            << tr("basic functions")
            << tr("transition functions")
            << tr("algebraic functions")
            << tr("trigonometric functions")
            << tr("geometric functions")
            << tr("statistical functions")
            << tr("number theory functions")
            << tr("oscillator functions")
            << tr("random functions")
            << tr("chaotic functions");

    str = "";
    auto funcs = p.functions().getFunctions();
    for (int k = 0; k<groups.size(); ++k)
    {
        str += "<h3>" + groupsTr[k] + "</h3>\n";
        str += "<table border=\"0\">\n";
        const std::string curgroup = groups[k].toStdString();
        for (const PPP_NAMESPACE::Function * f : funcs)
        {
            if (f->groupName() != curgroup)
                continue;

            if (f->type() != PPP_NAMESPACE::Function::FUNCTION
                || f->name() == "?")
                continue;

            const QString
                    fname = QString::fromStdString(f->name()),
                    anchor = fname,
                    anchorp = anchor + QString::number(f->num_param());

            QString funcName = fname,
                    helpText = tr("no description available");

            auto funcs = funcMap_.values(fname);
            for (EquFunc_ &fu : funcs)
            {
                if (fu.numParams == f->num_param())
                {
                    funcName = fu.niceDisplay;
                    helpText = fu.help;
                    break;
                }
            }

            str += "<tr><td>"
                   "<a name=\"" + anchor + "\"></a>"
                   "<a name=\"" + anchorp + "\"></a>"
                   "<b>" + funcName + "</b>"
                   "&nbsp;&nbsp;</td><td>"
                   + helpText + "<br/><br/></td></tr>\n";
        }
        str += "</table>\n";
    }

    doc.replace("!FUNCTIONS!", str);
}


void HelpSystem::addAngelScriptInfo_(QString& doc)
{
    QString str;


    doc.replace("!FUNCTIONS!", str);
}


} // namespace MO
