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

    if (type == HtmlResource)
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
            = "<html xmlns=\"http://www.w3.org/1999/xhtml\"><body>\n"
            + stream.readAll()
            + "</body></html>";

    // get dom tree
    QString error;
    int line, column;
    if (!doc.setContent(xhtml, &error, &line, &column))
    {
#ifdef MO_DO_DEBUG_HELP
        QString errline = xhtml.section('\n', line-1, line);
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

} // namespace MO
