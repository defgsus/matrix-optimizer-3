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
#include <QScrollArea>
#include <QScrollBar>
#include <QCheckBox>

#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/trackfloat.h"
#include "object/sequence.h"
#include "object/sequencefloat.h"
#include "object/clip.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parametertimeline1d.h"
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
    // config
    doChangeToCreatedMod_(false)

{
    setObjectName("_ParameterView");

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);

        scrollArea_ = new QScrollArea(this);
        layout->addWidget(scrollArea_);

            container_ = new QWidget(scrollArea_);
            container_->setObjectName("_parameter_container");
            container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

            layout_ = new QVBoxLayout(container_);
            layout_->setMargin(0);
            layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

            scrollArea_->setWidget(container_);
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

void ParameterView::updateParameterVisibility(Parameter * p)
{
    // see if parameter is known
    auto i = paramMap_.find(p);
    if (i!=paramMap_.end())
    {
        // see if there's a group
        auto j = groups_.find(p->groupId());
        if (j != groups_.end())
            j.value()->setVisible(i.value(), p->isVisible());
        // otherwise change widget directly
        else
            i.value()->setVisible(p->isVisible());

        squeezeView_();
    }
}

void ParameterView::squeezeView_()
{
    const int h = scrollArea_->verticalScrollBar()->sliderPosition();

    for (auto g : groups_)
        g->layout()->activate();
    layout_->activate();

    scrollArea_->widget()->setGeometry(QRect(0,0,1,1));

    scrollArea_->ensureWidgetVisible(scrollArea_->widget()->focusWidget());

    // little hack to update the viewport to the slider position
    // (it won't do it without)
    scrollArea_->verticalScrollBar()->setSliderPosition(h-1);
    scrollArea_->verticalScrollBar()->setSliderPosition(h);
}


void ParameterView::clearWidgets_()
{
    for (auto w : widgets_)
    {
        w->setVisible(false);
        w->deleteLater();
    }
    widgets_.clear();

    for (auto g : groups_)
    {
        if (!g)
        {
            MO_WARNING("This segfaulted once!!!!\n" __FILE__ "\nline " << __LINE__);
            continue;
        }
        g->setVisible(false);
        g->deleteLater();
    }
    groups_.clear();

    paramMap_.clear();

    spinsInt_.clear();
    spinsFloat_.clear();
    combosSelect_.clear();
    edits_.clear();
    checkBoxes_.clear();
}

