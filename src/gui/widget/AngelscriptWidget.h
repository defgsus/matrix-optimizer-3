/** @file angelscriptwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_GUI_WIDGET_ANGELSCRIPTWIDGET_H
#define MOSRC_GUI_WIDGET_ANGELSCRIPTWIDGET_H

#include "AbstractScriptWidget.h"

class asIScriptEngine;
class asIScriptModule;

namespace MO {
namespace GUI {


class AngelScriptWidget : public AbstractScriptWidget
{
    Q_OBJECT
public:
    explicit AngelScriptWidget(QWidget *parent = 0);
    ~AngelScriptWidget();

    asIScriptEngine * scriptEngine() const;
    asIScriptModule * scriptModule() const;

signals:

public slots:

    void updateSyntaxHighlighter();

    void setScriptEngine(asIScriptEngine * engine) const;

    /** @throws everything */
    void executeScript();

protected:

    bool compile() Q_DECL_OVERRIDE;

    QString getHelpUrl(const QString& token) const Q_DECL_OVERRIDE;

private:
    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ANGELSCRIPTWIDGET_H

#endif // MO_DISABLE_ANGELSCRIPT
