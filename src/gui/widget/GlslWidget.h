/** @file glslwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_GLSLWIDGET_H
#define MOSRC_GUI_WIDGET_GLSLWIDGET_H

#include "AbstractScriptWidget.h"

namespace MO {
namespace GUI {


class GlslWidget : public AbstractScriptWidget
{
    Q_OBJECT
public:
    explicit GlslWidget(QWidget *parent = 0);

signals:

public slots:

protected:

    bool compile() Q_DECL_OVERRIDE;

    QString getHelpUrl(const QString& token) const Q_DECL_OVERRIDE;

private:
    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_GLSLWIDGET_H
