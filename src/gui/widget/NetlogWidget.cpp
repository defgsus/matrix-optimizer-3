/** @file netlogwidget.cpp

    @brief Log display for NetworkLogger

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include "NetlogWidget.h"
#include "network/netlog.h"

namespace MO {
namespace GUI {


NetLogWidget::NetLogWidget(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setReadOnly(true);

    connect(&NetworkLogger::instance(), SIGNAL(textAdded(int,QString)),
            this, SLOT(addLine(int,QString)));
}

void NetLogWidget::addLine(int level, const QString & text)
{
    QString s = "<font color=\"";

    switch (NetworkLogger::Level(level))
    {
        case NetworkLogger::CTOR:
        case NetworkLogger::DEBUG:
            s += "#777";
            break;

        case NetworkLogger::DEBUG_V2:
            s += "#999";
            break;

        case NetworkLogger::APP_WARNING:
        case NetworkLogger::WARNING:
            s += "#faa";
            break;

        case NetworkLogger::APP_ERROR:
        case NetworkLogger::ERROR:
            s += "#f77";
            break;

        case NetworkLogger::EVENT:
            s += "#aaa";
            break;

        case NetworkLogger::EVENT_V2:
            s += "#999";
            break;
    }

    s += "\">" + text + "</font>";

    appendHtml(s);
}


} // namespace GUI
} // namespace MO
