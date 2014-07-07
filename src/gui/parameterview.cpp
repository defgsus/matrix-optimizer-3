/** @file parameterview.cpp

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QToolButton>

#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/param/parameterfloat.h"
#include "io/error.h"

namespace MO {
namespace GUI {

ParameterView::ParameterView(QWidget *parent) :
    QWidget (parent),
    object_ (0)
{
    layout_ = new QVBoxLayout(this);
    layout_->setMargin(0);
}



void ParameterView::setObject(Object *object)
{
    object_ = object;

    if (!object_)
    {
        clearWidgets_();
        return;
    }

    parameters_ = object_->parameters();

    createWidgets_();
}

void ParameterView::clearWidgets_()
{
    for (auto i : widgets_)
        i->deleteLater();

    widgets_.clear();
}

void ParameterView::createWidgets_()
{
    clearWidgets_();

    for (auto p : parameters_)
    {
        if (widgets_.contains(p->idName()))
        {
            //updateWidget
        }
        else
        {
            QWidget * w = createWidget_(p);
            widgets_.insert(p->idName(), w);
            layout_->addWidget(w);
        }
    }
}

QWidget * ParameterView::createWidget_(Parameter * p)
{
    QFrame * w = new QFrame(this);
    w->setFrameStyle(QFrame::Panel);
    w->setFrameShadow(QFrame::Sunken);

    QHBoxLayout * l = new QHBoxLayout(w);
    l->setMargin(0);

    QLabel * label = new QLabel(p->name(), w);
    l->addWidget(label);

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        QToolButton * but = new QToolButton(w);
        l->addWidget(but);
        but->setText("0");
        but->setToolTip(tr("Set to default value (%1)").arg(pf->defaultValue()));

        QDoubleSpinBox * spin = new QDoubleSpinBox(w);
        l->addWidget(spin);
        spin->setMinimum(pf->minValue());
        spin->setMaximum(pf->maxValue());
        spin->setValue(pf->baseValue());
        spin->setMaximumWidth(120);
        connect(spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=]()
        {
            QObject * scene = p->object()->sceneObject();
            MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
            if (!scene) return;
            // threadsafe send new parameter value
            // XXX only testing syntax here,
            //     Scene will probably have to handle more threads
            bool r =
                metaObject()->invokeMethod(scene,
                                           "setParameterValue",
                                           Qt::QueuedConnection,
                                           Q_ARG(MO::ParameterFloat*, pf),
                                           Q_ARG(Double, spin->value())
                                           );
            MO_ASSERT(r, "could not invoke Scene::setParameterValue");
            Q_UNUSED(r);
        });

        connect(but, &QToolButton::pressed, [=](){ spin->setValue(pf->defaultValue()); });
    }
    else
        MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

    return w;
}


} // namespace GUI
} // namespace MO
