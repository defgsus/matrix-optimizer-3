/** @file helptextbrowser.cpp

    @brief QTextBrowser with reimplemented loadResource() to work with resource files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QTextStream>
#include <QFile>
#include <QImage>

#include "helptextbrowser.h"
#include "io/log.h"
#include "math/funcparser/parser.h"

namespace MO {
namespace GUI {


HelpTextBrowser::HelpTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    setSearchPaths(QStringList() << ":/img" << ":/texture");
}

QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::HtmlResource)
    {
        const QString name = url.url();
        const QString fn = ":/help/"
            + (name.contains(".html")? name : name + ".html");

        QFile f(fn);
        if (!f.open(QFile::ReadOnly))
        {
            return tr("<h3>%1 is missing</h3>").arg(fn);
        }

        QTextStream s(&f);
        QString doc = s.readAll();

        return addRuntimeInfo_(doc, fn);
    }
    /*
    if (type == QTextDocument::ImageResource)
    {
        const QString fn = ":/texture/" + url.url();

        QImage img(fn);
        //return img.isNull() ? QImage(":/")
        return img;
    }*/

    return QTextBrowser::loadResource(type, url);
}


QString HelpTextBrowser::addRuntimeInfo_(
        const QString &orgdoc, const QString &filename) const
{
    if (filename.contains("equation.html"))
    {
        QString str;

        PPP_NAMESPACE::Parser p;

        std::vector<PPP_NAMESPACE::Variable*> vars;
        p.variables().getVariables(vars, false);
        for (PPP_NAMESPACE::Variable * v : vars)
        {
            if (!v->isConst())
                continue;

            str += QString("<b>%1<&b> = %2<br/>\n")
                    .arg(QString::fromStdString(v->name())).arg(v->value());
        }

        QString doc(orgdoc);
        doc.replace("!CONSTANTS!", str);

        str = "<table>";
        auto funcs = p.functions().getFunctions();
        for (const PPP_NAMESPACE::Function * f : funcs)
        {
            if (f->type() != PPP_NAMESPACE::Function::FUNCTION
                || f->name() == "?")
                continue;

            str += "<tr><td><b>" + QString::fromStdString(f->name()) + "</b>(";
            for (int i=0; i<f->num_param(); ++i)
            {
                str += QChar('a'+i);
                if (i<f->num_param()-1)
                    str += ", ";
            }
            str += ")</td><td>" + getFunctionDescription_(f) + "</td></tr>\n";
        }
        str += "</table>"

        doc.replace("!FUNCTIONS!", str);

        return doc;
    }


    return orgdoc;
}

QString HelpTextBrowser::getFunctionDescription_(
        const PPP_NAMESPACE::Function * f) const
{
    if (f->name() == "abs")
        return tr("Returns the absolute value - that is the result is always positive.");
    if (f->name() == "sign")
        return tr("Returns +1.0 if <i>a</i> is positive, -1.0 if <i>a</i> is negative and "
                  "0.0 if <i>a</i> is also zero.");

    return "";
}


} // namespace GUI
} // namespace MO
