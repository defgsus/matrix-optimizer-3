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

#include "HelpExporterLatex.h"
#include "HelpSystem.h"
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

    QString curUrl;
    bool hadnewline;
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
              "\\usepackage{hyperref}\n"
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

void HelpExporterLatex::gatherImages_(QDomElement &)
{

}


void HelpExporterLatex::exportHtml_(QDomDocument &doc, const QString& url, QString &content)
{
    QTextStream stream(&content);

    stream << "% --- begin of " << url << " ---\n";

    stream << "\\newpage"
           << "\\hypertarget{" << url << "}{}\n";

    p_->curUrl = url;

    QDomNode n(doc.documentElement());
    exportNode_(n, stream);

    stream << "\n% --- end of " << url << " ---\n";
}

QString HelpExporterLatex::str_(const QString &text)
{
    QString str(text);
    str.replace("{", "\\{");
    str.replace("}", "\\}");
    str.replace("%", "\\%");
    str.replace("_", "\\_");
    str.replace("^", "\\textasciicircum");
    str.replace("~", "\\textasciitilde");
    str.replace("&", "\\&");
    str.replace("ß", "\\SS");
    str.replace("<", "\\textless");
    str.replace(">", "\\textgreater");
    return str;
}

void HelpExporterLatex::exportNode_(const QDomNode & n, QTextStream &stream, bool newl)
{
#define MO__CHILDS \
    { exportChilds_(n, stream, newl); \
      p_->hadnewline = newline; }
// XXX still too many newlines created.
//     this test is not working correctly
#define MO__NEWLINE \
    if (!p_->hadnewline && !newline) \
        { stream << "\\newline\n"; p_->hadnewline = newline = true; }

    bool newline = false;

    if (n.isText())
    {
        if (!newl)
            stream << str_(n.nodeValue());
        else
        {
            QString textnl(str_(n.nodeValue()));
            textnl.replace("\n", "\\newline\n");
            stream << textnl;
        }
    }
    else
    if (n.isElement())
    {
        QDomElement e = n.toElement();
        const QString tag = e.tagName();

        stream << "% tag=" << tag << "\n";

        if (tag == "a")
        {
            if (e.hasAttribute("href"))
            {
                QString href = e.attribute("href");

                // external links
                if (href.startsWith("http"))
                {
                    stream << "\\href{" << href << "}{";
                    MO__CHILDS;
                    stream << "}";
                }
                // internal links
                else
                {
                    // add current page url
                    if (href.startsWith("#"))
                        href = p_->curUrl + href;

                    // Tex doesn't like #
                    href.replace("#", "_");

                    stream << "\\hyperlink{" << href << "}{";
                    MO__CHILDS;
                    stream << "}";
                }
            }
            // create a link target
            else if (e.hasAttribute("name"))
            {
                stream << "\\hypertarget{"
                       << p_->curUrl << "_" << e.attribute("name")
                       << "}{}";
                MO__CHILDS;
            }
        }

        else
        if (tag == "h1")
        {
            stream << "\\section{";
            MO__CHILDS;
            stream << "}\n";
        }

        else
        if (tag == "h2")
        {
            stream << "\\subsection{";
            MO__CHILDS;
            stream << "}\n";
        }

        else
        if (tag == "h3")
        {
            stream << "\\subsubsection{";
            MO__CHILDS;
            stream << "}\n";
        }

        else
        if (tag == "b")
        {
            stream << "\\textbf{";
            MO__CHILDS;
            stream << "}";
        }

        else
        if (tag == "i")
        {
            stream << "\\textit{";
            MO__CHILDS;
            stream << "}";
        }

        else
        if (tag == "p")
        {
            stream << "\\paragraph*{}";
            MO__CHILDS;
        }

        else
        if (tag == "pre")
        {
            MO__NEWLINE;
            stream << "\\texttt{";
            exportChilds_(n, stream, true);
            stream << "}";
            MO__NEWLINE;
        }

        else
        if (tag == "br")
        {
            MO__NEWLINE;
            MO__CHILDS;
        }
        /*
        else
        if (tag == "table")
        {
            stream << "\\begin{tabular}[t]{llll}\n";
            MO__CHILDS;
            stream << "\\end{tabular}\n";
        }

        else
        if (tag == "td")
        {
            MO__CHILDS;
            //stream << " &\n";
        }
        */
        else
        if (tag == "td")
        {
            MO__CHILDS;
            MO__NEWLINE;
        }

        else
            MO__CHILDS;
    }

#undef MO__CHILDS
#undef MO__NEWLINE
}

void HelpExporterLatex::exportChilds_(const QDomNode &n, QTextStream &stream, bool newl)
{
    // traverse childs
    QDomNodeList list = n.childNodes();
    for (int i=0; i<list.count(); ++i)
    {
        exportNode_(list.item(i), stream, newl);
    }
}


} // namespace MO
