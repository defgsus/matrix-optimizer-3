/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_PYTHON34WIDGET_H
#define MOSRC_GUI_WIDGET_PYTHON34WIDGET_H

#include "abstractscriptwidget.h"

namespace MO {
namespace PYTHON34 { class PythonInterpreter; }
namespace GUI {

class Python34Widget : public AbstractScriptWidget
{
    Q_OBJECT
public:
    explicit Python34Widget(QWidget *parent = 0);
    ~Python34Widget();

signals:

public slots:

    /** Exceptions are cought and displayed as error message */
    void executeScript();

    /** Sets the error messages from the interpreter,
        if there are any. */
    void setErrorFrom(const PYTHON34::PythonInterpreter*);

protected:

    bool compile() Q_DECL_OVERRIDE;

    QString getHelpUrl(const QString& token) const Q_DECL_OVERRIDE;

private:
    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // PYTHON34WIDGET_H

