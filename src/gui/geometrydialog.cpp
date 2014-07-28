/** @file geometrydialog.cpp

    @brief Editor for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QLayout>
#include <QComboBox>

#include "geometrydialog.h"
#include "gl/geometry.h"
#include "gl/geometryfactory.h"
#include "widget/geometrywidget.h"

namespace MO {
namespace GUI {

GeometryDialog::GeometryDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags)
{
    setObjectName("_GeometryWidget");
    setWindowTitle(tr("geomtry editor"));

    setMinimumSize(500,400);

    createWidgets_();
}

void GeometryDialog::createWidgets_()
{
    auto lh = new QHBoxLayout(this);


        geoWidget_ = new GeometryWidget(this);
        lh->addWidget(geoWidget_);
        geoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(geoWidget_, SIGNAL(glInitialized()), this, SLOT(updateGeometry_()));


        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            auto combo = new QComboBox(this);
            lv->addWidget(combo);

            combo->addItem(tr("Box"));
            combo->addItem(tr("Grid"));

}

void GeometryDialog::updateGeometry_()
{
    auto g = new GL::Geometry();
    GL::GeometryFactory::createBox(g, 1, 1, 1);
    geoWidget_->setGeometry(g);
}

} // namespace GUI
} // namespace MO
