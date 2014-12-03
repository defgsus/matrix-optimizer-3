/** @file parameterwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.12.2014</p>
*/

#include <QIcon>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLayout>
#include <QToolButton>
#include <QDragEnterEvent>
#include <QMenu>

#include "parameterwidget.h"
#include "spinbox.h"
#include "doublespinbox.h"
#include "object/scene.h"
#include "object/trackfloat.h"
#include "object/sequencefloat.h"
#include "object/clip.h"
#include "object/util/objecteditor.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/modulator.h"
#include "gui/modulatordialog.h"
#include "gui/util/objectmenu.h"
#include "io/error.h"

Q_DECLARE_METATYPE(MO::Modulator*)

namespace MO {
namespace GUI {

ParameterWidget::ParameterWidget(Parameter * p, QWidget *parent)
    :   QFrame      (parent),
        param_      (p),
        bmod_       (0),
        spinInt_    (0),
        spinFloat_  (0),
        comboSelect_(0),
        checkBox_   (0),
        lineEdit_   (0),
        doChangeToCreatedMod_   (true)
{
    setObjectName("_" + param_->idName());
    setFrameStyle(QFrame::Panel);
    setFrameShadow(QFrame::Sunken);
    setAcceptDrops(true);

    createWidgets_();
}

void ParameterWidget::emitObjectSelected_(Object * o)
{
    //if (doChangeToCreatedMod_)
        //emit objectSelected(o);
}

void ParameterWidget::dragEnterEvent(QDragEnterEvent * e)
{
    e->accept();
}


void ParameterWidget::createWidgets_()
{
    static const QIcon resetIcon(":/icon/reset.png");

    const int butfs = 25;

    MO_ASSERT(param_->object(), "no object assigned to parameter");
    MO_ASSERT(param_->object()->sceneObject(), "no scene assigned to parameter object");
    MO_ASSERT(param_->object()->sceneObject()->editor(), "no objec editor assigned to parameter object");
    ObjectEditor * editor = param_->object()->sceneObject()->editor();


    QHBoxLayout * l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(4);

    QLabel * label = new QLabel(param_->name(), this);
    l->addWidget(label);
    label->setStatusTip(param_->statusTip().isEmpty()
                        ? tr("The name of the parameter")
                        : param_->statusTip());

    QToolButton * but, * breset;

    // modulate button
    bmod_ = new QToolButton(this);
    l->addWidget(bmod_);
    bmod_->setStatusTip(tr("Click to open the context menu for modulating the parameter"));
    bmod_->setFixedSize(butfs, butfs);
    updateModulatorButton();
    if (param_->isModulateable())
        connect(bmod_, SIGNAL(pressed()),
                this, SLOT(openModulationPopup()));

    // reset-to-default button
    but = breset = new QToolButton(this);
    l->addWidget(but);
    but->setIcon(resetIcon);
    but->setVisible(param_->isEditable());
    but->setFixedSize(butfs*0.7, butfs);
    QString defaultValueName;

    // --- float parameter ---
    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param_))
    {
        defaultValueName = QString::number(pf->defaultValue());

        DoubleSpinBox * spin = spinFloat_ = new DoubleSpinBox(this);
        l->addWidget(spin);

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

        setFocusProxy(spin);

        connect(spin, static_cast<void(DoubleSpinBox::*)(double)>(&DoubleSpinBox::valueChanged),
            [this, editor, pf](Double value)
            {
                editor->setParameterValue(pf, value);
            });

        connect(breset, &QToolButton::pressed, [=](){ spin->setValue(pf->defaultValue(), true); });
    }
#if 0
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
            MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << p->idName() << "'");
            if (!scene || !scene->editor()) return;
            scene->editor()->setParameterValue(pi, value);
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
                MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << p->idName() << "'");
                if (!scene || !scene->editor()) return;
                scene->editor()->setParameterValue(ps, value);
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
                MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << p->idName() << "'");
                if (!scene || !scene->editor()) return;
                scene->editor()->setParameterValue(ps, cb->isChecked()? 1 : 0);
            });

            // reset to default
            connect(breset, &QToolButton::pressed, [=]()
            {
                cb->setChecked(ps->defaultValue() != 0);
                Scene * scene = p->object()->sceneObject();
                MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << p->idName() << "'");
                if (!scene || !scene->editor()) return;
                scene->editor()->setParameterValue(ps, ps->defaultValue());
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
            Scene * scene = p->object()->sceneObject();
            MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << p->idName() << "'");
            if (!scene || !scene->editor()) return;
            scene->editor()->setParameterValue(ptl, ptl->getDefaultTimeline());
        });
    }

    else
        MO_ASSERT(false, "could not create widget for Parameter '" << p->idName() << "'");
