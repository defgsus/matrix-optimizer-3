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

#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/parameterfloat.h"
#include "io/error.h"

namespace MO {
namespace GUI {

ParameterView::ParameterView(QWidget *parent) :
    QWidget (parent),
    object_ (0)
{
    layout_ = new QVBoxLayout(this);
    layout_->setMargin(1);
}



void ParameterView::setObject(Object *object)
{
    object_ = object;

    parameters_.clear();
    if (auto p = qobject_cast<Parameter*>(object))
    {
        parameters_.append(p);
    }
    //else
    //    parameters_ = object_->findChildObjects<Parameter>();

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
    w->setFrameStyle(QFrame::Box);

    QHBoxLayout * l = new QHBoxLayout(w);
    l->setMargin(1);

    QLabel * label = new QLabel(p->name(), w);
    l->addWidget(label);

    if (ParameterFloat * pf = qobject_cast<ParameterFloat*>(p))
    {
        QDoubleSpinBox * spin = new QDoubleSpinBox(w);
        l->addWidget(spin);
        spin->setValue(pf->baseValue());
        spin->setMinimum(pf->minValue());
        spin->setMaximum(pf->maxValue());
        spin->setMaximumWidth(120);
        connect(spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=]()
        {
            QObject * scene = p->sceneObject();
            MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
            if (!scene) return;
            // threadsafe send new parameter value
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
    }
    else
        MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

    return w;
}


} // namespace GUI
} // namespace MO
