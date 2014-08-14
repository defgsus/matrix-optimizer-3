/** @file groupwidget.cpp

    @brief A Widget to group other widgets with drop-down function

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include <QLayout>
#include <QToolButton>
#include <QLabel>

#include "groupwidget.h"

namespace MO {
namespace GUI {

GroupWidget::GroupWidget(QWidget *parent) :
    QWidget     (parent),
    expanded_   (true)
{
    createLayout_();
}

GroupWidget::GroupWidget(const QString& title, QWidget *parent) :
    QWidget     (parent),
    expanded_   (true),
    title_      (title)
{
    createLayout_();
}

void GroupWidget::createLayout_()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto lv = new QVBoxLayout(this);

        // create header
        auto head = new QWidget(this);
        lv->addWidget(head);
        head->setAutoFillBackground(true);
        QPalette p(head->palette());
        p.setColor(QPalette::Background, p.color(QPalette::Background).darker(120));
        head->setPalette(p);

        auto lh = new QHBoxLayout(head);
        lh->setMargin(0);

            button_ = new QToolButton(head);
            lh->addWidget(button_);
            button_->setFixedSize(20,20);
            button_->setArrowType(expanded_? Qt::UpArrow : Qt::DownArrow);
            connect(button_, &QToolButton::clicked, [=]()
            {
                setExpanded(!expanded_);
                button_->setArrowType(expanded_? Qt::UpArrow : Qt::DownArrow);
            });

            label_ = new QLabel(title_, head);
            lh->addWidget(label_);

}

void GroupWidget::setTitle(const QString & title)
{
    title_ = title;
    label_->setText(title_);
}

void GroupWidget::setExpanded(bool expanded, bool send_signal)
{
    if (expanded_ == expanded)
        return;

    if (expanded)
        expand(send_signal);
    else
        collapse(send_signal);
}

void GroupWidget::collapse(bool send_signal)
{
    if (!expanded_)
        return;

    for (auto w : containedWidgets_)
        w->setVisible(false);

    expanded_ = false;

    if (send_signal)
        emit collapsed();
}

void GroupWidget::expand(bool send_signal)
{
    if (expanded_)
        return;

    for (auto w : containedWidgets_)
        w->setVisible(true);

    expanded_ = true;

    if (send_signal)
        emit expanded();
}

void GroupWidget::addWidget(QWidget * w)
{
    layout()->addWidget(w);

    containedWidgets_.push_back(w);
    w->setParent(this);

    w->setVisible(expanded_);
}





} // namespace GUI
} // namespace MO
