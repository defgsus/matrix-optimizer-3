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
#include "io/error.h"

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

GroupWidget::GroupWidget(const QString& title, bool expanded, QWidget *parent) :
    QWidget     (parent),
    expanded_   (expanded),
    title_      (title)
{
    createLayout_();
    setExpanded(expanded_);
}

void GroupWidget::createLayout_()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto lv = new QVBoxLayout(this);
    lv->setMargin(1);

        // create header
        header_ = new QWidget(this);
        header_->setProperty("GroupHeader", true);
        lv->addWidget(header_);
        /*header_->setAutoFillBackground(true);
        QPalette p(header_->palette());
        p.setColor(QPalette::Background, p.color(QPalette::Background).darker(120));
        header_->setPalette(p);*/
        header_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        auto lh = new QHBoxLayout(header_);
        lh->setMargin(0);
        lh->setSpacing(1);

            button_ = new QToolButton(header_);
            button_->setProperty("GroupHeader", true);
            lh->addWidget(button_);
            button_->setFixedSize(20,20);
            button_->setStatusTip(tr("Expands or collapses the contents of the group"));
            updateArrow_();
            connect(button_, &QToolButton::clicked, [=]()
            {
                setExpanded(!expanded_, true);
            });

            label_ = new QLabel(title_, header_);
            label_->setProperty("GroupHeader", true);
            lh->addWidget(label_);
            lh->setStretch(lh->indexOf(label_), 2);

}

void GroupWidget::addHeaderWidget(QWidget *widget)
{
    header_->layout()->addWidget(widget);
}

void GroupWidget::addHeaderSpacing(int s)
{
    MO_ASSERT(qobject_cast<QHBoxLayout*>(header_->layout()), "wrong layout");
    static_cast<QHBoxLayout*>(header_->layout())->addSpacing(s);
}

void GroupWidget::setHeaderStatusTip(const QString &tip)
{
    header_->setStatusTip(tip);
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
    updateArrow_();

    if (send_signal)
        emit collapsed();
}

void GroupWidget::expand(bool send_signal)
{
    if (expanded_)
        return;

    for (auto w : containedWidgets_)
    {
        if (isVisible(w))
            w->setVisible(true);
    }

    expanded_ = true;
    updateArrow_();

    if (send_signal)
        emit expanded();
}

void GroupWidget::setVisible(QWidget *w, bool visible)
{
    visibility_[w] = visible;
    w->setVisible(visible & expanded_);

/* XXX seems to break the gui somehow

    // check if any of the widgets is visible
    // to hide/display the whole group widget
    bool vis = false;
    for (auto w : containedWidgets_)
    {
        if (isVisible(w))
        {
            vis = true;
            break;
        }
    }

    QWidget::setVisible(vis);
*/
}

bool GroupWidget::isVisible(QWidget * w) const
{
    auto i = visibility_.find(w);
    return (i == visibility_.end() || i.value());
}

void GroupWidget::addWidget(QWidget * w)
{
    w->setVisible(expanded_);

    layout()->addWidget(w);

    containedWidgets_.push_back(w);
    w->setParent(this);
}

void GroupWidget::addLayout(QLayout * l)
{
    MO_ASSERT(qobject_cast<QVBoxLayout*>(layout()), "wrong layout");
    static_cast<QVBoxLayout*>(layout())->addLayout(l);
}

void GroupWidget::updateArrow_()
{
    button_->setArrowType(expanded_? Qt::DownArrow : Qt::RightArrow);
}




} // namespace GUI
} // namespace MO
