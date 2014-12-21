/** @file abstractscriptwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
#define MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H

#include <QWidget>

class QSyntaxHighlighter;

namespace MO {
namespace GUI {


class AbstractScriptWidget : public QWidget
{
    Q_OBJECT
public:

    enum MessageType
    {
        M_INFO,
        M_WARNING,
        M_ERROR
    };


    explicit AbstractScriptWidget(QWidget *parent = 0);
    ~AbstractScriptWidget();

    // -------------- getter -------------------------------

    const QString scriptText() const;

    /** Returns true when the current text has been successfully compiled. */
    bool isScriptValid() const;

    // ------------------ actions --------------------------
public slots:

    void setScriptText(const QString&);

    /** Call this to install or update a syntax highlighter.
        Call this even if you have installed it already instead of
        QSyntaxHighlighter::rehighlight() because this function
        avoids the internal textChanged() signal. */
    void setSyntaxHighlighter(QSyntaxHighlighter * );

signals:

    /** Only emitted when the changed script is valid */
    void scriptTextChanged();

    // -------------- protected interface ------------------
protected:

    /** Override to compile the script and check for errors */
    virtual bool compile() = 0;

    /** Emit this from compile().
        @p line is zero-based */
    void addCompileMessage(int line, MessageType t, const QString & text);

private:
    class PrivateSW;
    PrivateSW * p_sw_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
