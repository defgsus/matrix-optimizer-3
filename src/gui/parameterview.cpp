/** @file parameterview.cpp

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QFrame>
#include <QToolButton>
#include <QComboBox>
#include <QAbstractItemView>
#include <QMenu>
#include <QLineEdit>


#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/trackfloat.h"
#include "object/sequence.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/modulator.h"
#include "io/error.h"
#include "io/log.h"
#include "model/objecttreemodel.h"
#include "util/objectmenu.h"
#include "widget/spinbox.h"
#include "widget/doublespinbox.h"
#include "widget/groupwidget.h"
#include "modulatordialog.h"
#include "util/scenesettings.h"

Q_DECLARE_METATYPE(MO::Modulator*)

namespace MO {
namespace GUI {

ParameterView::ParameterView(QWidget *parent) :
    QWidget         (parent),
    scene_          (0),
    sceneSettings_  (0),
    object_         (0),

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

    Scene * scene = object_->sceneObject();
    if (scene != scene_)
    {
        scene_ = scene;
        connect(scene, SIGNAL(parameterChanged(MO::Parameter*)),
                this, SLOT(updateWidgetValue_(MO::Parameter*)));
        connect(scene, SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(onSequenceChanged(MO::Sequence*)));
    }

    parameters_ = object_->parameters();

    createWidgets_();
}

void ParameterView::clearWidgets_()
{
    for (auto w : widgets_)
        w->deleteLater();
    widgets_.clear();
    for (auto g : groups_)
        g->deleteLater();
    groups_.clear();

    spinsInt_.clear();
    spinsFloat_.clear();
    combosSelect_.clear();
    editsFilename_.clear();
}

GroupWidget * ParameterView::getGroupWidget_(const Parameter * p)
{
    auto i = groups_.find(p->groupId());
    if (i == groups_.end())
    {
        // create new
        GroupWidget * g = new GroupWidget(p->groupName(), this);
        MO_ASSERT(p->object(), "parameter without object in ParameterView");
        g->setExpanded(
            sceneSettings_->getParameterGroupExpanded(p->object(), p->groupId()) );
        layout_->addWidget(g);
        groups_.insert(p->groupId(), g);

        connect(g, &GroupWidget::expanded, [=]()
        {
            sceneSettings_->setParameterGroupExpanded(p->object(), p->groupId(), true);
        });
        connect(g, &GroupWidget::collapsed, [=]()
        {
            sceneSettings_->setParameterGroupExpanded(p->object(), p->groupId(), false);
        });

        return g;
    }
    // return existing
    return i.value();
}

void ParameterView::createWidgets_()
{
    clearWidgets_();
    prevEditWidget_ = 0;

    //QWidget * prev = 0;
    for (auto p : parameters_)
    {
        QWidget * w = createWidget_(p);

        if (!p->groupId().isEmpty())
        {
            getGroupWidget_(p)->addWidget(w);
        }
        else
        {
            layout_->addWidget(w);
            widgets_.append(w);
        }
        /*
        if (prev)
            setTabOrder(prev, w);
        prev = w;
        */
    }
}

