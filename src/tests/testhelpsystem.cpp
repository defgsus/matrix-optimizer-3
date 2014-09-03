/** @file testhelpsystem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "testhelpsystem.h"
#include "io/helpsystem.h"

namespace MO {

TestHelpSystem::TestHelpSystem()
    : help  (new HelpSystem())
{
}

int TestHelpSystem::run()
{
    //qDebug() << help->loadResource("index", HelpSystem::XHtmlResource);

    QDomDocument doc;
    help->loadXhtml("index", doc);

    QString text;
    QTextStream stream(&text);

    help->renderHtml(doc, stream);

    qDebug() << text;

    return 0;
}

} // namespace MO
