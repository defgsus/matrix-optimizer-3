/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#ifndef MOSRC_GUI_WIDGET_PYTHON27WIDGET_H
#define MOSRC_GUI_WIDGET_PYTHON27WIDGET_H

#include "abstractscriptwidget.h"

namespace MO {
namespace GUI {

class Python27Widget : public AbstractScriptWidget
{
    Q_OBJECT
public:
    explicit Python27Widget(QWidget *parent = 0);
    ~Python27Widget();

signals:

public slots:

    /* @throws everything */
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

#endif // PYTHON27WIDGET_H

#endif // MO_ENABLE_PYTHON27
