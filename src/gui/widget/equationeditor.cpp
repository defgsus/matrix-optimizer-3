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
#include <QToolTip>

#include "equationeditor.h"
#include "tool/syntaxhighlighter.h"
#include "math/funcparser/parser.h"
#include "gui/helpdialog.h"
#include "io/equationpreset.h"
#include "io/equationpresets.h"
#include "io/files.h"
#include "io/error.h"
#include "gui/saveequationdialog.h"

namespace MO {
namespace GUI {


EquationEditor::EquationEditor(QWidget *parent) :
    QPlainTextEdit  (parent),
    highlighter_    (new SyntaxHighlighter(document())),
    completer_      (0),
    parser_         (0),
    extParser_      (0),
    timer_          (new QTimer(this)),
    hoverTimer_     (new QTimer(this)),
    ok_             (false),
    isHighlight_    (false)
{
    // --- setup palette ---

    colorBase_ = QColor(50,50,50);
    colorBaseHl_ = QColor(50,100,50);
    colorText_ = QColor(200,200,200);

    QPalette p(palette());
    p.setColor(QPalette::Base, colorBase_);
    p.setColor(QPalette::Text, colorText_);
    setPalette(p);

    // --- setup font ---

    // XXX does not work for Mac
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

    setMouseTracking(true);
    hoverTimer_->setSingleShot(true);
    hoverTimer_->setInterval(500);
    connect(hoverTimer_, SIGNAL(timeout()), this, SLOT(onHover_()));

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

    // get variable descriptions
    std::vector<PPP_NAMESPACE::Variable*> vars;
    parser_->variables().getVariables(vars, false);

    for (auto v : vars)
    {
        if (!v->description().empty())
            varDescriptions_[QString::fromStdString(v->name())]
                    = QString::fromStdString(v->description());
    }

    createCompleter_();
    updateVariableMenu_();
}

void EquationEditor::addVariables(const QStringList &variables)
{
    if (!parser_)
        parser_ = new PPP_NAMESPACE::Parser();

    for (auto &v : variables)
        parser_->variables().add(
                    v.toStdString(), (PPP_NAMESPACE::Float)0, "");

    createCompleter_();
    updateVariableMenu_();
}

void EquationEditor::addVariables(
        const QStringList &variables,
        const QStringList &descriptions)
{
    MO_ASSERT(descriptions.size() >= variables.size(),
              "descriptions and variables size not matching "
              << descriptions.size() << ":" << variables.size());

    int k = 0;
    for (auto &v : variables)
        varDescriptions_[v] = descriptions[k++];

    if (!parser_)
        parser_ = new PPP_NAMESPACE::Parser();

    for (auto &v : variables)
        parser_->variables().add(
                    v.toStdString(), (PPP_NAMESPACE::Float)0, "");

    createCompleter_();
    updateVariableMenu_();
}

void EquationEditor::highlight_(bool on)
{
    QPalette p(palette());
    p.setColor(QPalette::Base, on ? colorBaseHl_ : colorBase_);
    setPalette(p);

    if (on)
        timer_->start();

    isHighlight_ = on;
}

void EquationEditor::createCompleter_()
{
    // extract name lists

    // XXX highlighter currently doesn't like operator chars
    const QRegExp filter("^[a-z,A-Z,0-9,_]*$");

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

    // ignore '(' and get the left word of it
    if (word.startsWith('('))
    {
        // for some reason we need to move two chars
        c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        c.select(QTextCursor::WordUnderCursor);
        word = c.selectedText();
    }

    // and test for auto-completion
    if (!word.isEmpty())
        performCompletion_(word);


    // trigger parsing
    timer_->start();
}

void EquationEditor::checkEquation_()
{
    if (isHighlight_)
    {
        highlight_(false);
    }

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
    //QTextCursor c = textCursor();

    // hide the auto-completer
    if (completer_ && completer_->popup()->isVisible())
        completer_->popup()->hide();
}

void EquationEditor::leaveEvent(QEvent *)
{
    // hide the auto-completer
    if (completer_ && completer_->popup()->isVisible())
        completer_->popup()->hide();
}

void EquationEditor::insertCompletion_(const QString &word)
{
    QTextCursor c = textCursor();
    // characters left to insert from whole word
    int numChars = word.length() - completer_->completionPrefix().length();
    //int pos = c.position();
    c.insertText(word.right(numChars));

    // select the inserted characters
    //c.setPosition(pos);
    //c.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

    setTextCursor(c);
}

void EquationEditor::insertVariable_(QAction * a)
{
    QString varname = a->data().toString();

    QTextCursor c = textCursor();
    c.insertText(varname);
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

void EquationEditor::mousePressEvent(QMouseEvent * e)
{
    setToolTip("");
    // restart the timer on click
    hoverTimer_->start();
    QPlainTextEdit::mousePressEvent(e);
}

void EquationEditor::mouseMoveEvent(QMouseEvent * e)
{
    setToolTip("");
    // restart the timer whenever mouse moves
    hoverTimer_->start();
    QPlainTextEdit::mouseMoveEvent(e);
}

void EquationEditor::onHover_()
{
    // get mouse position on widget
    QPoint pos = mapFromGlobal(QCursor::pos());
    if (!rect().contains(pos))
        return;

    // adjust a littlebit so we get the center of a letter
    pos.setX(pos.x() - QFontMetrics(font()).width(" ") / 2);

    // find word under cursor
    QTextCursor c = cursorForPosition(pos);
    c.select(QTextCursor::WordUnderCursor);
    QString word = c.selectedText();

    // check for variable
    auto i = varDescriptions_.find(word);
    if (i != varDescriptions_.end())
    {
        setToolTip(QString("<b>%1</b> - %2")
                   .arg(word).arg(i.value()));
    }
}


void EquationEditor::createMenus_()
{
    QAction * a;

    contextMenu_ = createStandardContextMenu();

    contextMenu_->addSeparator();

    contextMenu_->addMenu( variableMenu_ = new QMenu(contextMenu_) );
    variableMenu_->setTitle(tr("insert variable"));
    connect(variableMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(insertVariable_(QAction*)));

    contextMenu_->addSeparator();

    contextMenu_->addMenu( presetLoadMenu_ = new QMenu(contextMenu_) );
    presetLoadMenu_->setTitle(tr("Load equation"));
    connect(presetLoadMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(loadEquation_(QAction*)));

    contextMenu_->addMenu( presetInsertMenu_ = new QMenu(contextMenu_) );
    presetInsertMenu_->setTitle(tr("Insert equation"));
    connect(presetInsertMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(insertEquation_(QAction*)));

    contextMenu_->addSeparator();

    contextMenu_->addAction(a = new QAction(tr("Save equation"), contextMenu_));
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(saveEquation_()));
    actSave_ = a;

    contextMenu_->addMenu( presetSaveMenu_ = new QMenu(contextMenu_) );
    presetSaveMenu_->setTitle(tr("Save equation into"));
    connect(presetSaveMenu_, SIGNAL(triggered(QAction*)),
            this, SLOT(saveEquation_(QAction*)));

    a = new QAction(tr("Save into new preset"), contextMenu_);
    connect(a, SIGNAL(triggered()), this, SLOT(saveEquationAs_()));
    contextMenu_->addAction( a );

    updateVariableMenu_();
    updatePresetMenu_();
}

void EquationEditor::updatePresetMenu_()
{
    presetLoadMenu_->clear();
    presetInsertMenu_->clear();
    presetSaveMenu_->clear();

    if (!presetName_.isEmpty() && !equationName_.isEmpty())
    {
        actSave_->setEnabled(true);
        actSave_->setText(tr("Save %1:%2").arg(presetName_).arg(equationName_));
    }
    else
    {
        actSave_->setEnabled(false);
        actSave_->setText(tr("Save equation"));
    }

    IO::EquationPresets pres;
    for (int i=0; i<pres.count(); ++i)
    {
        IO::EquationPreset * p = pres.preset(i);

        // save
        QAction * a = new QAction(p->name(), presetSaveMenu_);
        a->setData(p->name());
        presetSaveMenu_->addAction(a);

        // load
        QMenu * menu = new QMenu(p->name(), presetLoadMenu_);
        presetLoadMenu_->addMenu(menu);

        for (int j=0; j<p->count(); ++j)
        {
            a = new QAction(p->equationName(j), menu);
            menu->addAction( a );
            a->setData(p->equation(j));

            // XXX not displayed :(
            // a->setToolTip(p->equation(j));
        }

        // insert
        menu = new QMenu(p->name(), presetInsertMenu_);
        presetInsertMenu_->addMenu(menu);

        for (int j=0; j<p->count(); ++j)
        {
            a = new QAction(p->equationName(j), menu);
            menu->addAction( a );
            a->setData(p->equation(j));
        }
    }

    presetLoadMenu_->setEnabled(pres.count() > 0);
    presetInsertMenu_->setEnabled(pres.count() > 0);
    presetSaveMenu_->setEnabled(pres.count() > 0);
}

void EquationEditor::updateVariableMenu_()
{
    variableMenu_->clear();

    if (!parser_)
        return;

    std::vector<PPP_NAMESPACE::Variable*> vars;
    std::sort(vars.begin(), vars.end());

    parser_->variables().getVariables(vars, false);

    for (PPP_NAMESPACE::Variable * v : vars)
    {
        QAction * a = new QAction(variableMenu_);

        variableMenu_->addAction(a);

        QString varname = QString::fromStdString(v->name());
        // some name for the action
        a->setText(varname);

        // the definite name as data field
        a->setData(varname);

        // description as tooltip
        auto it = varDescriptions_.find(varname);
        if (it != varDescriptions_.end())
            a->setToolTip(it.value());
    }
}

void EquationEditor::contextMenuEvent(QContextMenuEvent * e)
{
    contextMenu_->popup(e->globalPos());
}


void EquationEditor::loadEquation_(QAction * a)
{
    setPlainText(a->data().toString());
}

void EquationEditor::insertEquation_(QAction * a)
{
    QTextCursor c(textCursor());
    c.insertText(a->data().toString());
    setTextCursor(c);
}

void EquationEditor::saveEquation_()
{
    if (presetName_.isEmpty() || equationName_.isEmpty())
        return;

    IO::EquationPreset * p = 0;

    IO::EquationPresets ps;
    for (int i=0; i<ps.count(); ++i)
    {
        if (ps.name(i) == presetName_)
        {
            p = ps.preset(i);
            break;
        }
    }

    if (p == 0)
        return;

    p->setEquation(equationName_, toPlainText());
    try
    {
        p->save();
        highlight_(true);
        updatePresetMenu_();
    }
    catch (Exception & e)
    {
        QMessageBox::critical(this, tr("saving equation"),
                tr("Sorry but saving the equation preset '%1' failed\n%2")
                              .arg(presetName_).arg(e.what()));
        return;
    }
}

void EquationEditor::saveEquationAs_()
{
    SaveEquationDialog diag(toPlainText());
    if (diag.exec() == QDialog::Accepted)
    {
        presetName_ = diag.presetName();
        equationName_ = diag.equationName();
        updatePresetMenu_();
        highlight_(true);
    }
}

void EquationEditor::saveEquation_(QAction * a)
{
    QString presName = a->data().toString();

    SaveEquationDialog diag(toPlainText(), presName);
    if (diag.exec() == QDialog::Accepted)
    {
        presetName_ = diag.presetName();
        equationName_ = diag.equationName();
        updatePresetMenu_();
        highlight_(true);
    }
}

} // namespace GUI
} // namespace MO
