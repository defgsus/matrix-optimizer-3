/** @file clipview.cpp

    @brief Clip object view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include <QLayout>
#include <QScrollArea>

#include "clipview.h"
#include "widget/clipwidget.h"

namespace MO {
namespace GUI {



ClipView::ClipView(QWidget * parent)
    : QWidget (parent)
{
    setObjectName("_ClipView");

    setMinimumSize(240, 120);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(20,20,20));
    setPalette(p);
    setAutoFillBackground(true);

    createWidgets_();
}

void ClipView::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        scrollArea_ = new QScrollArea(this);
        lv->addWidget(scrollArea_);

        container_ = new QWidget(scrollArea_);
        container_->setObjectName("_parameter_container");
        container_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

            // basic layout
            layout_ = new QGridLayout(container_);
            layout_->setContentsMargins(1,1,1,1);
            layout_->setSpacing(0);

                for (int y=0; y<20; ++y)
                {
                    for (int x=0; x<20; ++x)
                        if (!(x==0 && y==0))
                            layout_->addWidget(new ClipWidget(x, y, this), y, x);
                }

                layout_->setRowStretch(1000,1);
                layout_->setColumnStretch(1000,1);

                scrollArea_->setWidget(container_);

}

} // namespace GUI
} // namespace MO
