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

    /** Returns wheter the current text is a valid expression */
    bool isOk() const { return ok_; }

signals:

    /** This signal will be emitted when the current entered equation
        parses correctly. */
    void equationChanged();

public slots:

    /** Assigns the Parser class to the editor for syntax highlighting.
        The given parser will not be modified but instead the namespace
        is copied to an internal parser object.
        All previous content will be lost. */
    void setParser(const PPP_NAMESPACE::Parser * parser);

    /** Adds the variable names to the internal parser.
        All variables will be initialized to 0 */
    void addVariables(const QStringList& variables);

protected slots:

    void onTextChanged_();
    void onCursorChanged_();
    void checkEquation_();
    void insertCompletion_(const QString &word);
    void saveEquationAs_();
    void saveEquation_(QAction*);
    void loadEquation_(QAction*);

protected:

    void keyPressEvent(QKeyEvent *);
    void contextMenuEvent(QContextMenuEvent *e);

private:
    void createMenus_();
    void updatePresetMenu_();
    void createCompleter_();
    void setOkState_(bool isOk);
    void performCompletion_(const QString &word);

    SyntaxHighlighter * highlighter_;
    QCompleter * completer_;

    PPP_NAMESPACE::Parser * parser_;
    const PPP_NAMESPACE::Parser * extParser_;

    QTimer * timer_;
    bool ok_;

    QMenu * contextMenu_, *presetLoadMenu_, *presetSaveMenu_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_EQUATIONEDITOR_H
