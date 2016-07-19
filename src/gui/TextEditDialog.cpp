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

#include "TextEditDialog.h"
#include "HelpDialog.h"
#include "widget/EquationEditor.h"
#include "widget/AngelscriptWidget.h"
#include "widget/GlslWidget.h"
#include "widget/Python34Widget.h"
#include "tool/SyntaxHighlighter.h"
#include "script/angelscript.h"
#include "script/angelscript_object.h"
#include "script/angelscript_image.h"
#include "io/Settings.h"

namespace MO {


/** @todo This needs to be placed somewhere central */
QString textTypeId(TextType tt)
{
    switch (tt)
    {
        case TT_PLAIN_TEXT: return "plain";
        case TT_EQUATION: return "equ";
        case TT_APP_STYLESHEET: return "css";
        case TT_ANGELSCRIPT: return "angelscript";
        case TT_GLSL: return "glsl";
        case TT_PYTHON34: return "python34";
        case TT_OBJECT_WILDCARD: return "wildcard";
    }
    return QString();
}


namespace GUI {

class TextEditDialog::Private
{
public:
    Private(TextEditDialog * dialog)
        : dialog        (dialog),
          plainText     (0),
          equEdit       (0),
          scriptEdit    (0),
          rejected      (false),
          readOnly      (false)
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

    bool rejected, readOnly;
};

TextEditDialog::TextEditDialog(TextType textType, QWidget *parent) :
    QDialog (parent),
    p_      (new Private(this))
{
    p_->textType = textType;
    p_->rejected = false;

    setObjectName("_TextEditDialog_" + textTypeId(textType));
    setWindowTitle(tr("editor"));

    setMinimumSize(320, 200);

    // --- load user settings ---

    settings()->restoreGeometry(this);


    p_->createWidgets();
}

TextEditDialog::TextEditDialog(const QString &text, TextType textType, QWidget *parent)
    : TextEditDialog(textType, parent)
{
    setText(text);
}

TextEditDialog::~TextEditDialog()
{
    settings()->storeGeometry(this);
    delete p_;
}

void TextEditDialog::setReadOnly(bool readOnly)
{
    p_->readOnly = readOnly;
    if (p_->plainText)
        p_->plainText->setReadOnly(readOnly);
    if (p_->equEdit)
        p_->equEdit->setReadOnly(readOnly);
    /*
    if (p_->scriptEdit)
        p_->scriptEdit->setReadOnly(readOnly);
        */
}

TextType TextEditDialog::getTextType() const
{
    return p_->textType;
}

AbstractScriptWidget * TextEditDialog::getScriptWidget() const
{
    return p_->scriptEdit;
}

QString TextEditDialog::getText() const
{
    // XXX not used (forgot what it should do, sb)
    if (p_->rejected)
        return p_->defaultText;

    switch (p_->textType)
    {
        case TT_OBJECT_WILDCARD:
        case TT_PLAIN_TEXT:
        case TT_APP_STYLESHEET: return p_->plainText->toPlainText();
        case TT_EQUATION: return p_->equEdit->toPlainText();
        case TT_GLSL:
        case TT_ANGELSCRIPT:
        case TT_PYTHON34:
            return p_->scriptEdit->scriptText();
    }

    return QString();
}

void TextEditDialog::setText(const QString & text, bool send_signal)
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
        case TT_ANGELSCRIPT:
        case TT_PYTHON34:
            p_->scriptEdit->setScriptText(text); break;
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
            case TT_OBJECT_WILDCARD:
            case TT_APP_STYLESHEET:
            case TT_PLAIN_TEXT:
                plainText = new QTextEdit(dialog);
                plainText->setTabChangesFocus(false);
                plainText->setReadOnly(readOnly);
                lv->addWidget(plainText);
                connect(plainText, &QTextEdit::textChanged, [=]()
                {
                    emitTextChanged();
                });

                if (textType == TT_APP_STYLESHEET)
                {
                    auto s = new SyntaxHighlighter(false, dialog);
                                    //plainText->document());
                    s->initForStyleSheet();
                }
            break;

            case TT_EQUATION:
                equEdit = new EquationEditor(dialog);
                equEdit->setTabChangesFocus(false);
                equEdit->setReadOnly(readOnly);
                lv->addWidget(equEdit);
                connect(equEdit, &EquationEditor::equationChanged, [=]()
                {
                    emit dialog->textChanged();
                });
                break;

            case TT_GLSL:
            {
                auto glsl = new GlslWidget(dialog);
                lv->addWidget(glsl);
                scriptEdit = glsl;
                //scriptEdit->setReadOnly(readOnly);
                connect(scriptEdit, &AbstractScriptWidget::scriptTextChanged, [=]()
                {
                    emit dialog->textChanged();
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
                //scriptEdit->setReadOnly(readOnly);
                connect(scriptEdit, &AbstractScriptWidget::scriptTextChanged, [=]()
                {
                    emit dialog->textChanged();
                });
            }
            break;

            case TT_PYTHON34:
            {
                auto py = new Python34Widget(dialog);
                lv->addWidget(py);
                scriptEdit = py;
                //scriptEdit->setReadOnly(readOnly);
                connect(scriptEdit, &AbstractScriptWidget::scriptTextChanged, [=]()
                {
                    emit dialog->textChanged();
                });              
            }
        }
        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // always update?
            cbAlways = new QCheckBox(tr("auto update"), dialog);
            lh->addWidget(cbAlways);
            connect(cbAlways, &QCheckBox::stateChanged, [this]()
            {
                if (scriptEdit)
                    scriptEdit->setAlwaysUpdate(cbAlways->isChecked());
            });

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

            connect(butRun, &QPushButton::clicked, [this]()
            {
                if (scriptEdit)
                    scriptEdit->updateScript();
                else
                    emit dialog->textChanged();
            });

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

AngelScriptWidget * TextEditDialog::getWidgetAngelScript() const
{
    return dynamic_cast<AngelScriptWidget*>(p_->scriptEdit);
}


#ifdef MO_ENABLE_PYTHON34
Python34Widget * TextEditDialog::getWidgetPython() const
{
    return dynamic_cast<Python34Widget*>(p_->scriptEdit);
}
#endif

void TextEditDialog::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        e->accept();
        openHelp();
        return;
    }
    // avoid accidental exit
    if (e->key() == Qt::Key_Escape)
    {
        e->accept();
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
