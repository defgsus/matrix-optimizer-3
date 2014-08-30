/** @file helptextbrowser.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QTextStream>
#include <QFile>

#include "helptextbrowser.h"


namespace MO {
namespace GUI {


HelpTextBrowser::HelpTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
}

QVariant HelpTextBrowser::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::HtmlResource)
    {
        const QString name = url.url();
        const QString fn = ":/help/"
            + (name.endsWith(".html")? name : name + ".html");

        QFile f(fn);
        if (!f.open(QFile::ReadOnly))
        {
            return tr("<h3>%1 is missing</h3>").arg(fn);
        }

        QTextStream s(&f);
        return s.readAll();
    }

    if (type == QTextDocument::ImageResource)
    {

    }

    return QTextBrowser::loadResource(type, url);
}


} // namespace GUI
} // namespace MO
