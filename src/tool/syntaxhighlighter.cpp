/** @file syntaxhighlighter.cpp

    @brief Generic syntax highlighter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#include "syntaxhighlighter.h"

namespace MO {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    // setup multiline comments
    commentStartExpression_ = QRegExp("/\\*");
    commentEndExpression_ = QRegExp("\\*/");
    commentFormat_.setForeground(QBrush(QColor(140,140,140)));
}

void SyntaxHighlighter::setNames(const QStringList &variables, const QStringList &functions)
{
    rules_.clear();

    HighlightingRule rule;

    // setup single line comments
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = commentFormat_;
    rules_.append(rule);

    // -- styles for each category --

    QTextCharFormat
        functionFormat,
        variableFormat;

    // functions
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(QBrush(QColor(200,200,220)));
    // variables
    variableFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(QBrush(QColor(200,220,200)));

    // create rules for each reserved word

    for (auto &k : variables)
    {
        // match whole-word with word boundaries
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = variableFormat;
        rules_.append(rule);
    }

    for (auto &k : functions)
    {
        rule.pattern = QRegExp( "\\b" + k + "\\b" );
        rule.format = functionFormat;
        rules_.append(rule);
    }
}


void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // apply rules
    foreach (const HighlightingRule &rule, rules_)
    {
        QRegExp expression(rule.pattern);
        // find the string
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            // set format
            setFormat(index, length, rule.format);
            // find again
            index = expression.indexIn(text, index + length);
        }
    }


    // --- multiline comments ---

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression_.indexIn(text);

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression_.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                            + commentEndExpression_.matchedLength();
        }
        setFormat(startIndex, commentLength, commentFormat_);
        startIndex = commentStartExpression_.indexIn(text, startIndex + commentLength);
    }
}



} // namespace MO