void ParameterView::updateModulatorButton_(Parameter * p, QToolButton * b)
{
    static QIcon iconModulateOn(":/icon/modulate_on.png");
    static QIcon iconModulateOff(":/icon/modulate_off.png");

    b->setVisible(p->isModulateable());

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

void ParameterView::setNextTabWidget_(QWidget * w)
{
    if (prevEditWidget_)
        setTabOrder(prevEditWidget_, w);
    //MO_DEBUG("taborder " << prevEditWidget_ << " " << w);
    prevEditWidget_ = w;
}

QWidget * ParameterView::createWidget_(Parameter * p)
{
    static const QIcon resetIcon(":/icon/reset.png");

    const int butfs = 25;

    MO_ASSERT(p->object(), "no object assigned to parameter");
    MO_ASSERT(p->object()->sceneObject(), "no scene assigned to parameter object");
    //ObjectTreeModel * model = p->object()->sceneObject()->model();
    //MO_ASSERT(model, "No model assigned for Parameter");

    QFrame * w = new QFrame(this);
    w->setObjectName("_" + p->idName());
    w->setFrameStyle(QFrame::Panel);
    w->setFrameShadow(QFrame::Sunken);

    QHBoxLayout * l = new QHBoxLayout(w);
    l->setMargin(0);
    l->setSpacing(4);

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
    bmod->setFixedSize(butfs, butfs);
    updateModulatorButton_(p, bmod);
    if (p->isModulateable())
        connect(bmod, &QToolButton::pressed,
            [=]() { openModulationPopup_(p, bmod); });

    // reset-to-default button
    but = breset = new QToolButton(w);
    l->addWidget(but);
    but->setIcon(resetIcon);
    but->setVisible(p->isEditable());
    but->setFixedSize(butfs*0.7, butfs);
    QString defaultValueName;

    //int fs = bmod->contentsRect().height() - 4;

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        defaultValueName = QString::number(pf->defaultValue());

        DoubleSpinBox * spin = new DoubleSpinBox(w);
        l->addWidget(spin);
        spinsFloat_.append(spin);
        // important for update
        spin->setObjectName(p->idName());

        spin->setMinimum(pf->minValue());
        spin->setMaximum(pf->maxValue());
        spin->setDecimals(4);
        spin->setSingleStep(pf->smallStep());
        spin->setValue(pf->baseValue());
        spin->spinBox()->setMaximumWidth(120);
        spin->setEnabled(pf->isEditable());
        spin->setStatusTip(pf->statusTip().isEmpty()
                           ? tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons")
                           : pf->statusTip());

        w->setFocusProxy(spin);
        setNextTabWidget_(spin);

        connect(spin, static_cast<void(DoubleSpinBox::*)(double)>(&DoubleSpinBox::valueChanged), [=]()
        {
            QObject * scene = p->object()->sceneObject();
            MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
            if (!scene) return;
            // threadsafe send new parameter value
            // XXX only testing syntax here,
            //     Scene will have to handle more threads
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

        connect(breset, &QToolButton::pressed, [=](){ spin->setValue(pf->defaultValue(), true); });
    }
    else

    // --- int parameter ---
    if (ParameterInt * pi = dynamic_cast<ParameterInt*>(p))
    {
        defaultValueName = QString::number(pi->defaultValue());

        SpinBox * spin = new SpinBox(w);
        l->addWidget(spin);
        spinsInt_.append(spin);
        // important for update
        spin->setObjectName(p->idName());

        spin->setMinimum(pi->minValue());
        spin->setMaximum(pi->maxValue());
        spin->setSingleStep(pi->smallStep());
        spin->setValue(pi->baseValue());
        spin->spinBox()->setMaximumWidth(120);
        spin->setEnabled(pi->isEditable());
        spin->setStatusTip(pi->statusTip().isEmpty()
                           ? tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons")
                           : pi->statusTip());

        w->setFocusProxy(spin);
        setNextTabWidget_(spin);

        connect(spin, &SpinBox::valueChanged, [=](int value)
        {
            Scene * scene = p->object()->sceneObject();
            MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
            if (!scene) return;
            scene->setParameterValue(pi, value);
        });

        connect(breset, &QToolButton::pressed, [=](){ spin->setValue(pi->defaultValue(), true); });
    }
    else

    // --- select parameter ---
    if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(p))
    {
        defaultValueName = ps->defaultValueName();

        QComboBox * combo = new QComboBox(w);
        l->addWidget(combo);
        combosSelect_.append(combo);
        // important for update
        combo->setObjectName(p->idName());

        combo->setEnabled(ps->isEditable());

        // fill combobox with value names
        for (auto & name : ps->valueNames())
            combo->addItem(name);

        // set index and statustip
        combo->setCurrentIndex(ps->valueList().indexOf(ps->baseValue()));
        if (combo->currentIndex() >= 0 && combo->currentIndex() < ps->statusTips().size())
            combo->setStatusTip(ps->statusTips().at(combo->currentIndex()));

        w->setFocusProxy(combo);
        setNextTabWidget_(combo);

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

    // --- filename parameter ---
    if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(p))
    {
        defaultValueName = pfn->defaultValue();

        QLineEdit * edit = new QLineEdit(w);
        l->addWidget(edit);
        editsFilename_.append(edit);
        // important for update
        edit->setObjectName(p->idName());

        edit->setReadOnly(true);
        edit->setStatusTip(pfn->statusTip());
        edit->setText(pfn->value());

        w->setFocusProxy(edit);
        setNextTabWidget_(edit);

        // load button
        QToolButton * butload = new QToolButton(w);
        l->addWidget(butload);
        butload->setText("...");
        butload->setStatusTip(tr("Click to select a file"));

        // load button click
        connect(butload, &QToolButton::clicked, [=]()
        {
            pfn->openFileDialog(this);
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            edit->setText(pfn->defaultValue());
        });
    }

    else
        MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");

    if (defaultValueName.isEmpty())
        breset->setVisible(false);
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

        // link to existing modulator
        addLinkModMenu_(menu, param,
            Object::T_TRACK_FLOAT | Object::T_MODULATOR_OBJECT_FLOAT);

        menu->addSeparator();

        // edit modulations
        addEditModMenu_(menu, param);

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param);

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
        a->setIcon(QIcon(":/icon/delete.png"));
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
        a->setIcon(QIcon(":/icon/delete.png"));
        menu->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            param->object()->sceneObject()->removeAllModulators(param);
        });
    }
}

