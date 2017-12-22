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
#include <QIcon>
#include <QPainter>

#include "HelpSystem.h"
#include "io/log.h"
#include "gui/widget/EquationDisplayWidget.h"
#include "gui/util/ViewSpace.h"
#include "gui/util/AppIcons.h"
#include "gui/item/AbstractObjectItem.h"
#include "math/funcparser/parser.h"
#include "object/util/ObjectFactory.h"
#include "object/Object.h"
#include "object/param/Parameters.h"
#include "script/angelscript.h"
#include "python/34/python.h"

#ifdef MO_DO_DEBUG_HELP
#   include <iomanip>
#endif

namespace MO {


namespace {
    static QString help_url_ = "index.html";
}

void setHelpUrl(const QString &url) { help_url_ = url; }
QString currentHelpUrl() { return help_url_; }


HelpSystem::HelpSystem(QObject *parent) :
    QObject(parent)
{
    MO_DEBUG_HELP("HelpSystem::HelpSystem()");

    searchPaths_
            << ":/help"
            << ":/helpimg"
            << ":/img"
            << ":/icon"
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

        if (partial_url.startsWith("_oi_"))
            return getObjectImage(partial_url.mid(4));

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

    QString pseudo_html, url;

    // getruntime created pages ?
    QVariant rv = getRuntimeResource(partial_url);
    if (rv.isValid() && rv.canConvert(QVariant::String))
    {
        url = partial_url;
        pseudo_html = rv.toString();
    }
    // load from resource file
    else
    {
        url = findResource(partial_url, HtmlResource);
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
        pseudo_html = stream.readAll();
    }

    // put tags around
    QString xhtml
            = "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
              "<head>\n"
              "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n"
              "</head>\n"
              "<body>\n"
            + pseudo_html
            + "</body></html>";

    // --- add runtime infos to pages ---

        if (url.contains("equation.html"))
            addEquationInfo_(xhtml);
#ifndef MO_DISABLE_ANGELSCRIPT
        if (url.contains("angelscript.html"))
            addAngelScriptInfo_(xhtml);
#endif
        if (url.contains("python"))
            addPythonInfo_(xhtml);

        addObjectIndex_(xhtml);


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

QVariant HelpSystem::getRuntimeResource(const QString &url)
{
    // find achor
    QString anchor;
    const int aidx = url.indexOf("#");
    if (aidx > 0)
        anchor = url.mid(aidx+1);
    // find actual name
    QString name;
    if (aidx > 0)
        name = url.left(aidx);
    else
        name = url;
    name.remove(".html", Qt::CaseInsensitive);

    MO_DEBUG_HELP("HelpSystem::getRuntimeResource("
                  << url << "): name='" << name << "', anchor='" << anchor << "'");

    // specific Object page
    if (name.startsWith("_object_"))
    {
        QString objName = name.mid(8);
        if (objName.isEmpty())
            return QVariant();

        auto o = ObjectFactory::createObject(objName);
        return o ? getObjectDoc(o) : QString();
    }

    return QVariant();
}

QString HelpSystem::getObjectDoc(const Object * o)
{
    QString str;
    QTextStream html(&str);

    html << "<h1>"
         << "<img src=\"_oi_" << o->className() << "\"/>"
         << o->name().toHtmlEscaped() << "</h1>";

    QVariant v = loadResource("_od_" + o->className(), HtmlResource);
    if (v.isValid())
        html << v.toString() << "\n";
    else
        html << tr("No description currently available on this object");

    html << "<a name=\"parameters\"></a><h2>" << o->name().toHtmlEscaped() << " " << tr("parameters") << "</h2>";

    html << o->params()->getParameterDoc();

    return str;
}

QImage HelpSystem::getObjectImage(const QString &url) const
{
    const QString
            className = url.section(QChar('#'), 0, 0),
            widthstr = url.section(QChar('#'), 1, 1),
            heightstr = url.section(QChar('#'), 2, 2);

    // create a temp object
    auto o = ObjectFactory::createObject(className);
    if (!o)
        return QImage();

    // create a temp object item (easiest way to paint nicely)
    GUI::AbstractObjectItem item(o);

    // determine width and height for drawing
    QSize itemsize = item.boundingRect().size().toSize(),
          size = itemsize;
    if (!widthstr.isEmpty())
    {
        size.setWidth( widthstr.toInt() );
        if (heightstr.isEmpty())
            size.setHeight( size.width() );
        else
            size.setHeight( heightstr.toInt() );
    }

    // paint on image
    QImage img(itemsize, QImage::Format_ARGB32);
    QPainter p(&img);
    p.fillRect(0,0, size.width(), size.height(), QBrush(Qt::black));
    p.translate(-item.boundingRect().topLeft());
    item.paint(&p, 0, 0);

    o->releaseRef("HelpSystem getObjectImage release temp");
    return size != itemsize
            ? img.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
            : img;
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
    MO_DEBUG_HELP("HelpSystem::addEquationInfo_()");

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

#ifndef MO_DISABLE_ANGELSCRIPT
void HelpSystem::addAngelScriptInfo_(QString& doc)
{
    if (doc.contains("!FUNCTIONS_INDEX!"))
    {
        QString str = getAngelScriptFunctionsIndexHtml();
        doc.replace("!FUNCTIONS_INDEX!", str);
    }

    if (doc.contains("!EXAMPLES!"))
    {
        QString str = exampleAngelScript().toHtmlEscaped();
        doc.replace("!EXAMPLES!", str);
    }

    if (doc.contains("!FUNCTIONS!"))
    {
        QString str = getAngelScriptFunctionsHtml();
        doc.replace("!FUNCTIONS!", str);
    }
}
#endif

void HelpSystem::addPythonInfo_(QString &doc)
{
    Q_UNUSED(doc);
#ifdef MO_ENABLE_PYTHON34
    if (doc.contains("!REFERENCE!"))
    {
        QString str = PYTHON34::PythonInterpreter::getHelpHtmlString();
        doc.replace("!REFERENCE!", str);
    }
#endif
}

void HelpSystem::addObjectIndex_(QString &doc)
{
    if (!doc.contains("!OBJECT_INDEX!"))
        return;

    // get all objects
    QList<const Object*> list(ObjectFactory::objects(Object::TG_ALL & ~Object::T_DUMMY));
    // sort by priority
    //qStableSort(list.begin(), list.end(), sortObjectList_Priority);
    // sort into groups
    QMap<QString, QList<const Object*>> map;
    for (auto o : list)
    {
        QString groupName = ObjectFactory::objectPriorityName(ObjectFactory::objectPriority(o));
        auto i = map.find(groupName);
        if (i == map.end())
            map.insert(groupName, QList<const Object*>() << o);
        else
            i.value() << o;
    }


    QString str = "<ul>\n";
    for (auto i = map.begin(); i != map.end(); ++i)
    {
        // group name
        str += "<li><b>" + i.key() + "</b><ul>\n";
        for (auto j : i.value())
        {
            str += "<li>";
            // link
            str += "<a href=\"_object_" + j->className() + ".html\">";
            // image
            if (!j->isAudioObject())
                str += "<img src=\"_oi_" + j->className() + "#24#24\"/>";
            // name
            str += j->name().toHtmlEscaped() + "</a></li>\n";
        }
        str += "</ul></li>";
    }
    str += "</ul>";

    doc.replace("!OBJECT_INDEX!", str);
}


} // namespace MO
