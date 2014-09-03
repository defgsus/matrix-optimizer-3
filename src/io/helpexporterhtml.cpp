/** @file helpexporterhtml.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/
#include <QDebug>
#include <QDomDocument>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QImage>

#include "helpexporterhtml.h"
#include "helpsystem.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

HelpExporterHtml::HelpExporterHtml(QObject *parent) :
    QObject (parent),
    help_   (new HelpSystem(this))

{
}

void HelpExporterHtml::save(const QString &directory)
{
    QDir dir(directory);
    if (!dir.mkpath(directory))
        MO_IO_ERROR(WRITE, "Can't create directory '" << directory << "'");


    QDomDocument doc;
    htmls_.clear();
    htmlsExp_.clear();

    help_->loadXhtml("index.html", doc);

    // write first page
    prepareHtml_(doc);
    writeHtml_(directory + QDir::separator() + "index.html", doc);
    htmls_.push_back("index.html");
    htmlsExp_.insert("index.html", true);

    // write all other pages
    while (htmls_.begin() != htmls_.end())
    {
        const QString link = htmls_.front();

        // not saved?
        if (!htmlsExp_[link])
        {
            // remove from todolist
            htmlsExp_[link] = true;
            htmls_.pop_front();

            if (!help_->loadXhtml(link, doc))
            {
                MO_IO_WARNING(READ, "Could not load xhtml '" << link << "' "
                              "from HelpSystem");
                continue;
            }

            prepareHtml_(doc);
            writeHtml_(directory + QDir::separator() + link, doc);
        }
        else htmls_.pop_front();
    }

    writeImages_(directory);
}

void HelpExporterHtml::prepareHtml_(QDomDocument &doc)
{
    // search for links
    gatherLinks_(doc.documentElement());
    gatherImages_(doc.documentElement());
}

void HelpExporterHtml::writeHtml_(const QString &filename, const QDomDocument &doc)
{
    MO_DEBUG_HELP("HelpExporterHtml::writeHtml_('" << filename << "')");

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "Can't create file '" << filename << "'\n"
                    << file.errorString());

    QTextStream stream(&file);
    doc.save(stream, 0);
}

void HelpExporterHtml::gatherLinks_(const QDomElement &e)
{
    if (!e.isElement())
        return;

    if (e.tagName() == "a")
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

void HelpExporterHtml::getPotentialLink_(const QDomElement & e)
{
    QString href = e.attribute("href");
    if (href.isEmpty())
        return;

    MO_DEBUG_HELP("HelpExporterHtml: found href '" << href << "'");

    if (href.startsWith("#") || href.startsWith("http"))
        return;

    // strip anchor
    int idx = href.indexOf("#");
    if (idx>0)
        href = href.left(idx);

    // add to todo-list
    auto it = htmlsExp_.find(href);
    if (it == htmlsExp_.end())
    {
        htmlsExp_.insert(href, false);
        htmls_.push_back(href);
    }
}

void HelpExporterHtml::gatherImages_(const QDomElement &e)
{
    if (!e.isElement())
        return;

    if (e.tagName() == "img")
    {
        QString src = e.attribute("src");
        MO_DEBUG_HELP("HelpExporterHtml: found image, src='" << src << "'");

        if (!src.isEmpty())
        {
            imgs_.push_back(src);
            imgsExp_.insert(src, false);
        }
    }

    // traverse childs
    QDomNodeList list = e.childNodes();
    for (int i=0; i<list.count(); ++i)
    {
        QDomElement ce = list.item(i).toElement();
        if (ce.isElement())
            gatherImages_(ce);
    }
}

void HelpExporterHtml::writeImages_(const QString &directory)
{
    while (imgs_.begin() != imgs_.end())
    {
        const QString link = imgs_.front();
        imgs_.pop_front();

        // not saved?
        if (!imgsExp_[link])
        {
            MO_DEBUG_HELP("HelpExporterHtml: writing image '" << link << "'");
            // remove from todolist
            imgsExp_[link] = true;

            QVariant v = help_->loadResource(link, HelpSystem::ImageResource);
            if (v.isNull())
            {
                MO_IO_WARNING(READ, "Could not get image '" << link << "' "
                              "from HelpSystem");
                continue;
            }

            QImage img = v.value<QImage>();
            img.save(directory + QDir::separator() + link);
        }
    }

}

} // namespace MO