GroupWidget * ParameterView::getGroupWidget_(const Parameter * p)
{
    auto i = groups_.find(p->groupId());
    if (i == groups_.end())
    {
        // create new
        GroupWidget * g = new GroupWidget(p->groupName(), container_);
        g->setMinimumWidth(300);

        layout_->addWidget(g);
        groups_.insert(p->groupId(), g);

        // get expanded flag from scene-settings
        MO_ASSERT(p->object(), "parameter without object in ParameterView");
        g->setExpanded(
            sceneSettings_->getParameterGroupExpanded(p->object(), p->groupId()) );

        connect(g, &GroupWidget::expanded, [=]()
        {
            sceneSettings_->setParameterGroupExpanded(p->object(), p->groupId(), true);
        });
        connect(g, &GroupWidget::collapsed, [=]()
        {
            sceneSettings_->setParameterGroupExpanded(p->object(), p->groupId(), false);
            squeezeView_();
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

        // visibility
        paramMap_.insert(p, w);
        if (!p->isVisible())
            w->setVisible(false);

        if (!p->groupId().isEmpty())
        {
            GroupWidget * group = getGroupWidget_(p);
            group->addWidget(w);
            group->setVisible(w, p->isVisible());
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

    squeezeView_();
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

    QFrame * w = new QFrame(container_);
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

        if (!ps->isBoolean())
        {
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

        // boolean parameter
        else
        {
            QCheckBox * cb = new QCheckBox(w);
            l->addWidget(cb);
            checkBoxes_.append(cb);
            // important for update
            cb->setObjectName(p->idName());

            cb->setEnabled(ps->isEditable());
            cb->setStatusTip(ps->statusTip());
            cb->setChecked(ps->baseValue() != 0);

            w->setFocusProxy(cb);
            setNextTabWidget_(cb);

            connect(cb, &QCheckBox::clicked, [=]()
            {
                Scene * scene = p->object()->sceneObject();
                MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
                if (!scene) return;
                scene->setParameterValue(ps, cb->isChecked()? 1 : 0);
            });

            // reset to default
            connect(breset, &QToolButton::pressed, [=]()
            {
                cb->setChecked(ps->defaultValue() != 0);
                Scene * scene = p->object()->sceneObject();
                MO_ASSERT(scene, "no Scene for Parameter '" << p->idName() << "'");
                if (!scene) return;
                scene->setParameterValue(ps, ps->defaultValue());
            });
        }
    }
    else

    // --- filename parameter ---
    if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(p))
    {
        defaultValueName = pfn->defaultValue();

        QLineEdit * edit = new QLineEdit(w);
        l->addWidget(edit);
        edits_.append(edit);
        // important for update
        edit->setObjectName(p->idName());

        // XXX
        breset->setVisible(false);

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

    // --- text parameter ---
    if (ParameterText * ptxt = dynamic_cast<ParameterText*>(p))
    {
        defaultValueName = ptxt->defaultValue();

        QLineEdit * edit = new QLineEdit(w);
        l->addWidget(edit);
        edits_.append(edit);
        // important for update
        edit->setObjectName(p->idName());

        edit->setReadOnly(true);
        edit->setStatusTip(ptxt->statusTip());
        edit->setText(ptxt->baseValue());

        // XXX
        breset->setVisible(false);

        w->setFocusProxy(edit);
        setNextTabWidget_(edit);

        // edit button
        QToolButton * butedit = new QToolButton(w);
        l->addWidget(butedit);
        butedit->setText("...");
        butedit->setStatusTip(tr("Click to edit the text"));

        // load button click
        connect(butedit, &QToolButton::clicked, [=]()
        {
            ptxt->openEditDialog(this);
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            edit->setText(ptxt->defaultValue());
        });
    }

    else
    // --- timeline1d parameter ---
    if (ParameterTimeline1D * ptl = dynamic_cast<ParameterTimeline1D*>(p))
    {
        defaultValueName = ptl->defaultTimeline() ? "..." : "empty";

        // edit button
        QToolButton * butedit = new QToolButton(w);
        l->addWidget(butedit);
        butedit->setText("...");
        butedit->setStatusTip(tr("Click to edit the timeline"));

        // load button click
        connect(butedit, &QToolButton::clicked, [=]()
        {
            ptl->openEditDialog(this);
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            ptl->reset();
            scene_->setParameterValue(ptl, ptl->getDefaultTimeline());
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
#ifndef MO_DISABLE_TREE
    ObjectTreeModel * model = scene->model();
    MO_ASSERT(model, "No model assigned for Parameter");
#endif

    QMenu * menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction * a;

    // --- parameter float ---

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param))
    {
        addCreateModMenuFloat_(menu, param);

        // link to existing modulator
        addLinkModMenu_(menu, param,
            pf->getModulatorTypes());

        menu->addSeparator();

        // edit modulations
        addEditModMenu_(menu, param);

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param);

    }
    else
    if (ParameterInt * pi = dynamic_cast<ParameterInt*>(param))
    {
        // create modulation
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
        connect(a, &QAction::triggered, [=]()
        {
#ifndef MO_DISABLE_TREE
            if (Object * o = model->createFloatTrack(pi))
            {
                if (doChangeToCreatedMod_)
                    emit objectSelected(o);
            }
#endif
        });
        // create modulation in clip
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float sequence"), menu) );
        QMenu * sub = new QMenu(menu);
        a->setMenu(sub);
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("in new clip"), sub) );
        auto list = Clip::getAssociatedClips(param, true);
        for (Clip * clip : list)
        {
            sub->addAction( a = new QAction(tr("clip %1").arg(clip->name()), sub) );
            a->setData(QVariant::fromValue(clip));
        }
        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            Clip * parent = a->data().value<Clip*>();
#ifndef MO_DISABLE_TREE
            if (Object * o = model->createInClip("SequenceFloat", parent))
            {
                // modulate
                scene_->addModulator(param, o->idName());
                o->setName("mod: " + param->infoName());

                if (doChangeToCreatedMod_)
                    emit objectSelected(o);
            }
#endif
        });

        // link to existing modulator
        addLinkModMenu_(menu, param,
            pi->getModulatorTypes());

        menu->addSeparator();

        // edit modulations
        addEditModMenu_(menu, param);

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param);
    }
    else
        MO_ASSERT(false, "No modulation menu implemented for requested parameter '" << param->infoIdName() << "'");

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
    // remove single
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
    // remove all
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

    QMenu * linkMenu = ObjectMenu::createObjectChildMenu(scene, objectTypeFlags, menu);
    if (linkMenu->isEmpty())
    {
        linkMenu->deleteLater();
        return;
    }

    // disable the entries that are already modulators
    ObjectMenu::setEnabled(linkMenu, param->modulatorIds(), false);

    QAction * a = menu->addMenu(linkMenu);
    a->setText(tr("Choose existing source"));
    a->setIcon(QIcon(":/icon/modulate_on.png"));
    connect(linkMenu, &QMenu::triggered, [=](QAction* a)
    {
        scene->addModulator(param, a->data().toString());
    });

}