void ParameterView::addEditModMenu_(QMenu * menu, Parameter * param)
{
    if (param->modulators().size() > 0)
    {
        QMenu * edit = new QMenu(menu);
        QAction * a = menu->addMenu(edit);
        a->setText("Edit modulations");
        a->setStatusTip("Modifies modulation parameters");

        for (auto m : param->modulators())
        {
            MO_ASSERT(m->modulator(), "no assigned modulation object in Modulator");

            edit->addAction(a = new QAction(m->name(), edit));
            a->setData(QVariant::fromValue(m));
            connect(a, &QAction::triggered, [this, param, m]()
            {
                ModulatorDialog diag(this);
                diag.setModulators(param->modulators(), m);
                diag.exec();
            });
        }
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

    // disable the entries that are already modulators
    ObjectMenu::setEnabled(linkMenu, param->modulatorIds(), false);

    QAction * a = menu->addMenu(linkMenu);
    a->setText(tr("Choose existing track"));
    a->setIcon(QIcon(":/icon/obj_track.png"));
    connect(linkMenu, &QMenu::triggered, [=](QAction* a)
    {
        scene->addModulator(param, a->data().toString());
    });

}

void ParameterView::onSequenceChanged(Sequence * seq)
{
    if (seq == object_)
        updateWidgetValues_();
}

void ParameterView::updateWidgetValues_()
{
    for (auto p : parameters_)
    {
        updateWidgetValue_(p);
    }
}

void ParameterView::updateWidgetValue_(Parameter * p)
{
    //MO_DEBUG_GUI("ParameterView::updateWidgetValue_(" << p << ")");

    if (ParameterInt * pi = dynamic_cast<ParameterInt*>(p))
    {
        for (SpinBox * spin : spinsInt_)
        {
            if (spin->objectName() == pi->idName())
                spin->setValue(pi->baseValue());
        }
    }
    else
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(p))
    {
        for (DoubleSpinBox * spin : spinsFloat_)
        {
            if (spin->objectName() == pf->idName())
                spin->setValue(pf->baseValue());
        }
    }
    else
    if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(p))
    {
        for (QComboBox * combo : combosSelect_)
        {
            if (combo->objectName() == ps->idName())
            {
                int idx = ps->valueList().indexOf(ps->baseValue());
                if (combo->currentIndex() != idx)
                {
                    combo->setCurrentIndex(idx);
                    // update statustip
                    if (combo->currentIndex() >= 0 && combo->currentIndex() < ps->statusTips().size())
                        combo->setStatusTip(ps->statusTips().at(combo->currentIndex()));
                }
            }
        }
    }
    else
    if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(p))
    {
        for (QLineEdit * edit : editsFilename_)
        {
            if (edit->objectName() == pfn->idName())
                edit->setText(pfn->value());
        }
    }
}


} // namespace GUI
} // namespace MO
