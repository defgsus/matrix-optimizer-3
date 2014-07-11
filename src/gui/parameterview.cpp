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
#include "object/trackfloat.h"
#include "object/param/parameterfloat.h"
#include "io/error.h"
#include "model/objecttreemodel.h"

namespace MO {
namespace GUI {

ParameterView::ParameterView(QWidget *parent) :
    QWidget (parent),
    object_ (0),

    doChangeToCreatedTrack_    (false)
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
    prevEditWidget_ = 0;

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
    static QIcon iconModulateOn(":/icon/modulate_on.png");
    static QIcon iconModulateOff(":/icon/modulate_off.png");

    ObjectTreeModel * model = p->object()->sceneObject()->model();
    MO_ASSERT(model, "No model assigned for Parameter");

    QFrame * w = new QFrame(this);
    w->setFrameStyle(QFrame::Panel);
    w->setFrameShadow(QFrame::Sunken);

    QHBoxLayout * l = new QHBoxLayout(w);
    l->setMargin(0);

    QLabel * label = new QLabel(p->name(), w);
    l->addWidget(label);
    label->setStatusTip(p->statusTip().isEmpty()
                        ? tr("The name of the parameter")
                        : p->statusTip());

    QToolButton * but, * bmod, * breset;

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        but = bmod = new QToolButton(w);
        l->addWidget(but);
        but->setIcon(pf->modulatorIds().isEmpty()? iconModulateOff : iconModulateOn);
        but->setToolTip(tr("Create modulation track"));
        but->setStatusTip(tr("Creates a new modulation track for the given parameter"));

        but = breset = new QToolButton(w);
        l->addWidget(but);
        but->setText("0");
        but->setToolTip(tr("Set to default value (%1)").arg(pf->defaultValue()));
        but->setStatusTip(tr("Sets the value of the parameter back to the default value (%1)")
                          .arg(pf->defaultValue()));
        but->setEnabled(pf->isEditable());

        int fs = breset->contentsRect().height() - 4;
        bmod->setFixedSize(fs, fs);

        QDoubleSpinBox * spin = new QDoubleSpinBox(w);
        l->addWidget(spin);
        spin->setMinimum(pf->minValue());
        spin->setMaximum(pf->maxValue());
        spin->setDecimals(4);
        spin->setSingleStep(pf->smallStep());
        spin->setValue(pf->baseValue());
        spin->setMaximumWidth(120);
        spin->setEnabled(pf->isEditable());
        spin->setStatusTip(pf->statusTip().isEmpty()
                           ? tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons")
                           : pf->statusTip());

        if (prevEditWidget_)
            setTabOrder(prevEditWidget_, spin);
        prevEditWidget_ = spin;

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

        connect(breset, &QToolButton::pressed, [=](){ spin->setValue(pf->defaultValue()); });
        connect(bmod, &QToolButton::pressed, [=]()
        {
            if (Object * o = model->createFloatTrack(pf))
            {
                bmod->setIcon(iconModulateOn);
                if (doChangeToCreatedTrack_)
                    emit objectSelected(o);
            }
        });
    }
    else
        MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

    return w;
}


} // namespace GUI
} // namespace MO
