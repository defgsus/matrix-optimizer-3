/** @file angelscriptwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef ANGELSCRIPTWIDGET_H
#define ANGELSCRIPTWIDGET_H

#include "abstractscriptwidget.h"


namespace MO {
namespace GUI {


class AngelScriptWidget : public AbstractScriptWidget
{
    Q_OBJECT
public:
    explicit AngelScriptWidget(QWidget *parent = 0);
    ~AngelScriptWidget();

signals:

public slots:

protected:

    bool compile() Q_DECL_OVERRIDE;

private:
    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // ANGELSCRIPTWIDGET_H

#endif // MO_DISABLE_ANGELSCRIPT
