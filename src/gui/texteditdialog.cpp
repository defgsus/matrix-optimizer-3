/** @file texteditdialog.cpp

    @brief Editor for text, equations and source-code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QKeyEvent>

#include "texteditdialog.h"
#include "widget/equationeditor.h"
#include "helpdialog.h"

namespace MO {
namespace GUI {

class TextEditDialog::Private
{
public:
    Private(TextEditDialog * dialog) : dialog(dialog) { }

    TextEditDialog * dialog;
    TextType textType;

    QTextEdit * plainText;
    EquationEditor * equEdit;

    QString defaultText;

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
    createWidgets_();
}

TextEditDialog::TextEditDialog(const QString &text, TextType textType, QWidget *parent)
    : TextEditDialog(textType, parent)
{
    setText(text);
}

TextEditDialog::~TextEditDialog()
{
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
        case TT_PLAIN_TEXT: return p_->plainText->toPlainText();
        case TT_EQUATION: return p_->equEdit->toPlainText();
    }

    return QString();
}

void TextEditDialog::setText(const QString & text, bool send_signal)
{
    send_signal &= getText() != text;

    p_->defaultText = text;

    switch (p_->textType)
    {
        case TT_PLAIN_TEXT: p_->plainText->setText(text); break;
        case TT_EQUATION: p_->equEdit->setPlainText(text); break;
    }

    if (send_signal)
        emit textChanged();
}

void TextEditDialog::addVariableNames(const QStringList & vars)
{
    if (p_->textType == TT_EQUATION)
        p_->equEdit->addVariables(vars);
}

void TextEditDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        switch (p_->textType)
        {
            case TT_PLAIN_TEXT:
                p_->plainText = new QTextEdit(this);
                lv->addWidget(p_->plainText);
                connect(p_->plainText, &QTextEdit::textChanged, [=]()
                {
                    emit textChanged();
                });
                break;

            case TT_EQUATION:
                p_->equEdit = new EquationEditor(this);
                lv->addWidget(p_->equEdit);
                connect(p_->equEdit, &EquationEditor::equationChanged, [=]()
                {
                    emit textChanged();
                });
                break;
        }

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto butOk = new QPushButton(tr("Ok"), this);
            lh->addWidget(butOk);
            butOk->setDefault(true);

            connect(butOk, &QPushButton::pressed, [=]()
            {
                accept();
            });

            auto butCancel = new QPushButton(tr("Cancel"), this);
            lh->addWidget(butCancel);

            connect(butCancel, &QPushButton::pressed, [=]()
            {
                p_->rejected = true;
                emit textChanged();
                reject();
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
