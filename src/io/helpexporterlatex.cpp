/** @file helpexporterlatex.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/5/2014</p>
*/

#include <QList>
#include <QMap>
#include <QSet>
#include <QDomDocument>
#include <QDir>
#include <QTextStream>

#include "helpexporterlatex.h"
#include "helpsystem.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

class HelpExporterLatex::Private
{
public:
    struct Link
    {
        Link(bool scanned=false) : scanned(scanned)
        { }

        bool scanned;
        QString content;
    };

    QList<QString> todo;

    QMap<QString, Link> links;
};




HelpExporterLatex::HelpExporterLatex(QObject *parent) :
    QObject (parent),
    help_   (new HelpSystem(this)),
    p_      (new Private())
{
}

HelpExporterLatex::~HelpExporterLatex()
{
    delete p_;
}

void HelpExporterLatex::save(const QString &directory)
{
    // create directories
    QDir dir(directory);
    if (!dir.mkpath(directory))
        MO_IO_ERROR(WRITE, "Can't create directory '" << directory << "'");
    QString imgdir = directory + QDir::separator() + "img";
    if (!dir.mkpath(imgdir))
        MO_IO_ERROR(WRITE, "Can't create directory '" << imgdir << "'");

    // --- init ---
    p_->links.clear();

    // start on index
    QDomDocument doc;
    p_->todo.append("index.html");

    // scan all pages
    while (p_->todo.begin() != p_->todo.end())
    {
        QString html = p_->todo.front();

        // see if scanned already
        if (!isScanned_(html))
        {
            // get document
            help_->loadXhtml(html, doc);
            // scan for links
            scanHtml_(html, doc);
        }

        p_->todo.pop_front();
    }

    // --- export ---
    QString outfile = directory + QDir::separator() + "matrixoptimizer.tex";
    QFile file(outfile);
    if (!file.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not create tex file '" << outfile << "'\n"
                    << file.errorString());

    QTextStream stream(&file);

    stream << "\\documentclass{article}\n"
              "\\begin{document}\n"
              "\\tableofcontents\n"
              "\\newpage\n";

    for (auto i=p_->links.begin(); i!=p_->links.end(); ++i)
    {
        stream << i.value().content;
    }

    stream << "\\end{document}\n";
}

void HelpExporterLatex::scanHtml_(const QString & url, QDomDocument &doc)
{
    MO_DEBUG_HELP("HelpExporterLatex::scanHtml_('"<< url << "')");

    // get sublinks
    QDomElement e = doc.documentElement();
    gatherLinks_(e);

    // say we scanned this url
    auto it = p_->links.find(url);
    if (it == p_->links.end())
        it = p_->links.insert(url, Private::Link(true));
    else
        it.value().scanned = true;

    // translate
    exportHtml_(doc, url, it.value().content);
}

bool HelpExporterLatex::isScanned_(const QString &url) const
{
    auto it = p_->links.find(url);
    return (it != p_->links.end() && it.value().scanned);
}

void HelpExporterLatex::gatherLinks_(QDomElement &e)
{
    if (!e.isElement())
        return;

    if (e.tagName() == "a" || e.tagName() == "link")
        getPotentialLink_(e);

    // traverse childs
    QDomNodeList list = e.childNodes();
    for (int i=0; i<list.count(); ++i)
    {
        QDomElement ce = list.item(i).toElement();
        if (ce.isElement())
            gatherLinks_(ce);
    }
}

void HelpExporterLatex::getPotentialLink_(const QDomElement &e)
{
    QString href = e.attribute("href");
    if (href.isEmpty())
        return;

    MO_DEBUG_HELP("HelpExporterLatex: found href '" << href << "'");

    // ignore these
    if (href.startsWith("#") || href.startsWith("http"))
        return;

    // strip anchor
    int idx = href.indexOf("#");
    if (idx>0)
        href = href.left(idx);

    // add to todo-list
    if (!isScanned_(href))
    {
        p_->todo.append(href);
        p_->links.insert(href, Private::Link());
    }
}

void HelpExporterLatex::gatherImages_(QDomElement &e)
{

}


void HelpExporterLatex::exportHtml_(QDomDocument &doc, const QString& url, QString &content)
{
    QTextStream stream(&content);

    stream << "% " << url << "\n";

    QDomElement e(doc.documentElement());
    prepareNode_(e);
    exportNode_(e, stream);

    stream << "% end " << url << "\n";
}

QString HelpExporterLatex::str_(const QString &text)
{
    QString str(text);
    str.replace("{", "\\{");
    str.replace("}", "\\}");
    str.replace("%", "\\%");
    str.replace("_", "\\_");
    str.replace("^", "\\^");
    str.replace("&", "\\&");
    return str;
}

