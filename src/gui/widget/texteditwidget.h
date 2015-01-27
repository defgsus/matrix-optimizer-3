/** @file texteditwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.01.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_TEXTEDITWIDGET_H
#define MOSRC_GUI_WIDGET_TEXTEDITWIDGET_H

#include <QWidget>

#include "object/object_fwd.h" // for TextType

namespace MO {
namespace GUI {


/** This class is essentially like TextEditDialog but without the dialog */
class TextEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditWidget(TextType textType, QWidget *parent = 0);
    explicit TextEditWidget(const QString& text, TextType textType, QWidget *parent = 0);
    ~TextEditWidget();

    // ------------ getter ---------------

    TextType getTextType() const;

    QString getText() const;

    // ------------ setter ---------------

    /** Sets the text that is currently edited.
        When the Dialog is cancelled, this text will be restored */
    void setText(const QString&, bool send_signal = false);

    /** Give a list of variables to the equation editor */
    void addVariableNames(const QStringList&);

    /** Give a list of variables with their descriptions to the equation editor */
    void addVariableNames(const QStringList& names, const QStringList& descriptions);

signals:

    /** The text was changed by the user.
        If getTextType() is TT_EQUATION or TT_ANGELSCRIPT, this signal only emits when
        the equation actually compiles without errors. */
    void textChanged();

    /** Emitted when the widget wants to disappear because of user-says-so */
    void closeRequest();

public slots:

    void openHelp();

    /** Implemented for scripts */
    void addErrorMessage(int line, const QString & text);

protected:

    void keyPressEvent(QKeyEvent *);

private:

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TEXTEDITWIDGET_H
