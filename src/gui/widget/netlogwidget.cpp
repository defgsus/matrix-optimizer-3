/** @file netlogwidget.cpp

    @brief Log display for NetworkLogger

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include "netlogwidget.h"
#include "network/netlog.h"

namespace MO {
namespace GUI {


NetLogWidget::NetLogWidget(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setReadOnly(true);

    connect(&NetworkLogger::instance(), SIGNAL(textAdded(int,QString)),
            this, SLOT(addLine_(int,QString)));
}

void NetLogWidget::addLine_(int level, const QString & text)
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
            s += "#a00";
            break;

        case NetworkLogger::APP_ERROR:
        case NetworkLogger::ERROR:
            s += "#f00";
            break;

        case NetworkLogger::EVENT:
            s += "#000";
            break;

        case NetworkLogger::EVENT_V2:
            s += "#333";
            break;
    }

    s += "\">" + text + "</font>";

    appendHtml(s);
}


} // namespace GUI
} // namespace MO
