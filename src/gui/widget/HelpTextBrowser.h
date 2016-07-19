/** @file helptextbrowser.h

    @brief QTextBrowser with reimplemented loadResource() to work with resource files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_HELPTEXTBROWSER_H
#define MOSRC_GUI_WIDGET_HELPTEXTBROWSER_H

#include <QTextBrowser>

namespace MO {
class HelpSystem;
namespace GUI {

class HelpTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit HelpTextBrowser(QWidget *parent = 0);

    QVariant loadResource(int type, const QUrl &name) Q_DECL_OVERRIDE;

private:

    HelpSystem * help_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_HELPTEXTBROWSER_H
