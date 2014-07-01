/** @file objectview.cpp

    @brief Object display / editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QIcon>
#include <QToolButton>
#include <QLayout>
#include <QLabel>

#include "objectview.h"
#include "parameterview.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "tool/stringmanip.h"

namespace MO {
namespace GUI {


ObjectView::ObjectView(QWidget *parent) :
    QWidget (parent),
    object_ (0)
{
    layout_ = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        layout_->addLayout(lh);

            icon_ = new QToolButton(this);
            lh->addWidget(icon_);
            icon_->setToolButtonStyle(Qt::ToolButtonIconOnly);

            label_ = new QLabel(this);
            lh->addWidget(label_);

        paramView_ = new ParameterView(this);
        layout_->addWidget(paramView_);
}

void ObjectView::setObject(Object * object)
{
    object_ = object;

    if (object_)
    {
        icon_->setIcon(ObjectFactory::iconForObject(object_));
        label_->setText(QString("<html><b>%1</b><br/>%2/%1</html>")
                        .arg(object_->name())
                        .arg(fit_in_length(object_->namePath(), 36))
                        );
    }
    else
    {
        icon_->setIcon(QIcon());
        label_->setText(QString());
    }

    paramView_->setObject(object_);
}


} // namespace GUI
} // namespace MO