#endif

    if (defaultValueName.isEmpty())
        breset->setVisible(false);
    else
        breset->setStatusTip(tr("Sets the value of the parameter back to the default value (%1)")
                      .arg(defaultValueName));
}


void ParameterWidget::updateModulatorButton()
{
    static QIcon iconModulateOn(":/icon/modulate_on.png");
    static QIcon iconModulateOff(":/icon/modulate_off.png");

    bmod_->setVisible(param_->isModulateable());

    if (param_->modulatorIds().size())
    {
        bmod_->setDown(true);
        bmod_->setIcon(iconModulateOn);
    }
    else
    {
        bmod_->setDown(false);
        bmod_->setIcon(iconModulateOff);
    }
}

void ParameterWidget::openModulationPopup()
{
    Object * object = param_->object();
    MO_ASSERT(object, "No Object for edit Parameter");
    Scene * scene = object->sceneObject();
    MO_ASSERT(scene, "No Scene for object in Parameter");
    auto * editor = scene->editor();
    MO_ASSERT(editor, "No ObjectEditor assigned for Parameter");

    QMenu * menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction * a;

    // --- parameter float ---

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param_))
    {
        addCreateModMenuFloat_(menu, param_);

        // link to existing modulator
        addLinkModMenu_(menu, param_,
            pf->getModulatorTypes());

        menu->addSeparator();

        // edit modulations
        addEditModMenu_(menu, param_);

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param_);

    }
    else
    if (ParameterInt * pi = dynamic_cast<ParameterInt*>(param_))
    {
        // create modulation
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = editor->createFloatTrack(pi))
            {
                emitObjectSelected_(o);
            }
        });
        // create modulation in clip
        menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float sequence"), menu) );
        QMenu * sub = new QMenu(menu);
        a->setMenu(sub);
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("in new clip"), sub) );
        auto list = Clip::getAssociatedClips(param_, true);
        for (Clip * clip : list)
        {
            sub->addAction( a = new QAction(tr("clip %1").arg(clip->name()), sub) );
            a->setData(QVariant::fromValue(clip));
        }
        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            Clip * parent = a->data().value<Clip*>();
            if (Object * o = editor->createInClip("SequenceFloat", parent))
            {
                // modulate
                editor->addModulator(param_, o->idName());
                o->setName(ObjectEditor::modulatorName(param_));

                emitObjectSelected_(o);
            }
        });

        // link to existing modulator
        addLinkModMenu_(menu, param_,
            pi->getModulatorTypes());

        menu->addSeparator();

        // edit modulations
        addEditModMenu_(menu, param_);

        menu->addSeparator();

        // remove modulation
        addRemoveModMenu_(menu, param_);
    }
    else
        MO_ASSERT(false, "No modulation menu implemented for requested parameter '"
                  << param_->infoIdName() << "'");

    if (menu->isEmpty())
    {
        menu->deleteLater();
        return;
    }

    connect(menu, SIGNAL(destroyed()), this, SLOT(updateModulatorButton()));
    menu->popup(bmod_->mapToGlobal(QPoint(0,0)));
}

