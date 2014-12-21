/** @file abstractscriptwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
#define MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H

#include <QWidget>


namespace MO {
namespace GUI {


class AbstractScriptWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractScriptWidget(QWidget *parent = 0);
    ~AbstractScriptWidget();

    // -------------- getter -------------------------------

    const QString scriptText() const;

    /** Returns true when the current text has been successfully compiled. */
    bool isScriptValid() const;

    // ------------------ actions --------------------------
public slots:

    void setScriptText(const QString&);

signals:

    // -------------- protected interface ------------------
protected:

    /** Override to compile the script and check for errors */
    virtual bool compile() = 0;

    /** Emit this from compile() */
    void addScriptError(int line, const QString & text);

private:
    class PrivateSW;
    PrivateSW * p_sw_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
