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

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        but = bmod = new QToolButton(w);
        l->addWidget(but);
        but->setStatusTip(tr("Creates a new modulation track for the given parameter"));
        updateModulatorButton_(p, bmod);

        but = breset = new QToolButton(w);
        l->addWidget(but);
        but->setText("0");
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
        if (pf->isModulateable())
        connect(bmod, &QToolButton::pressed, [=]()
        {
            openModulationPopup_(p, bmod);
            /*
            if (Object * o = model->createFloatTrack(pf))
            {
                bmod->setIcon(iconModulateOn);
                if (doChangeToCreatedTrack_)
                    emit objectSelected(o);
            }*/
        });
    }
    else
    // --- select parameter ---
    if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(p))
    {
        but = bmod = new QToolButton(w);
        l->addWidget(but);
        but->setStatusTip(tr("Creates a new modulation track for the given parameter"));
        updateModulatorButton_(p, bmod);

        but = breset = new QToolButton(w);
        l->addWidget(but);
        but->setText("0");
        but->setStatusTip(tr("Sets the value of the parameter back to the default value (%1)")
                          .arg(ps->defaultValueName()));
        but->setEnabled(ps->isEditable());

        int fs = breset->contentsRect().height() - 4;
        bmod->setFixedSize(fs, fs);

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

        connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::highlighted), [=](int idx)
        {
            if (idx >= 0 && idx < ps->statusTips().size())
            {
                combo->view()->setStatusTip(ps->statusTips().at(idx));
                // need to emit explicity for statusbar to update
                emit statusTipChanged(combo->view()->statusTip());
            }
        });

        connect(breset, &QToolButton::pressed, [=]()
        {
            combo->setCurrentIndex(ps->valueList().indexOf(ps->defaultValue()));
        });
        if (ps->isModulateable())
        connect(bmod, &QToolButton::pressed, [=]()
        {
            /*
            if (Object * o = model->createFloatTrack(pf))
            {
                bmod->setIcon(iconModulateOn);
                if (doChangeToCreatedTrack_)
                    emit objectSelected(o);
            }
            */
        });
    }
    else
    MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

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
    QAction * a;

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param))
    {
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = model->createFloatTrack(pf))
            {
                updateModulatorButton_(param, button);
                if (doChangeToCreatedTrack_)
                    emit objectSelected(o);
            }
        });

        addRemoveModMenu_(menu, param, button);

        QMenu * linkMenu = ObjectMenu::createObjectMenu(scene, Object::T_TRACK_FLOAT, menu);
        a = menu->addMenu(linkMenu);
        a->setText(tr("Choose existing track"));
        a->setIcon(QIcon(":/icon/obj_track.png"));
        connect(linkMenu, &QMenu::triggered, [=](QAction* a)
        {
            scene->addModulator(param, a->data().toString());
            updateModulatorButton_(param, button);
        });
    }
    else
        MO_ASSERT(false, "No modulation menu implemented for requested parameter '" << param->idName() << "'");

    if (menu->isEmpty())
        return;

    connect(menu, &QMenu::aboutToHide, [=](){ updateModulatorButton_(param, button); });
    menu->popup(button->mapToGlobal(QPoint(0,0)));

    //delete menu;
}

void ParameterView::addRemoveModMenu_(QMenu * menu, Parameter * param, QToolButton * button)
{
    if (param->modulatorIds().size() == 1)
    {
        QAction * a = new QAction(tr("Remove modulation"), menu);
        connect(a, &QAction::triggered, [=]()
        {
            param->object()->sceneObject()->removeModulator(param, param->modulatorIds().at(0));
            updateModulatorButton_(param, button);
        });
        menu->addAction(a);
    }
    else
    if (param->modulatorIds().size() > 1)
    {
        QMenu * rem = ObjectMenu::createRemoveModulationMenu(param, menu);
        QAction * a = menu->addMenu(rem);
        a->setText(tr("Remove modulation"));
        connect(rem, &QMenu::triggered, [=](QAction* a)
        {
            param->object()->sceneObject()->removeModulator(param, a->data().toString());
            updateModulatorButton_(param, button);
        });
    }
}

} // namespace GUI
} // namespace MO