void HelpExporterLatex::prepareNode_(QDomElement &e)
{
    // traverse childs first
    QDomNodeList list = e.childNodes();
    for (int i=0; i<list.count(); ++i)
    {
        QDomElement ce = list.item(i).toElement();
        if (ce.isElement())
            prepareNode_(ce);
    }

    const QString
            tag = e.tagName(),
            text = str_(e.text());
    QString repl;

    if (tag == "h1")
    {
        repl = QString("\\newpage\n"
                       "\\section{%1}\n").arg(text);
    }

    else
    if (tag == "h2")
    {
        repl = QString("\\subsection{%1}\n").arg(text);
    }

    else
    if (tag == "h3")
    {
        repl = QString("\\subsubsection{%1}\n").arg(text);
    }

    if (tag == "i")
    {
        repl = QString("\\textit{%1}").arg(text);
    }

    if (tag == "b")
    {
        repl = QString("\\textbf{%1}").arg(text);
    }

    if (tag == "td")
    {
        repl = QString("%1\\newline").arg(text);
    }

    if (tag == "p")
    {
        repl = text;
    }

    if (!repl.isEmpty())
        MO_DEBUG_HELP("prepared: [" << repl << "]");

    e.setAttribute("latex", repl);
}

void HelpExporterLatex::exportNode_(QDomElement &e, QTextStream &stream)
{
    const QString
            tag = e.tagName(),
            text = str_(e.text());

    //QDomNodeList list = e.childNodes();
    //exportNodeFragment_(e, stream);

    stream << "% tag=" << tag << "\n";

    if (tag == "html" || e.tagName() == "body" || e.tagName() == "head")
    {
        // nothing
    }

    if (e.hasAttribute("latex"))
        stream << e.attribute("latex");
    /*
    else
    if (tag == "p" || tag == "td" || tag == "h1" || tag == "h2" || tag == "h3")
        stream << text << "\n";
    */
    /*
    else
    if (tag == "h1")
    {
        stream << "\\newpage\n"
                  "\\section{" << text << "}\n";
    }

    else
    if (tag == "h2")
    {
        stream << "\\subsection{" << text << "}\n";
    }

    else
    if (tag == "h3")
    {
        stream << "\\subsubsection{" << text << "}\n";
    }

    else
    if (tag == "b")
    {
        stream << "\\textbf{" << text << "}";
    }

    else
    if (tag == "i")
    {
        stream << "\\textit{" << text << "}";
    }

    else
    if (tag == "p")
    {
        stream << "\n" << text << "\n";
    }

    else
    if (tag == "td")
    {
        stream << "\n" << text << "\\newline\n";
    }

    else
    if (tag == "pre")
    {
        QString textnl(text);
        textnl.replace("\n", "\\newline\n");
        stream <<
                  "\\texttt{" << textnl << "}"
                  "\\newline\n";
    }
    */

    // traverse childs
    QDomNodeList list = e.childNodes();
    for (int i=0; i<list.count(); ++i)
    {
        QDomElement ce = list.item(i).toElement();
        if (ce.isElement())
            exportNode_(ce, stream);
    }
}

void HelpExporterLatex::exportNodeFragment_(QDomElement &e, QTextStream &stream)
{
    // <p><a name="bla"/>Hal<i>l</i>o Welt</p>
    // "Hallo Welt"
    int y = e.lineNumber(),
        x = e.columnNumber();
    QDomNodeList list = e.childNodes();

    std::cout << "element " << e.tagName() << " "
              << y << ":" << x << " text [" << e.text() << "]"
              << std::endl;

    int lx = x, ly = y, tpos=0, ltpos = 0;
    for (int i=0; i<list.count(); ++i)
    {
        QDomElement ce = list.item(i).toElement();
        if (!ce.isElement())
            continue;

        int cy = ce.lineNumber(),
            cx = ce.columnNumber();

        if (cy>ly)
            tpos += lx;
        else
            tpos += std::max(0, cx - lx);
        /*
        if (cy == y)
            tpos += (cx - lx);
        if (cy>ly)
            tpos += lx;
        */

        QString textpart = e.text().mid(ltpos, tpos-ltpos);
        std::cout << "  textpart (" << ltpos << "-" << tpos << ") [" << textpart << "]\n"
                 "  subelement " << ce.tagName() << " "
                 << cy << ":" << cx << " text [" << ce.text() << "]"
                 << std::endl;

        lx = cx;
        ly = cy;
        ltpos = tpos;
    }
}


} // namespace MO
