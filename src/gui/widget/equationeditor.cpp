/** @file equationeditor.cpp

    @brief Text editor with highlighting for MATH::Parser

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#include <vector>
#include <string>


#include <QDebug>
#include <QPalette>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTimer>
#include <QKeyEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QInputDialog>

#include "equationeditor.h"
#include "tool/syntaxhighlighter.h"
#include "math/funcparser/parser.h"
#include "gui/helpdialog.h"
#include "io/equationpreset.h"
#include "io/equationpresets.h"
#include "io/files.h"
#include "io/error.h"

namespace MO {
namespace GUI {


EquationEditor::EquationEditor(QWidget *parent) :
    QPlainTextEdit  (parent),
    highlighter_    (new SyntaxHighlighter(document())),
    completer_      (0),
    parser_         (0),
    extParser_      (0),
    timer_          (new QTimer(this)),
    ok_             (false)
{
    // --- setup palette ---
    QPalette p(palette());
    p.setColor(QPalette::Base, QColor(50,50,50));
    p.setColor(QPalette::Text, QColor(200,200,200));
    setPalette(p);

    // --- setup font ---
    QFont f("Monospace");
    f.setStyleHint(QFont::Monospace);
    setFont(f);
    setTabStopWidth(QFontMetrics(font()).width("    "));

    // ---- edit logic ----

    connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged_()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorChanged_()));

    timer_->setSingleShot(true);
    timer_->setInterval(200); /* some interval for fast writers */
    connect(timer_, SIGNAL(timeout()), this, SLOT(checkEquation_()));

    // ---- context menu ----

    createMenus_();
}

EquationEditor::~EquationEditor()
{
    if (parser_)
        delete parser_;
}

void EquationEditor::setOkState_(bool ok)
{
    ok_ = ok;

    QPalette p(palette());
    p.setColor(QPalette::Base, ok_? QColor(50,50,50) : QColor(60,30,30));
    setPalette(p);

}

void EquationEditor::setParser(const PPP_NAMESPACE::Parser * parser )
{
    extParser_ = parser;
    if (!parser)
        return;

    if (!parser_)
        parser_ = new PPP_NAMESPACE::Parser();

    // copy namespace
    parser_->variables() = parser->variables();
    parser_->functions() = parser->functions();

    createCompleter_();
}

void EquationEditor::addVariables(const QStringList &variables)
{
    if (!parser_)
        parser_ = new PPP_NAMESPACE::Parser();

    for (auto &v : variables)
        parser_->variables().add(v.toStdString(), (PPP_NAMESPACE::Float)0);

    createCompleter_();
}

void EquationEditor::createCompleter_()
{
    // extract name lists

    // XXX highlighter currently doesn't like operator chars
    const QRegExp filter("^[a-z,A-Z,_]*$");

    std::vector<std::string> vec = parser_->functions().functionNames();
    QStringList funcNames;
    for (auto &i : vec)
    {
        const QString s = QString::fromStdString(i);
        if (s.contains(filter))
            funcNames.append(s);
    }
    vec = parser_->variables().variableNames();
    QStringList varNames;
    for (auto &i : vec)
    {
        const QString s = QString::fromStdString(i);
        if (s.contains(filter))
        varNames.append(s);
    }

    // update highlighter
    highlighter_->setNames(varNames, funcNames);

    // create completer

    funcNames << varNames;
    funcNames.sort(Qt::CaseInsensitive);

    if (completer_)
        completer_->deleteLater();

    completer_ = new QCompleter(funcNames, this);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    completer_->setCompletionMode(QCompleter::PopupCompletion);
    completer_->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer_->setWrapAround(true);
    completer_->setWidget(this);
    connect(completer_, SIGNAL(activated(QString)), this, SLOT(insertCompletion_(QString)));

    highlighter_->rehighlight();
}

void EquationEditor::onTextChanged_()
{
    if (!parser_)
        return;

    // get the word under cursor
    QTextCursor c = textCursor();
    c.select(QTextCursor::WordUnderCursor);
    QString word = c.selectedText();
    // and test for auto-completion
    if (!word.isEmpty())
        performCompletion_(word);

    // trigger parsing
    timer_->start();
}

void EquationEditor::checkEquation_()
{
    if (!parser_)
        return;

    setOkState_( parser_->parse(toPlainText().toStdString()) );

    if (ok_)
        emit equationChanged();
}

void EquationEditor::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        QString url = "equation.html";

        auto c = textCursor();
        c.select(QTextCursor::WordUnderCursor);
        if (!c.selectedText().isEmpty())
            url += "#" + c.selectedText();
        HelpDialog::run(url);

        e->accept();
        return;
    }

    // Let Enter accept the current auto-complete suggestion
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        && completer_ && completer_->popup()->isVisible()
        // XXX duh, completer_->currendIndex() stays zero !?!
        //&& !completer_->currentCompletion().isEmpty()
        )
    {
        // .. so we read from the popup
        QString word = completer_->popup()->model()->data(
                completer_->popup()->currentIndex()).toString();

        if (!word.isEmpty())
        {
            completer_->popup()->hide();
            e->accept();
            //insertCompletion_(completer_->currentCompletion());
            insertCompletion_(word);
        }
    }
    else
        QPlainTextEdit::keyPressEvent(e);
}