void ParameterWidget::addRemoveModMenu_(QMenu * menu, Parameter * param)
{
    Scene * scene = param->object()->sceneObject();
    MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << param->idName() << "'");
    if (!scene || !scene->editor()) return;

    // remove single
    if (param->modulatorIds().size() > 0)
    {
        QMenu * rem = ObjectMenu::createRemoveModulationMenu(param_, menu);
        QAction * a = menu->addMenu(rem);
        a->setText(tr("Remove modulation"));
        a->setStatusTip(tr("Removes individual modulators from this parameter"));
        a->setIcon(QIcon(":/icon/delete.png"));
        connect(rem, &QMenu::triggered, [=](QAction* a)
        {
            scene->editor()->removeModulator(param, a->data().toString());
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
            scene->editor()->removeAllModulators(param);
        });
    }
}


void ParameterWidget::addEditModMenu_(QMenu * menu, Parameter * param)
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

void ParameterWidget::addLinkModMenu_(
        QMenu * menu, Parameter * param, int objectTypeFlags)
{
    Scene * scene = param->object()->sceneObject();
    MO_ASSERT(scene && scene->editor(), "no Scene for Parameter '" << param->idName() << "'");
    if (!scene || !scene->editor()) return;

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
        scene->editor()->addModulator(param, a->data().toString());
    });

}


void ParameterWidget::addCreateModMenuFloat_(QMenu * menu, Parameter * param)
{
    // get model
    MO_ASSERT(param->object() && param->object()->sceneObject()
              && param->object()->sceneObject()->editor()
              , "missing ObjectEditor in modulator menu");
    auto * editor = param->object()->sceneObject()->editor();

    // create track modulation
    QAction * a;
    menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
    connect(a, &QAction::triggered, [=]()
    {
        if (Object * o = editor->createFloatTrack(param))
        {
            emitObjectSelected_(o);
        }
    });

    // create sequence
    menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float sequence"), menu) );
    QMenu * sub = new QMenu(menu);
    a->setMenu(sub);

        // right here
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("right here"), sub) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = editor->createFloatSequenceFor(param_))
            {
                // modulate
                editor->addModulator(param_, o->idName());

                emitObjectSelected_(o);
            }
        });

        sub->addSeparator();

        // in new clip
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("in new clip"), sub) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = editor->createInClip("SequenceFloat", 0))
            {
                // modulate
                editor->addModulator(param, o->idName());
                o->setName(ObjectEditor::modulatorName(param, true));

                emitObjectSelected_(o);
            }
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
            if (Clip * parent = a->data().value<Clip*>())
            if (Object * o = editor->createInClip("SequenceFloat", parent))
            {
                // modulate
                editor->addModulator(param, o->idName());
                o->setName(ObjectEditor::modulatorName(param, true));

                emitObjectSelected_(o);
            }

        });

}

void ParameterWidget::updateWidgetValue()
{
    if (spinInt_)
    {
        if (ParameterInt * pi = dynamic_cast<ParameterInt*>(param_))
            spinInt_->setValue(pi->baseValue());
    }
    else
    if (spinFloat_)
    {
        if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param_))
            spinFloat_->setValue(pf->baseValue());
    }
    else
    if (comboSelect_)
    {
        if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(param_))
        {
            int idx = ps->valueList().indexOf(ps->baseValue());
            if (comboSelect_->currentIndex() != idx)
            {
                comboSelect_->setCurrentIndex(idx);
                // update statustip
                if (comboSelect_->currentIndex() >= 0
                        && comboSelect_->currentIndex() < ps->statusTips().size())
                    comboSelect_->setStatusTip(ps->statusTips().at(
                                                   comboSelect_->currentIndex()));
            }
        }
    }
    else
    if (checkBox_)
    {
        if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(param_))
            checkBox_->setChecked(ps->baseValue());
    }
    else
    if (lineEdit_)
    {
        if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(param_))
            lineEdit_->setText(pfn->value());
        else
        if (ParameterText * ptxt = dynamic_cast<ParameterText*>(param_))
            lineEdit_->setText(ptxt->value());
    }
}

} // namespace GUI
} // namespace MO
