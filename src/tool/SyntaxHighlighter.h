/** @file syntaxhighlighter.h

    @brief Generic syntax highlighter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#ifndef MOSRC_TOOL_SYNTAXHIGHLIGHTER_H
#define MOSRC_TOOL_SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;
#ifndef MO_DISABLE_ANGELSCRIPT
class asIScriptModule;
class asIScriptContext;
#endif

namespace MO {

#ifdef MO_ENABLE_PYTHON34
namespace PYTHON34 { class PythonInterpreter; }
#endif


class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(bool pythonComments, QObject * parent);

signals:

public slots:

    /** Set all reserved words */
    void setNames(
            const QStringList& variables,
            const QStringList& functions,
            const QStringList& types = QStringList(),
            const QStringList& reserved = QStringList());

    /** Sets reserved words for qt stylesheet syntax */
    void initForStyleSheet();

    void initForGlsl();

#ifndef MO_DISABLE_ANGELSCRIPT
    /** Gathers all installed names from the angelscript module */
    void initForAngelScript(asIScriptModule*);
#endif

#ifdef MO_ENABLE_PYTHON34
    void initForPython(const PYTHON34::PythonInterpreter*);
#endif

    /** Sets text to highlight (e.g. for text-finder) */
    void setMarkText(const QString& text);
    void setMarkText(const QStringList& texts);

protected:

    void highlightBlock(const QString &text);

private:

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule>
        rules_, markRules_;

    QTextCharFormat commentFormat_,
                    markFormat_;

    QRegExp commentStartExpression_;
    QRegExp commentEndExpression_;

    bool pythonComments_;
};


} // namespace MO

#endif // MOSRC_TOOL_SYNTAXHIGHLIGHTER_H
