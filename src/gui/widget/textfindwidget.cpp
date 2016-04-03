/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/3/2016</p>
*/

#include <QLineEdit>
#include <QLayout>
#include <QToolButton>

#include "textfindwidget.h"


namespace MO {
namespace GUI {

struct TextFindWidget::Private
{
    Private(TextFindWidget* p)
        : p         (p)
    { }

    void createWidgets();

    TextFindWidget* p;
    QLineEdit* lineEdit;
};

TextFindWidget::TextFindWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    p_->createWidgets();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setStatusTip(tr("Enter text to find"));
    setToolTip(statusTip());
}

TextFindWidget::~TextFindWidget()
{
    delete p_;
}

QString TextFindWidget::text() const { return p_->lineEdit->text(); }

void TextFindWidget::setFocusEdit() { p_->lineEdit->setFocus(); }
void TextFindWidget::setText(const QString& t) { p_->lineEdit->setText(t); }
void TextFindWidget::findNext() { emit nextClick(text()); }
void TextFindWidget::findPrevious() { emit previousClick(text()); }
void TextFindWidget::endSearch() { emit searchEnded(); }

void TextFindWidget::Private::createWidgets()
{
    auto lh = new QHBoxLayout(p);
    lh->setMargin(0);

        auto but = new QToolButton(p);
        but->setText("x");
        but->setShortcut(Qt::Key_Escape);
        connect(but, SIGNAL(clicked(bool)), p, SLOT(endSearch()));
        lh->addWidget(but);

        lineEdit = new QLineEdit(p);
        connect(lineEdit, SIGNAL(textChanged(QString)),
                p, SIGNAL(textChanged(QString)));
        lh->addWidget(lineEdit);

        but = new QToolButton(p);
        but->setText("<");
        but->setShortcut(Qt::CTRL + Qt::Key_Left);
        connect(but, SIGNAL(clicked(bool)), p, SLOT(findPrevious()));
        lh->addWidget(but);

        but = new QToolButton(p);
        but->setText(">");
        but->setShortcut(Qt::CTRL + Qt::Key_Right);
        connect(but, SIGNAL(clicked(bool)), p, SLOT(findNext()));
        lh->addWidget(but);

        lh->addStretch();
}



} // namespace GUI
} // namespace MO
