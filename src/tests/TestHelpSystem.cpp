/** @file testhelpsystem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/3/2014</p>
*/

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>

#include "TestHelpSystem.h"
#include "io/HelpSystem.h"
#include "io/HelpExporterHtml.h"
#include "io/HelpExporterLatex.h"

namespace MO {

TestHelpSystem::TestHelpSystem()
    : help  (new HelpSystem())
{
}

int TestHelpSystem::run()
{
    //qDebug() << help->loadResource("index", HelpSystem::XHtmlResource);
    /*
    QDomDocument doc;
    help->loadXhtml("index", doc);

    QString text;
    QTextStream stream(&text);

    help->renderHtml(doc, stream);

    qDebug() << text;
    */

//    HelpExporterHtml exp;
//    exp.save("/home/defgsus/prog/qt_project/mo/matrixoptimizer/help_export");

    HelpExporterLatex exp;
    exp.save("/home/defgsus/prog/qt_project/mo/matrixoptimizer/help_export/latex");

    return 0;
}

} // namespace MO
