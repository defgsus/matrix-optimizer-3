/** @file parameterview.cpp

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/
#include <QDebug>

#include <QLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QToolButton>
#include <QComboBox>
#include <QAbstractItemView>
#include <QMenu>

#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/trackfloat.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "io/error.h"
#include "model/objecttreemodel.h"
#include "util/objectmenu.h"


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

void ParameterView::updateModulatorButton_(Parameter * p, QToolButton * b)
{
    static QIcon iconModulateOn(":/icon/modulate_on.png");
    static QIcon iconModulateOff(":/icon/modulate_off.png");

    b->setEnabled(p->isModulateable());

    if (p->modulatorIds().size())
    {
        b->setDown(true);
        b->setIcon(iconModulateOn);
    }
    else
    {
        b->setDown(false);
        b->setIcon(iconModulateOff);
    }
}

QWidget * ParameterView::createWidget_(Parameter * p)
{
    MO_ASSERT(p->object(), "no object assigned to parameter");
    MO_ASSERT(p->object()->sceneObject(), "no scene assigned to parameter object");
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

    QToolButton * but, * bmod = 0, * breset;


    // modulate button
    bmod = new QToolButton(w);
    l->addWidget(bmod);
    bmod->setStatusTip(tr("Click to open the context menu for modulating the parameter"));
    updateModulatorButton_(p, bmod);
    if (p->isModulateable())
        connect(bmod, &QToolButton::pressed,
            [=]() { openModulationPopup_(p, bmod); });

    // reset-to-default button
    but = breset = new QToolButton(w);
    l->addWidget(but);
    but->setText("0");
    but->setEnabled(p->isEditable());
    QString defaultValueName;

    int fs = breset->contentsRect().height() - 4;
    bmod->setFixedSize(fs, fs);

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        defaultValueName = QString::number(pf->defaultValue());

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
    }
    else
    // --- select parameter ---
    if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(p))
    {
        defaultValueName = ps->defaultValueName();

        QComboBox * combo = new QComboBox(w);
        l->addWidget(combo);

        combo->setEnabled(ps->isEditable());

        // fill combobox with value names
        for (auto & name : ps->valueNames())
            combo->addItem(name);

        // set index and statustip
        combo->setCurrentIndex(ps->valueList().indexOf(ps->baseValue()));
        if (combo->currentIndex() >= 0 && combo->currentIndex() < ps->statusTips().size())
            combo->setStatusTip(ps->statusTips().at(combo->currentIndex()));

        if (prevEditWidget_)
            setTabOrder(prevEditWidget_, combo);
        prevEditWidget_ = combo;

        connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx)
        {
            if (idx<0 || idx >= ps->valueList().size())
                return;
            // update statustip of combobox
            if (idx >= 0 && idx < ps->statusTips().size())
                combo->setStatusTip(ps->statusTips().at(idx));

            // get value
            int value = ps->valueList().at(combo->currentIndex());
            Scene * scene = p->object()->sceneObject();
            MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
            if (!scene) return;
            /*bool r =
                metaObject()->invokeMethod(scene,
                                           "setParameterValue",
                                           Qt::QueuedConnection,
                                           Q_ARG(MO::ParameterSelect*, ps),
                                           Q_ARG(int, value)
                                           );
            MO_ASSERT(r, "could not invoke Scene::setParameterValue");
            Q_UNUSED(r);
            */
            scene->setParameterValue(ps, value);
        });

        // statustips from combobox items
        connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::highlighted), [=](int idx)
        {
            if (idx >= 0 && idx < ps->statusTips().size())
            {
                combo->view()->setStatusTip(ps->statusTips().at(idx));
                // need to emit explicity for statusbar to update
                emit statusTipChanged(combo->view()->statusTip());
            }
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            combo->setCurrentIndex(ps->valueList().indexOf(ps->defaultValue()));
        });
    }
    else
    MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

    if (defaultValueName.isEmpty())
        breset->setEnabled(false);
    else
        breset->setStatusTip(tr("Sets the value of the parameter back to the default value (%1)")
                      .arg(defaultValueName));

    return w;
}

void ParameterView::openModulationPopup_(Parameter * param, QToolButton * button)
{
    Object * object = param->object();
    MO_ASSERT(object, "No Object for edit Parameter");
    Scene * scene = object->sceneObject();
    MO_ASSERT(scene, "No Scene for object in Parameter");
    ObjectTreeModel * model = scene->model();
    MO_ASSERT(model, "No model assigned for Parameter");

    QMenu * menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction * a;

    // --- parameter float ---

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param))
    {
        // create modulation
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = model->createFloatTrack(pf))
            {
                if (doChangeToCreatedTrack_)
                    emit objectSelected(o);
            }
        });

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param);

        menu->addSeparator();

        // link to existing modulator
        addLinkModMenu_(menu, param, Object::T_TRACK_FLOAT);
    }
    else
        MO_ASSERT(false, "No modulation menu implemented for requested parameter '" << param->idName() << "'");

    if (menu->isEmpty())
    {
        menu->deleteLater();
        return;
    }

    connect(menu, &QMenu::destroyed, [=](){ updateModulatorButton_(param, button); });
    menu->popup(button->mapToGlobal(QPoint(0,0)));
}

void ParameterView::addRemoveModMenu_(QMenu * menu, Parameter * param)
{
    /*
    if (param->modulatorIds().size() == 1)
    {
        QAction * a = new QAction(tr("Remove"), menu);
        a->setStatusTip(tr("Removes the modulator from this parameter"));
        connect(a, &QAction::triggered, [=]()
        {
            param->object()->sceneObject()->removeModulator(param, param->modulatorIds().at(0));
        });
        menu->addAction(a);
    }
    else
    */
    if (param->modulatorIds().size() > 0)
    {
        QMenu * rem = ObjectMenu::createRemoveModulationMenu(param, menu);
        QAction * a = menu->addMenu(rem);
        a->setText(tr("Remove modulation"));
        a->setStatusTip(tr("Removes individual modulators from this parameter"));
        connect(rem, &QMenu::triggered, [=](QAction* a)
        {
            param->object()->sceneObject()->removeModulator(param, a->data().toString());
        });
    }
    if (param->modulatorIds().size() > 1)
    {
        QAction * a = new QAction(tr("Remove all modulations (%1)")
                                  .arg(param->modulatorIds().size()), menu);
        a->setStatusTip(tr("Removes all modulators from this parameter"));
        menu->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            param->object()->sceneObject()->removeAllModulators(param);
        });
    }
}

void ParameterView::addLinkModMenu_(
        QMenu * menu, Parameter * param, int objectTypeFlags)
{
    Scene * scene = param->object()->sceneObject();

    QMenu * linkMenu = ObjectMenu::createObjectMenu(scene, objectTypeFlags, menu);
    if (linkMenu->isEmpty())
    {
        linkMenu->deleteLater();
        return;
    }

    QAction * a = menu->addMenu(linkMenu);
    a->setText(tr("Choose existing track"));
    a->setIcon(QIcon(":/icon/obj_track.png"));
    connect(linkMenu, &QMenu::triggered, [=](QAction* a)
    {
        scene->addModulator(param, a->data().toString());
    });

}

} // namespace GUI
} // namespace MO
