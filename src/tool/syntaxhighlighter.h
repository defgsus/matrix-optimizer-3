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

namespace MO {



class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument * parent);

signals:

public slots:

    /** Set all reserved words */
    void setNames(
            const QStringList& variables,
            const QStringList& functions);

    /** Sets reserved words for qt stylesheet syntax */
    void initForStyleSheet();

protected:

    void highlightBlock(const QString &text);

private:

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> rules_;

    QTextCharFormat commentFormat_;

    QRegExp commentStartExpression_;
    QRegExp commentEndExpression_;

};


} // namespace MO

#endif // MOSRC_TOOL_SYNTAXHIGHLIGHTER_H
