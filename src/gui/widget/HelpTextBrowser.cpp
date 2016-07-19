/** @file helptextbrowser.cpp

    @brief QTextBrowser with reimplemented loadResource() to work with resource files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QTextStream>
#include <QDomDocument>

#include "HelpTextBrowser.h"
#include "io/log.h"
#include "io/error.h"
#include "io/HelpSystem.h"

namespace MO {
namespace GUI {


HelpTextBrowser::HelpTextBrowser(QWidget *parent) :
    QTextBrowser    (parent),
    help_           (new HelpSystem(this))
{
    setOpenExternalLinks(true);
}


QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::ImageResource)
        return help_->loadResource(url.url(), HelpSystem::ImageResource);

    if (type == QTextDocument::StyleSheetResource)
        return help_->loadResource(url.url(), HelpSystem::StyleSheetResource);

    if (type == QTextDocument::HtmlResource)
    {
        QDomDocument doc;
        if (!help_->loadXhtml(url.url(), doc))
            return tr("<h3>%1 is missing</h3>").arg(url.url());

        QString html;
        QTextStream stream(&html);
        doc.save(stream, 0);

        return html;
    }

    return QTextBrowser::loadResource(type, url);
}


} // namespace GUI
} // namespace MO
