/** @file texteditdialog.cpp

    @brief Editor for text, equations and source-code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTimer>

#include "texteditdialog.h"
#include "helpdialog.h"
#include "widget/equationeditor.h"
#include "widget/angelscriptwidget.h"
#include "widget/glslwidget.h"
#include "tool/syntaxhighlighter.h"
#include "script/angelscript.h"
#include "script/angelscript_object.h"
#include "script/angelscript_image.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

class TextEditDialog::Private
{
public:
    Private(TextEditDialog * dialog)
        : dialog        (dialog),
          plainText     (0),
          equEdit       (0),
          scriptEdit    (0),
          rejected      (false)
    {
        // wait before text-changed
        timer.setInterval(350);
        timer.setSingleShot(true);
        connect(&timer, SIGNAL(timeout()), dialog, SIGNAL(textChanged()));
    }

    void createWidgets();
    void emitTextChanged() { if (cbAlways->isChecked()) timer.start(); }

    TextEditDialog * dialog;
    TextType textType;

    QTextEdit * plainText;
    EquationEditor * equEdit;
    AbstractScriptWidget * scriptEdit;

    QString defaultText;

    QTimer timer;
    QCheckBox * cbAlways;

    bool rejected;
};

TextEditDialog::TextEditDialog(TextType textType, QWidget *parent) :
    QDialog (parent),
    p_      (new Private(this))
{
    p_->textType = textType;
    p_->rejected = false;

    setObjectName("_TextEditDialog");
    setWindowTitle(tr("editor"));

    setMinimumSize(320, 200);

    // --- load user settings ---

    settings->restoreGeometry(this);


    p_->createWidgets();
}

TextEditDialog::TextEditDialog(const QString &text, TextType textType, QWidget *parent)
    : TextEditDialog(textType, parent)
{
    setText(text);
}

TextEditDialog::~TextEditDialog()
{
    settings->storeGeometry(this);
    delete p_;
}

TextType TextEditDialog::getTextType() const
{
    return p_->textType;
}

QString TextEditDialog::getText() const
{
    if (p_->rejected)
        return p_->defaultText;

    switch (p_->textType)
    {
        case TT_PLAIN_TEXT:
        case TT_APP_STYLESHEET: return p_->plainText->toPlainText();
        case TT_EQUATION: return p_->equEdit->toPlainText();
        case TT_GLSL:
        case TT_ANGELSCRIPT: return p_->scriptEdit->scriptText();
    }

    return QString();
}

void TextEditDialog::setText(const QString & text, bool send_signal)
{
    send_signal &= getText() != text;

    p_->defaultText = text;

    switch (p_->textType)
    {
        case TT_PLAIN_TEXT:
        case TT_APP_STYLESHEET: p_->plainText->setText(text); break;
        case TT_EQUATION: p_->equEdit->setPlainText(text); break;
        case TT_GLSL:
        case TT_ANGELSCRIPT: p_->scriptEdit->setScriptText(text); break;
    }

    if (send_signal)
        emit textChanged();
}

void TextEditDialog::addVariableNames(const QStringList & vars)
{
    if (p_->textType == TT_EQUATION)
        p_->equEdit->addVariables(vars);
}

void TextEditDialog::addVariableNames(const QStringList & vars,
                                      const QStringList & descs)
{
    if (p_->textType == TT_EQUATION)
        p_->equEdit->addVariables(vars, descs);
}

void TextEditDialog::Private::createWidgets()
{
    auto lv = new QVBoxLayout(dialog);

        switch (textType)
        {
            case TT_APP_STYLESHEET:
            case TT_PLAIN_TEXT:
                plainText = new QTextEdit(dialog);
                plainText->setTabChangesFocus(false);
                lv->addWidget(plainText);
                connect(plainText, &QTextEdit::textChanged, [=]()
                {
                    emitTextChanged();
                });

                if (textType == TT_APP_STYLESHEET)
                {
                    auto s = new SyntaxHighlighter(plainText->document());
                    s->initForStyleSheet();
                }
            break;

            case TT_EQUATION:
                equEdit = new EquationEditor(dialog);
                equEdit->setTabChangesFocus(false);
                lv->addWidget(equEdit);
                connect(equEdit, &EquationEditor::equationChanged, [=]()
                {
                    emitTextChanged();
                });
                break;

            case TT_GLSL:
            {
                auto glsl = new GlslWidget(dialog);
                lv->addWidget(glsl);
                scriptEdit = glsl;
                connect(scriptEdit, &AbstractScriptWidget::scriptTextChanged, [=]()
                {
                    emitTextChanged();
                });
            }
            break;

            case TT_ANGELSCRIPT:
            {
                auto as = new AngelScriptWidget(dialog);
                lv->addWidget(as);
                registerDefaultAngelScript( as->scriptEngine() );
                registerAngelScript_object( as->scriptEngine(), 0, true, true);
                registerAngelScript_image( as->scriptEngine(), 0, true);
                scriptEdit = as;
                connect(scriptEdit, &AbstractScriptWidget::scriptTextChanged, [=]()
                {
                    emitTextChanged();
                });
            }
            break;
        }

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // alway update?
            cbAlways = new QCheckBox(tr("auto update"), dialog);
            lh->addWidget(cbAlways);

            // ok
            auto butOk = new QPushButton(tr("Ok"), dialog);
            lh->addWidget(butOk);
            butOk->setDefault(true);

            connect(butOk, &QPushButton::pressed, [=]()
            {
                if (!cbAlways->isChecked())
                    emit dialog->textChanged();
                dialog->accept();
            });

            // run
            auto butRun = new QPushButton(tr("Up&date"), dialog);
            lh->addWidget(butRun);

            connect(butRun, SIGNAL(clicked()), dialog, SIGNAL(textChanged()));

            // cancel
            auto butCancel = new QPushButton(tr("Cancel"), dialog);
            lh->addWidget(butCancel);

            connect(butCancel, &QPushButton::pressed, [=]()
            {
                rejected = true;
                emit dialog->textChanged();
                dialog->reject();
            });

}

void TextEditDialog::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        e->accept();
        openHelp();
        return;
    }
    QDialog::keyPressEvent(e);
}

void TextEditDialog::addErrorMessage(int line, const QString &text)
{
    if (p_->scriptEdit)
        p_->scriptEdit->addCompileMessage(line, AbstractScriptWidget::M_ERROR, text);
}

void TextEditDialog::openHelp()
{
    QString url = "index";

    if (p_->textType == TT_EQUATION)
    {
        url = "equation.html";

        auto c = p_->equEdit->textCursor();
        c.select(QTextCursor::WordUnderCursor);
        if (!c.selectedText().isEmpty())
            url += "#" + c.selectedText();
    }

    auto help = new HelpDialog(url, this);
    help->show();
}

} // namespace GUI
} // namespace MO
