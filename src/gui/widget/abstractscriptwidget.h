/** @file abstractscriptwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
#define MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H

#include <QWidget>

#include "io/filetypes.h"

class QSyntaxHighlighter;

namespace MO {
namespace GUI {

/** Abstract source code editor.
    Implement compile() and getHelpUrl() in derived classes
    to handle the implemented language */
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


    explicit AbstractScriptWidget(IO::FileType type = IO::FT_TEXT, QWidget *parent = 0);
    ~AbstractScriptWidget();

    // -------------- getter -------------------------------

    const QString scriptText() const;

    /** Returns true when the current text has been successfully compiled. */
    bool isScriptValid() const;
    /** Returns true when the script text has been edited */
    bool isChanged() const;

    IO::FileType fileType() const;

signals:

    /** Only emitted when the changed script is valid */
    void scriptTextChanged();

    // ------------------ actions --------------------------
public slots:

    /** Sets the current script text */
    void setScriptText(const QString&);

    bool loadScript();
    bool saveScript();
    bool saveScriptAs();
    bool loadScript(const QString& fn);
    bool saveScript(const QString& fn);

    /** Checks syntax when script was changed
        and always sends a scriptTextChanged() signal */
    void updateScript();

    /** Call this to install or update a syntax highlighter.
        Call this even if you have installed it already instead of
        QSyntaxHighlighter::rehighlight() because this function
        avoids the internal textChanged() signal. */
    void setSyntaxHighlighter(QSyntaxHighlighter * );

    /** If updates are optional, a checkbox and/or an update button is visible.
        Updates, e.g. scriptTextChanged() signal is then only emitted on user's request.
        Default is false. */
    void setUpdateOptional(bool enable = true);

    /** When true, every textchange triggers a scriptTextChanged() signal.
        Buffered by a few ms. Setting is not reflected in gui (overrides optional updates).
        Default is false. */
    void setAlwaysUpdate(bool enable = true);

    /** Emit this from compile() generally.
        @p line is zero-based */
    void addCompileMessage(int line, MessageType t, const QString & text);

protected:

    /** Returns true if script is unchanged or if user
        chooses to discard changes */
    bool isSaveToChange();

    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;

    // -------------- protected interface ------------------

    /** Override to compile the script and check for errors */
    virtual bool compile() = 0;

    /** Override to return an internal html reference for the type of script.
        Optionally return an anchor using the word under cursor or whatever */
    virtual QString getHelpUrl(const QString& token) const = 0;

private:
    class PrivateSW;
    PrivateSW * p_sw_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ABSTRACTSCRIPTWIDGET_H
