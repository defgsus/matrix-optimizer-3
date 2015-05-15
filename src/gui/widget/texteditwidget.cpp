/** @file texteditwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.01.2015</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTimer>

#include "texteditwidget.h"
#include "gui/helpdialog.h"
#include "equationeditor.h"
#include "angelscriptwidget.h"
#include "glslwidget.h"
#include "tool/syntaxhighlighter.h"
#include "script/angelscript.h"
#include "script/angelscript_object.h"
#include "script/angelscript_image.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

class TextEditWidget::Private
{
public:
    Private(TextEditWidget * dialog)
        : widget        (dialog),
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

    TextEditWidget * widget;
    TextType textType;

    QTextEdit * plainText;
    EquationEditor * equEdit;
    AbstractScriptWidget * scriptEdit;

    QString defaultText;

    QTimer timer;
    QCheckBox * cbAlways;

    bool rejected;
};

TextEditWidget::TextEditWidget(TextType textType, QWidget *parent) :
    QWidget (parent),
    p_      (new Private(this))
{
    p_->textType = textType;

    setObjectName("_TextEditWidget");
    setWindowTitle(tr("editor"));

    setMinimumSize(320, 200);

    // --- load user settings ---

    settings()->restoreGeometry(this);


    p_->createWidgets();
}

TextEditWidget::TextEditWidget(const QString &text, TextType textType, QWidget *parent)
    : TextEditWidget(textType, parent)
{
    setText(text);
}

TextEditWidget::~TextEditWidget()
{
    settings()->storeGeometry(this);
    delete p_;
}

TextType TextEditWidget::getTextType() const
{
    return p_->textType;
}

QString TextEditWidget::getText() const
{
    if (p_->rejected)
        return p_->defaultText;

    switch (p_->textType)
    {
        case TT_OBJECT_WILDCARD:
        case TT_PLAIN_TEXT:
        case TT_APP_STYLESHEET: return p_->plainText->toPlainText();
        case TT_EQUATION: return p_->equEdit->toPlainText();
        case TT_GLSL:
        case TT_ANGELSCRIPT: return p_->scriptEdit->scriptText();
    }

    return QString();
}

void TextEditWidget::setText(const QString & text, bool send_signal)
{
    send_signal &= getText() != text;

    p_->defaultText = text;

    switch (p_->textType)
    {
        case TT_OBJECT_WILDCARD:
        case TT_PLAIN_TEXT:
        case TT_APP_STYLESHEET: p_->plainText->setText(text); break;
        case TT_EQUATION: p_->equEdit->setPlainText(text); break;
        case TT_GLSL:
        case TT_ANGELSCRIPT: p_->scriptEdit->setScriptText(text); break;
    }

    if (send_signal)
        emit textChanged();
}

void TextEditWidget::addVariableNames(const QStringList & vars)
{
    if (p_->textType == TT_EQUATION)
        p_->equEdit->addVariables(vars);
}

void TextEditWidget::addVariableNames(const QStringList & vars,
                                      const QStringList & descs)
{
    if (p_->textType == TT_EQUATION)
        p_->equEdit->addVariables(vars, descs);
}

void TextEditWidget::Private::createWidgets()
{
    auto lv = new QVBoxLayout(widget);

        switch (textType)
        {
            case TT_OBJECT_WILDCARD:
            case TT_APP_STYLESHEET:
            case TT_PLAIN_TEXT:
                plainText = new QTextEdit(widget);
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
                equEdit = new EquationEditor(widget);
                equEdit->setTabChangesFocus(false);
                lv->addWidget(equEdit);
                connect(equEdit, &EquationEditor::equationChanged, [=]()
                {
                    emitTextChanged();
                });
                break;

            case TT_GLSL:
            {
                auto glsl = new GlslWidget(widget);
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
                auto as = new AngelScriptWidget(widget);
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
            cbAlways = new QCheckBox(tr("auto update"), widget);
            lh->addWidget(cbAlways);

            // ok
            auto butOk = new QPushButton(tr("Ok"), widget);
            lh->addWidget(butOk);
            butOk->setDefault(true);

            connect(butOk, &QPushButton::pressed, [=]()
            {
                if (!cbAlways->isChecked())
                    emit widget->textChanged();
                emit widget->closeRequest();
            });

            // run
            auto butRun = new QPushButton(tr("Up&date"), widget);
            lh->addWidget(butRun);

            connect(butRun, SIGNAL(clicked()), widget, SIGNAL(textChanged()));

            // cancel
            auto butCancel = new QPushButton(tr("Cancel"), widget);
            lh->addWidget(butCancel);

            connect(butCancel, &QPushButton::pressed, [=]()
            {
                widget->setText(defaultText, true);
                emit widget->closeRequest();
            });

}

void TextEditWidget::addErrorMessage(int line, const QString &text)
{
    if (p_->scriptEdit)
        p_->scriptEdit->addCompileMessage(line, AbstractScriptWidget::M_ERROR, text);
}

void TextEditWidget::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        e->accept();
        openHelp();
        return;
    }
    QWidget::keyPressEvent(e);
}

void TextEditWidget::openHelp()
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
