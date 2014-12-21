/** @file syntaxhighlighter.cpp

    @brief Generic syntax highlighter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT
#include <angelscript.h>
#endif

#include <QDebug>

#include "syntaxhighlighter.h"
//#include "io/log.h"

namespace MO {

SyntaxHighlighter::SyntaxHighlighter(QObject * parent) :
    QSyntaxHighlighter  (parent)
{
    // setup multiline comments
    commentStartExpression_ = QRegExp("/\\*");
    commentEndExpression_ = QRegExp("\\*/");
    commentFormat_.setForeground(QBrush(QColor(140,140,140)));
}

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
    qDebug() << "SyntaxHighligher::setNames:\nvars " << variables << "\nfuncs " << functions;

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





void SyntaxHighlighter::initForStyleSheet()
{
    QStringList vars;
    vars
            << "color"
            << "background-color"
            << "subcontrol-origin"
            << "selection-color"
            << "selection-background-color"
            << "border"
            << "image"
            << "margin"
            << "padding"
            << "position"
            << "top"
            << "left"
               ;

    QStringList selects;
    selects
            << ":hover"
            << ":checked"
            << ":pressed"
            << ":enabled"
            << ":active"
            << ":alternate"
            << "::drop-down"
            << "::up-arrow"
            << "::down-arrow"
            << "::item"
            << "GroupHeader="
               ;

    setNames(vars, selects);
}


#ifndef MO_DISABLE_ANGELSCRIPT

void SyntaxHighlighter::initForAngelScript(asIScriptModule * mod)
{
    QStringList vars, funcs;
#if 0
    for (int i=0; i<ctx->GetVarCount(); ++i)
    {
        vars << QString( ctx->GetVarName(i) );
    }

#else
    for (asUINT i=0; i<mod->GetEnumCount(); ++i)
    {
        vars << QString( mod->GetEnumByIndex(i, 0) );
    }

    for (asUINT i=0; i<mod->GetGlobalVarCount(); ++i)
    {
        const char * name;
        mod->GetGlobalVar(i, &name);
        vars << QString( name );
    }

    for (asUINT i=0; i<mod->GetObjectTypeCount(); ++i)
    {
        funcs << QString( mod->GetObjectTypeByIndex(i)->GetName() );
    }

    for (asUINT i=0; i<mod->GetEngine()->GetObjectTypeCount(); ++i)
    {
        funcs << QString( mod->GetEngine()->GetObjectTypeByIndex(i)->GetName() );
    }


    for (asUINT i=0; i<mod->GetTypedefCount(); ++i)
    {
        funcs << QString( mod->GetTypedefByIndex(i, 0) );
    }

    for (asUINT i=0; i<mod->GetFunctionCount(); ++i)
    {
        funcs << QString( mod->GetFunctionByIndex(i)->GetName() );
    }

    for (asUINT i=0; i<mod->GetEngine()->GetGlobalFunctionCount(); ++i)
    {
        funcs << QString( mod->GetEngine()->GetGlobalFunctionByIndex(i)->GetName() );
    }

#endif
    setNames(vars, funcs);
}

#endif


} // namespace MO
