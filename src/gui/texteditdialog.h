/** @file texteditdialog.h

    @brief Editor for text, equations and source-code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#ifndef MOSRC_GUI_TEXTEDITDIALOG_H
#define MOSRC_GUI_TEXTEDITDIALOG_H

#include <QDialog>

#include "object/object_fwd.h" // for TextType

namespace MO {
namespace GUI {

class AbstractScriptWidget;
class AngelScriptWidget;

class TextEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TextEditDialog(TextType textType, QWidget *parent = 0);
    explicit TextEditDialog(const QString& text, TextType textType, QWidget *parent = 0);
    ~TextEditDialog();

    // ------------ getter ---------------

    TextType getTextType() const;

    QString getText() const;

    AbstractScriptWidget * getScriptWidget() const;

    // ------------ setter ---------------

    void setReadOnly(bool readOnly);

    /** Sets the text that is currently edited.
        When the Dialog is cancelled, this text will be restored */
    void setText(const QString&, bool send_signal = false);

    /** Give a list of variables to the equation editor */
    void addVariableNames(const QStringList&);

    /** Give a list of variables with their descriptions to the equation editor */
    void addVariableNames(const QStringList& names, const QStringList& descriptions);

    /** Access to the angelscript widget (if text type was TT_ANGELSCRIPT in constructor) */
    AngelScriptWidget * getWidgetAngelScript() const;

signals:

    /** The text was changed by the user.
        If getTextType() is TT_EQUATION, this signal only emits when
        the equation actually compiles without errors. */
    void textChanged();

public slots:

    void openHelp();

    /** Implemented for scripts.
        Posts messages into the script edit window when open. */
    void addErrorMessage(int line, const QString & text);

protected:

    void keyPressEvent(QKeyEvent *);

private:

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TEXTEDITDIALOG_H
