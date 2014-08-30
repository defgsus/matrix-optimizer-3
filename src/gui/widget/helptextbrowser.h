/** @file helptextbrowser.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#ifndef HELPTEXTBROWSER_H
#define HELPTEXTBROWSER_H

#include <QTextBrowser>

namespace MO {
namespace GUI {

class HelpTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit HelpTextBrowser(QWidget *parent = 0);

    QVariant loadResource(int type, const QUrl &name) Q_DECL_OVERRIDE;

};

} // namespace GUI
} // namespace MO

#endif // HELPTEXTBROWSER_H
