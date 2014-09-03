/** @file helptextbrowser.cpp

    @brief QTextBrowser with reimplemented loadResource() to work with resource files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QTextStream>
#include <QFile>
#include <QImage>
#include <QPainter>

#include "helptextbrowser.h"
#include "io/log.h"
#include "io/error.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"

namespace MO {
namespace GUI {

namespace
{
    class EquationData : public PAINTER::ValueCurveData
    {
    public:
        mutable PPP_NAMESPACE::Parser p;
        mutable Double x, xr;

        EquationData(const QString& equ)
        {
            p.variables().add("x", &x, "");
            p.variables().add("xr", &xr, "");
            p.parse(equ.toStdString());
        }
        Double value(Double time) const Q_DECL_OVERRIDE
        {
            x = time;
            xr = x * TWO_PI;
            return p.eval();
        }
    };
}


HelpTextBrowser::HelpTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    setOpenExternalLinks(true);

    setSearchPaths(
        QStringList()
                << ":/help"
                << ":/helpimg"
                << ":/img"
                << ":/texture"
                );

    loadEquationFunctions_();
}


QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::HtmlResource)
    {
        const QString name = url.url();
        QString fn = ":/help/"
            + (name.contains(".html")? name : name + ".html");

        // strip anchor
        int idx = fn.indexOf("#");
        if (idx > 0)
            fn = fn.left(idx);

        QFile f(fn);
        if (!f.open(QFile::ReadOnly))
        {
            return tr("<h3>%1 is missing</h3>").arg(fn);
        }

        QTextStream s(&f);
        QString doc = s.readAll();

        return addRuntimeInfo_(doc, fn);
    }

    if (type == QTextDocument::ImageResource)
    {
        if (url.url().startsWith("_equ"))
            return getFunctionImage(url.url());
    }

    return QTextBrowser::loadResource(type, url);
}


QString HelpTextBrowser::addRuntimeInfo_(
        const QString &orgdoc, const QString &filename) const
{
    QString doc =
            "<html><head>\n"
            "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">"
            "</head><body>\n"
            + orgdoc;

    if (filename.contains("equation.html"))
    {
        addEquationInfo_(doc);
    }

    doc += "</body></html>";

    return doc;
}

void HelpTextBrowser::addEquationInfo_(QString& doc) const
{
    QString str;

    PPP_NAMESPACE::Parser p;

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
                   + helpText + "<br/></td></tr>\n";
        }
        str += "</table>\n";
    }

    doc.replace("!FUNCTIONS!", str);
}

QImage HelpTextBrowser::getFunctionImage(const QString &url) const
{
    //    MO_DEBUG("making image " << equation);

    PPP_NAMESPACE::Float
            xstart = url.section(QChar('#'), 1, 1).toDouble(),
            xend = url.section(QChar('#'), 2, 2).toDouble(),
            ymin = url.section(QChar('#'), 3, 3).toDouble(),
            ymax = url.section(QChar('#'), 4, 4).toDouble();
    const QString
            equation = url.section(QChar('#'), 5, 5),
            widthstr = url.section(QChar('#'), 6, 6),
            heightstr = url.section(QChar('#'), 7, 7);

    // create parser and test equation
    EquationData data(equation);
    if (!data.p.ok())
        return QImage();

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
    ymin -= (ymax-ymin) / 50;
    ymax += (ymax-ymin) / 50;

    // get image and painter
    QImage img(width,height,QImage::Format_RGB32);
    img.fill(Qt::black);

    QPainter paint(&img);
    paint.setRenderHint(QPainter::Antialiasing, true);

    UTIL::ViewSpace vs(xstart,ymin, xend-xstart, ymax-ymin);

    // draw background grid
    PAINTER::Grid grid;
    grid.setOptions(PAINTER::Grid::O_DrawAll);
    grid.setViewSpace(vs);
    grid.paint(paint);

    // -- value curve --

    PAINTER::ValueCurve curve;
    curve.setViewSpace(vs);
    curve.setCurveData(&data);
    curve.paint(paint);

    return img;
}

void HelpTextBrowser::loadEquationFunctions_()
{
    QFile f(":/help/equationfunctions.html");
    if (!f.open(QFile::ReadOnly))
        MO_IO_ERROR(READ, "equtionfunctions.html is missing from resources");
    QTextStream stream(&f);
    QString text = stream.readAll();

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

        // add <i></i>s around parameters
        equfunc.niceDisplay.insert(x+1, "<i>");
        equfunc.niceDisplay.insert(
                    equfunc.niceDisplay.indexOf(')'), "</i>");
        /*
        bool ins = true;
        while (++x < equfunc.niceDisplay.size()
               && equfunc.niceDisplay.at(x) != ')')
        {
            if (!ins && equfunc.niceDisplay.at(x) == ',')
            {
                ++x;
                ins = true;
            }
            if (ins)
            {
                equfunc.niceDisplay.insert(x, "<i>");
                x+=3;
                ins = false;
            }
            if (equfunc.niceDisplay.at(x) == ','
            || equfunc.niceDisplay.at(x) == ')'
            || equfunc.niceDisplay.at(x).isSpace())
            {
                equfunc.niceDisplay.insert(x, "</i>");
                x+=4;
            }
        }
        MO_DEBUG("["<<equfunc.niceDisplay<<"]");
        */

        funcMap_.insert(funcName, equfunc);

        x = text.indexOf("{{", end);
    }
}


} // namespace GUI
} // namespace MO