void ParameterView::addCreateModMenuFloat_(QMenu * menu, Parameter * param)
{
    // get model
    MO_ASSERT(param->object() && param->object()->sceneObject()
#ifndef MO_DISABLE_TREE
              && param->object()->sceneObject()->model()
#endif
              , "missing model in modulator menu");
#ifndef MO_DISABLE_TREE
    ObjectTreeModel * model =
        param->object()->sceneObject()->model();
#endif
    // create track modulation
    QAction * a;
    menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
    connect(a, &QAction::triggered, [=]()
    {
#ifndef MO_DISABLE_TREE
        if (Object * o = model->createFloatTrack(param))
        {
            if (doChangeToCreatedMod_)
                emit objectSelected(o);
        }
#endif
    });

    // create sequence
    menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float sequence"), menu) );
    QMenu * sub = new QMenu(menu);
    a->setMenu(sub);

        // right here
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("right here"), sub) );
        connect(a, &QAction::triggered, [=]()
        {
#ifndef MO_DISABLE_TREE
            if (Object * o = model->createFloatSequenceFor(param->object()))
            {
                // modulate
                scene_->addModulator(param, o->idName());
                o->setName(">" + param->infoName());

                if (doChangeToCreatedMod_)
                    emit objectSelected(o);
            }
#endif
        });

        sub->addSeparator();

        // in new clip
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("in new clip"), sub) );
        connect(a, &QAction::triggered, [=]()
        {
#ifndef MO_DISABLE_TREE
            if (Object * o = model->createInClip("SequenceFloat", 0))
            {
                // modulate
                scene_->addModulator(param, o->idName());
                o->setName(">" + param->infoName());

                if (doChangeToCreatedMod_)
                    emit objectSelected(o);
            }
#endif
        });

        // in existing clip
        auto list = Clip::getAssociatedClips(param,
                Object::TG_REAL_OBJECT | Object::TG_SEQUENCE
                | Object::TG_TRACK | Object::TG_TRANSFORMATION
                | Object::T_CLIP);

        for (Clip * clip : list)
        {
            sub->addAction( a = new QAction(tr("clip %1 (%2)").arg(clip->name()).arg(clip->idName()), sub) );
            a->setData(QVariant::fromValue(clip));
        }

        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
#ifndef MO_DISABLE_TREE
            if (Clip * parent = a->data().value<Clip*>())
            if (Object * o = model->createInClip("SequenceFloat", parent))
            {
                // modulate
                scene_->addModulator(param, o->idName());
                o->setName(">" + param->infoName());

                if (doChangeToCreatedMod_)
                    emit objectSelected(o);
            }
#endif
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
        for (QCheckBox * cb : checkBoxes_)
        {
            if (cb->objectName() == ps->idName())
            {
                cb->setChecked(ps->baseValue());
            }
        }
    }
    else
    if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(p))
    {
        for (QLineEdit * edit : edits_)
        {
            if (edit->objectName() == pfn->idName())
                edit->setText(pfn->value());
        }
    }
    else
    if (ParameterText * ptxt = dynamic_cast<ParameterText*>(p))
    {
        for (QLineEdit * edit : edits_)
        {
            if (edit->objectName() == ptxt->idName())
                edit->setText(ptxt->value());
        }
    }

    else
        MO_WARNING("Parameter widget update not implemented for '" << p->infoIdName() << "'");
}



} // namespace GUI
} // namespace MO
