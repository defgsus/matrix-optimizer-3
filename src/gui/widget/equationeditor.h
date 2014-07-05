/** @file equationeditor.h

    @brief Text editor with highlighting for MATH::Parser

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_EQUATIONEDITOR_H
#define MOSRC_GUI_WIDGET_EQUATIONEDITOR_H

#include <QPlainTextEdit>

class QCompleter;
class QTimer;

namespace PPP_NAMESPACE { class Parser; }
namespace MO {
class SyntaxHighlighter;
namespace GUI {


class EquationEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit EquationEditor(QWidget *parent = 0);
    ~EquationEditor();

    const PPP_NAMESPACE::Parser * assignedParser() const { return extParser_; }

    bool ok() const { return ok_; }

signals:

    /** This signal will be emitted when the current entered equation
        parses correctly. */
    void equationChanged();

public slots:

    /** Assigns the Parser class to the editor for syntax highlighting.
        The given parser will not be modified but instead the namespace
        is copied to an internal parser object. */
    void setParser(const PPP_NAMESPACE::Parser * parser);

protected slots:

    void onTextChanged_();
    void onCursorChanged_();
    void checkEquation_();
    void insertCompletion_(const QString &word);

protected:

    void keyPressEvent(QKeyEvent *);

private:

    void setOkState_(bool ok);
    void performCompletion_(const QString &word);

    SyntaxHighlighter * highlighter_;
    QCompleter * completer_;

    PPP_NAMESPACE::Parser * parser_;
    const PPP_NAMESPACE::Parser * extParser_;

    QTimer * timer_;
    bool ok_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_EQUATIONEDITOR_H