void EquationEditor::onCursorChanged_()
{
    QTextCursor c = textCursor();
    // hide the auto-completer
    if (completer_ && completer_->popup()->isVisible())
        completer_->popup()->hide();
}


void EquationEditor::insertCompletion_(const QString &word)
{
    qDebug() << "current" << word;
    QTextCursor c = textCursor();
    // characters left to insert from whole word
    int numChars = word.length() - completer_->completionPrefix().length();
    int pos = c.position();
    c.insertText(word.right(numChars));

    // select the inserted characters
    c.setPosition(pos);
    c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

    setTextCursor(c);
}

void EquationEditor::performCompletion_(const QString &word)
{
    // scan the strings for matches
    completer_->setCompletionPrefix(word);

    // if match
    if (completer_->completionCount() > 0)
    {
        // break if the only word was already fully written by user
        if (completer_->completionCount() == 1 &&
            completer_->currentCompletion() == word)
            return;

        // get the rectangle for the completion popup
        QRect rect = cursorRect();
        rect.setWidth(completer_->popup()->sizeHintForColumn(0)
                    + completer_->popup()->verticalScrollBar()->sizeHint().width() );
        // select first entry
        completer_->popup()->setCurrentIndex(
                    completer_->completionModel()->index(0,0) );
        // show popup
        completer_->complete(rect);
    }
}

void EquationEditor::createMenus_()
{
    QAction * a;

    contextMenu_ = createStandardContextMenu();
    contextMenu_->addSeparator();

    contextMenu_->addMenu( presetLoadMenu_ = new QMenu(contextMenu_) );
    presetLoadMenu_->setTitle(tr("Load equation"));
    connect(presetLoadMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(loadEquation_(QAction*)));

    contextMenu_->addMenu( presetSaveMenu_ = new QMenu(contextMenu_) );
    presetSaveMenu_->setTitle(tr("Save equation into"));
    connect(presetSaveMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(saveEquation_(QAction*)));

    a = new QAction(tr("Save into new preset"), contextMenu_);
    connect(a, SIGNAL(triggered()), this, SLOT(saveEquationAs_()));
    contextMenu_->addAction( a );


    updatePresetMenu_();
}

void EquationEditor::updatePresetMenu_()
{
    presetLoadMenu_->clear();
    presetSaveMenu_->clear();

    IO::EquationPresets pres;
    for (int i=0; i<pres.count(); ++i)
    {
        IO::EquationPreset * p = pres.preset(i);

        QAction * a = new QAction(p->name(), presetSaveMenu_);
        a->setData(p->filename());
        presetSaveMenu_->addAction(a);

        QMenu * menu = new QMenu(p->name(), presetLoadMenu_);
        presetLoadMenu_->addMenu(menu);

        for (int j=0; j<p->count(); ++j)
        {
            a = new QAction(p->equationName(j), menu);
            menu->addAction( a );
            a->setData(p->equation(j));
        }
    }
}

void EquationEditor::contextMenuEvent(QContextMenuEvent * e)
{
    contextMenu_->popup(e->globalPos());
}

void EquationEditor::saveEquationAs_()
{
    QString filename = IO::Files::getSaveFileName(IO::FT_EQUATION_PRESET, this, false, false);
    if (filename.isEmpty())
        return;

    QString presetName =
            QInputDialog::getText(this, tr("input preset name"),
                tr("Name of the preset collection"),
                QLineEdit::Normal, tr("new"));

    QString equName =
            QInputDialog::getText(this, tr("input equation name"),
                tr("Name of the equation"));

    IO::EquationPreset p;
    p.setName(presetName);
    p.setEquation(equName, toPlainText());
    try
    {
        p.save(filename);
        updatePresetMenu_();
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("saving equation"),
                tr("Sorry but saving the equation preset failed\n%1").arg(e.what()));
    }
}

void EquationEditor::loadEquation_(QAction * a)
{
    setPlainText(a->data().toString());
}

void EquationEditor::saveEquation_(QAction * a)
{
    QString presName = a->data().toString();

    IO::EquationPresets pres;

ask_again:
    QString equName =
            QInputDialog::getText(this,
                tr("input equation name"),
                tr("Name of the equation"));

    IO::EquationPreset * p = 0;
    for (int i=0; i<pres.count(); ++i)
    {
        if (pres.preset(i)->filename() == presName)
        {
            p = pres.preset(i);
            break;
        }
    }
    if (!p)
    {
        MO_WARNING("preset file '"
                   << presName << "' not found, aborting save");
        return;
    }

    if (p->hasEquation(equName))
    {
        QMessageBox::Button res =
        QMessageBox::question(this, tr("confirm overwrite"),
            tr("Preset collection <b>%1</b> already has an equation called <b>%2</b>."
               "<br/>Do you want to overwrite it?").arg(presName).arg(equName),
              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
              QMessageBox::No);
        if (res == QMessageBox::No)
            goto ask_again;
        if (res == QMessageBox::Cancel)
            return;
    }

    p->setEquation(equName, toPlainText());
    try
    {
        p->save();
        updatePresetMenu_();
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("saving equation"),
                tr("Sorry but saving the equation preset failed\n%1").arg(e.what()));
    }
}

} // namespace GUI
} // namespace MO
