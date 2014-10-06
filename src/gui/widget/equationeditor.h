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

    /** Adds the variable names and descriptions to the internal parser.
        All variables will be initialized to 0 */
    void addVariables(const QStringList& variables,
                      const QStringList& descriptions);

protected:

    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void leaveEvent(QEvent *);

protected slots:

    void onTextChanged_();
    void onCursorChanged_();
    void onHover_();
    void checkEquation_();
    void insertCompletion_(const QString &word);
    void insertVariable_(QAction*);
    void saveEquationAs_();
    void saveEquation_(QAction*);
    void saveEquation_();
    void loadEquation_(QAction*);
    void insertEquation_(QAction*);

private:
    void createMenus_();
    void updatePresetMenu_();
    void updateVariableMenu_();
    void createCompleter_();
    void setOkState_(bool isOk);
    void performCompletion_(const QString &word);
    void highlight_(bool on);

    SyntaxHighlighter * highlighter_;
    QCompleter * completer_;
    QMap<QString, QString> varDescriptions_;

    PPP_NAMESPACE::Parser * parser_;
    const PPP_NAMESPACE::Parser * extParser_;

    QString presetName_, equationName_;

    QTimer * timer_, * hoverTimer_;
    bool ok_, isHighlight_;

    QMenu
        * contextMenu_,
        * presetLoadMenu_,
        * presetInsertMenu_,
        * presetSaveMenu_,
        * variableMenu_;
    QAction * actSave_;

    // config

    QColor colorBase_, colorText_, colorBaseHl_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_EQUATIONEDITOR_H
