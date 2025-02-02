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
#include <QMenu>
#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QFontComboBox>

#include "ParameterWidget.h"
#include "SpinBox.h"
#include "DoubleSpinBox.h"
#include "EditableLabel.h"
#include "object/Scene.h"
#include "object/control/TrackFloat.h"
#include "object/control/SequenceFloat.h"
#include "object/control/Clip.h"
#include "object/util/ObjectEditor.h"
#include "object/visual/ObjectGl.h"
#include "object/param/ParameterCallback.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterImageList.h"
#include "object/param/ParameterFilename.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterFloatMatrix.h"
#include "object/param/ParameterFont.h"
#include "object/param/ParameterGeometry.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "object/param/ParameterTexture.h"
#include "object/param/ParameterTransformation.h"
#include "object/param/ParameterTimeline1d.h"
#include "object/param/Modulator.h"
#include "gui/ModulatorDialog.h"
#include "gui/widget/TextEditWidget.h"
#include "gui/util/ObjectMenu.h"
#include "gui/item/FrontFloatItem.h"
#include "model/ObjectMimeData.h"
#include "tool/enumnames.h"
#include "io/Application.h"
#include "io/error.h"
#include "io/log.h"

Q_DECLARE_METATYPE(MO::Modulator*)

namespace MO {
namespace GUI {

ParameterWidget::ParameterWidget(Parameter * p, QWidget *parent)
    :   QFrame      (parent),
        param_      (p),
        editor_     (0),
        bmod_       (0),
        btexmm_     (0),
        spinInt_    (0),
        spinFloat_  (0),
        comboSelect_(0),
        checkBox_   (0),
        lineEdit_   (0),
        doChangeToCreatedMod_   (true)
{
    setObjectName("_PW_" + param_->idName());
    setFrameStyle(QFrame::Panel);
    setFrameShadow(QFrame::Sunken);
    setAcceptDrops(true);

    if (param_->object()
            && param_->object()->sceneObject())
    editor_ = param_->object()->sceneObject()->editor();

    createWidgets_();
}

void ParameterWidget::emitObjectSelected_(Object * o)
{
    if (doChangeToCreatedMod_)
        emit objectSelected(o);
}

void ParameterWidget::emitStatusTipChanged_(const QString & s)
{
    emit statusTipChanged(s);
}

void ParameterWidget::dragEnterEvent(QDragEnterEvent * e)
{
    // another object?
    if (e->mimeData()->hasFormat(ObjectMimeData::mimeTypeString))
    {
        // construct a wrapper
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        auto desc = data->getDescription();

        // analyze further
        if (!desc.isSameApplicationInstance())
            return;

        if (desc.pointer() == param_->object())
            return;

        if (param_->getModulatorTypes() & desc.type())
        {
            e->setDropAction(Qt::LinkAction);
            e->accept();
            return;
        }
    }

#ifndef MO_DISABLE_FRONT
    if (auto idata = FrontItemMimeData::frontItemMimeData(e->mimeData()))
    {
        if (   !param_->isModulateable()
            || !idata->isSameApplicationInstance()
            || !(idata->modulatorType() & param_->getModulatorTypes())
                )
            return;

        e->setDropAction(Qt::LinkAction);
        e->accept();
        return;
    }
#endif
}

void ParameterWidget::dropEvent(QDropEvent * e)
{
    // analyze mime data
    if (e->mimeData()->hasFormat(ObjectMimeData::mimeTypeString))
    {
        // construct a wrapper
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        auto desc = data->getDescription();

        if (!editor_)
            return;

        // create modulation
        editor_->addModulator(param_, desc.id(), "");

        // select the parameter's object
        if (param_->object())
            emitObjectSelected_(param_->object());

        e->accept();
        return;
    }

#ifndef MO_DISABLE_FRONT
    // drop of a ui-item
    if (auto idata = FrontItemMimeData::frontItemMimeData(e->mimeData()))
    {
        if (!param_->isModulateable())
            return;

        if (!idata->isSameApplicationInstance())
            return;

        if (!(idata->modulatorType() & param_->getModulatorTypes()))
            return;

        auto item = idata->getItem();

        editor_->addUiModulator(param_, item);

        e->accept();
        //MO_DEBUG("ui-item " << idata->getItemId() << " -> param " << param_->name());
    }
#endif
}

/** Long and tedious routine.. */
void ParameterWidget::createWidgets_()
{
    if (!editor_)
        return;

    setToolTip("id: '" + param_->idName() + "'");

    static const QIcon resetIcon(":/icon/reset.png");

    QHBoxLayout * l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(4);

    const int butfs = 25;

    // modulate button
    bmod_ = new QToolButton(this);
    l->addWidget(bmod_);
    bmod_->setStatusTip(tr("Click to open the context menu for modulating the parameter"));
    bmod_->setFixedSize(butfs, butfs);
    if (param_->isModulateable())
        connect(bmod_, SIGNAL(pressed()),
                this, SLOT(openModulationPopup()));

    // visible (in graph) button
    bvis_ = new QToolButton(this);
    l->addWidget(bvis_);
    bvis_->setStatusTip(tr("Click to open the context menu for the visibility of the parameter in other views"));
    bvis_->setFixedSize(butfs, butfs);
    bvis_->setCheckable(true);
#if 0
    connect(bvis_, SIGNAL(pressed()), this, SLOT(openVisibilityPopup()));
#else
    connect(bvis_, &QToolButton::clicked, [=]()
    {
        editor_->setParameterVisibleInGraph(param_, !param_->isVisibleInGraph());
        updateButtons();
    });
#endif

    // label
    /*QLabel * label = new QLabel(param_->name(), this);
    l->addWidget(label);
    label->setStatusTip(param_->statusTip().isEmpty()
                        ? tr("The name of the parameter")
                        : param_->statusTip());*/
    auto label = new EditableLabel(param_->userName().isEmpty()
                                     ? param_->name() : param_->userName(), this);
    l->addWidget(label);
    label->setStatusTip(param_->statusTip().isEmpty()
                        ? tr("The name of the parameter, double-click to change")
                        : param_->statusTip());
    connect(label, &EditableLabel::editingFinished, [=]()
    {
        editor_->setParameterUserName(param_, label->text());
    });

    l->addStretch(1);

    QToolButton * but, * breset;

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
        defaultValueName = pf->isDefaultFractional()
                ? pf->defaultValueFraction().toString()
                : QString::number(pf->defaultValue());

        DoubleSpinBox * spin = spinFloat_ = new DoubleSpinBox(this);
        l->addWidget(spin);

        spin->setMinimum(pf->minValue());
        spin->setMaximum(pf->maxValue());
        spin->setDecimals(6);
        spin->setSingleStep(pf->smallStep());
        if (pf->isFractional())
            spin->setValue(pf->baseValueFraction());
        else
            spin->setValue(pf->baseValue());
        spin->lineEdit()->setMaximumWidth(120);
        spin->setEnabled(pf->isEditable());
        spin->setStatusTip(pf->statusTip().isEmpty()
                           ? tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons")
                           : pf->statusTip());

        setFocusProxy(spin);

        connect(spin, &DoubleSpinBox::valueChanged, [this, pf, spin](Double value)
        {
            if (spin->isFractional())
                editor_->setParameterValue(pf, spin->fraction());
            else
                editor_->setParameterValue(pf, value);
        });

        connect(breset, &QToolButton::pressed, [=]()
        {
            if (pf->isDefaultFractional())
                spin->setValue(pf->defaultValueFraction(), true);
            else
                spin->setValue(pf->defaultValue(), true);
        });
    }
    else

    // --- int parameter ---
    if (ParameterInt * pi = dynamic_cast<ParameterInt*>(param_))
    {
        if (pi->specificFlag() == Parameter::SF_KEYCODE)
        {
            defaultValueName = enumName( Qt::Key( pi->defaultValue() ) );

            auto combo = new QComboBox(this);
            l->addWidget(combo);

            for (auto i = keycodeNames().begin(); i != keycodeNames().end(); ++i)
            {
                combo->addItem(i.value(), i.key());
            }
            combo->setCurrentText(enumName( Qt::Key( pi->baseValue() ) ));
            combo->setEnabled(pi->isEditable());
            combo->setStatusTip(pi->statusTip().isEmpty()
                               ? tr("Select the keycode")
                               : pi->statusTip());

            setFocusProxy(combo);

            connect(combo, static_cast<void(QComboBox::*)(int)>(
                        &QComboBox::currentIndexChanged), [=](int idx)
            {
                editor_->setParameterValue(pi, combo->itemData(idx).toInt());
            });

            connect(breset, &QToolButton::pressed, [=]()
            {
                combo->setCurrentText(enumName( Qt::Key( pi->baseValue() ) ));
            });
        }
        else
        {
            defaultValueName = QString::number(pi->defaultValue());

            SpinBox * spin = spinInt_ = new SpinBox(this);
            l->addWidget(spin);

            spin->setMinimum(pi->minValue());
            spin->setMaximum(pi->maxValue());
            spin->setSingleStep(pi->smallStep());
            spin->setValue(pi->baseValue());
            spin->lineEdit()->setMaximumWidth(120);
            spin->setEnabled(pi->isEditable());
            spin->setStatusTip(pi->statusTip().isEmpty()
                               ? tr("Edit with keyboard, scroll with mouse-wheel "
                                    "or use the up/down buttons")
                               : pi->statusTip());

            setFocusProxy(spin);

            connect(spin, &SpinBox::valueChanged, [=](int value)
            {
                editor_->setParameterValue(pi, value);
            });

            connect(breset, &QToolButton::pressed, [=](){ spin->setValue(pi->defaultValue(), true); });
        }
    }
    else

    // --- select parameter ---
    if (ParameterSelect * ps = dynamic_cast<ParameterSelect*>(param_))
    {
        defaultValueName = ps->defaultValueName();

        if (!ps->isBoolean())
        {
            QComboBox * combo = comboSelect_ = new QComboBox(this);
            l->addWidget(combo);

            combo->setEnabled(ps->isEditable());

            // fill combobox with value names
            for (auto & name : ps->valueNames())
                combo->addItem(name);

            // set index and statustip
            combo->setCurrentIndex(ps->valueList().indexOf(ps->baseValue()));
            if (combo->currentIndex() >= 0 && combo->currentIndex() < ps->statusTips().size())
                combo->setStatusTip(ps->statusTips().at(combo->currentIndex()));

            setFocusProxy(combo);

            connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx)
            {
                if (idx<0 || idx >= ps->valueList().size())
                    return;
                // update statustip of combobox
                if (idx >= 0 && idx < ps->statusTips().size())
                    combo->setStatusTip(ps->statusTips().at(idx));

                // get value
                int value = ps->valueList().at(combo->currentIndex());
                editor_->setParameterValue(ps, value);
            });

            // statustips from combobox items
            connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::highlighted), [=](int idx)
            {
                if (idx >= 0 && idx < ps->statusTips().size())
                {
                    combo->view()->setStatusTip(ps->statusTips().at(idx));
                    // need to emit explicity for statusbar to update
                    emitStatusTipChanged_(combo->view()->statusTip());
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
            QCheckBox * cb = checkBox_ = new QCheckBox(this);
            l->addWidget(cb);

            cb->setEnabled(ps->isEditable());
            cb->setStatusTip(ps->statusTip());
            cb->setChecked(ps->baseValue() != 0);

            setFocusProxy(cb);

            connect(cb, &QCheckBox::clicked, [=]()
            {
                editor_->setParameterValue(ps, cb->isChecked()? 1 : 0);
            });

            // reset to default
            connect(breset, &QToolButton::pressed, [=]()
            {
                cb->setChecked(ps->defaultValue() != 0);
                editor_->setParameterValue(ps, ps->defaultValue());
            });
        }
    }
    else

    // --- filename parameter ---
    if (ParameterFilename * pfn = dynamic_cast<ParameterFilename*>(param_))
    {
        defaultValueName = pfn->defaultValue();

        QLineEdit * edit = lineEdit_ = new QLineEdit(this);
        edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        l->addWidget(edit);

        // XXX
        breset->setVisible(false);

        edit->setReadOnly(true);
        edit->setStatusTip(pfn->statusTip());
        edit->setText(pfn->value());

        // load button
        QToolButton * butload = new QToolButton(this);
        l->addWidget(butload);
        butload->setText("...");
        butload->setStatusTip(tr("Click to select a file"));

        setFocusProxy(butload);

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

    // --- font parameter ---
    if (ParameterFont * pf = dynamic_cast<ParameterFont*>(param_))
    {
        defaultValueName = pf->baseValue().toString();

        auto combo = new QFontComboBox(this);
        l->addWidget(combo);

        setFocusProxy(combo);

        connect(combo, &QFontComboBox::currentFontChanged, [=](const QFont& f)
        {
            editor_->setParameterValue(pf, f);
        });

        connect(breset, &QToolButton::pressed, [=]()
        {
            combo->setCurrentFont( pf->defaultValue() );
        });
    }
    else
    // --- imagelist parameter ---
    if (ParameterImageList * pil = dynamic_cast<ParameterImageList*>(param_))
    {
        defaultValueName = pil->defaultValueText();

        QLineEdit * edit = lineEdit_ = new QLineEdit(this);
        l->addWidget(edit);

        edit->setReadOnly(true);
        edit->setStatusTip(pil->statusTip());
        edit->setText(pil->valueText());

        setFocusProxy(edit);

        // load button
        QToolButton * butload = new QToolButton(this);
        l->addWidget(butload);
        butload->setText("...");
        butload->setStatusTip(tr("Click to select image files"));

        // load button click
        connect(butload, &QToolButton::clicked, [=]()
        {
            pil->openFileDialog(this);
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            editor_->setParameterValue(pil, pil->defaultValue());
        });
    }
    else

    // --- text parameter ---
    if (ParameterText * ptxt = dynamic_cast<ParameterText*>(param_))
    {
        defaultValueName = ptxt->defaultValue();

        QLineEdit * edit = lineEdit_ = new QLineEdit(this);
        edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        l->addWidget(edit);

        edit->setStatusTip(ptxt->statusTip());
        edit->setText(ptxt->baseValue());

        edit->setReadOnly( !ptxt->isOneliner() );

        // edit button
        QToolButton * butedit = new QToolButton(this);
        l->addWidget(butedit);
        butedit->setText("...");
        butedit->setStatusTip(tr("Click to edit the text"));

        setFocusProxy(ptxt->isOneliner() ? (QWidget*)butedit : (QWidget*)edit);

        // edit button click
        connect(butedit, &QToolButton::clicked, [=]()
        {
#if 1
            ptxt->openEditDialog();// application()->mainWindow() );
#else
            // XXX works but is not fully integrated yet
            auto w = ptxt->createEditWidget( application()->mainWindow() );
            application()->createDockWidget("parameter " + ptxt->infoName(), w);
#endif
        });

        // pure edit
        if (!edit->isReadOnly())
        connect(edit, &QLineEdit::editingFinished, [=]()
        {
            editor_->setParameterValue(ptxt, edit->text());
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            editor_->setParameterValue(ptxt, ptxt->defaultValue());
        });
    }

    else
    // --- timeline1d parameter ---
    if (ParameterTimeline1D * ptl = dynamic_cast<ParameterTimeline1D*>(param_))
    {
        defaultValueName = ptl->defaultTimeline() ? "..." : "empty";

        // edit button
        QToolButton * butedit = new QToolButton(this);
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
            editor_->setParameterValue(ptl, ptl->getDefaultTimeline());
        });
    }

    else
    // --- callback parameter ---
    if (ParameterCallback * pc = dynamic_cast<ParameterCallback*>(param_))
    {
        //defaultValueName = QString::number(pf->defaultValue());

        auto but = new QToolButton(this);
        but->setStatusTip(pc->statusTip());
        l->addWidget(but, -1);

        setFocusProxy(but);

        connect(but, &QToolButton::clicked, [this, pc]()
        {
            // TODO: make it always come from GUI thread
            pc->fire();
            emit editor_->parameterChanged(pc);
        });
    }

    else
    // --- texture parameter ---
    if (ParameterTexture* pt = dynamic_cast<ParameterTexture*>(param_))
    {
        l->addStretch(1);
        addTexParamButtons_(pt, l);
        l->addStretch(1);
    }
    else
    // --- geometry parameter ---
    if (/*ParameterGeometry* pt = */dynamic_cast<ParameterGeometry*>(param_))
    {

    }
    else
    // --- float matrix parameter ---
    if (ParameterFloatMatrix* pfm = dynamic_cast<ParameterFloatMatrix*>(param_))
    {
        defaultValueName = QString::fromStdString(
                    pfm->defaultValue().layoutString() );

        // edit button
        QToolButton * butedit = new QToolButton(this);
        l->addWidget(butedit);
        butedit->setText("...");
        butedit->setStatusTip(tr("Click to edit the float matrix"));

        // load button click
        connect(butedit, &QToolButton::clicked, [=]()
        {
            pfm->openEditDialog(this);
        });

        // reset to default
        connect(breset, &QToolButton::pressed, [=]()
        {
            editor_->setParameterValue(pfm, pfm->defaultValue());
        });

    }
    else
    // --- transformation parameter ---
    if (/*ParameterTransformation* pt = */
            dynamic_cast<ParameterTransformation*>(param_))
    {

    }

    else
        MO_ASSERT(false, "could not create widget for Parameter '" << param_->idName() << "'");


    if (defaultValueName.isNull())
        breset->setVisible(false);
    else
        breset->setStatusTip(tr("Sets the value of the parameter back to the default value (%1)")
                      .arg(defaultValueName));

    updateButtons();
}

namespace
{
    template <class ENUM>
    void texParamPopup_(QWidget* parent,
                        ParameterTexture* param,
                        const QString& title,
                        const QStringList& names,
                        const QList<ENUM>& values,
                        ENUM curValue,
                        std::function<void(ENUM)> func)
    {
        auto pop = new QMenu(parent);
        pop->setAttribute(Qt::WA_DeleteOnClose);
        auto a = pop->addAction(title);
        auto font = a->font();
        font.setBold(true);
        a->setFont(font);
        pop->addSeparator();
        for (int i=0; i<values.size(); ++i)
        {
            a = pop->addAction(names[i]);
            a->setData(i);
            if (values[i] == curValue)
                { a->setCheckable(true); a->setChecked(true); }
            ParameterWidget::connect(a, &QAction::triggered, [=]()
            {
                func(values[i]);

                // XXX Replicate an actual Parameter change
                // We are only manipulating ParameterTexture values
                // that are outside of the signal scope
                // XXX This could be refactured into an interface
                //     to Parameter/ObjectEditor
                if (auto o = param->object())
                {
                    o->onParameterChanged(param);
                    o->updateParameterVisibility();
                    if (auto e = o->editor())
                        emit e->parameterChanged(param);

                    if (auto s = o->sceneObject())
                        s->render();
                }

                if (ObjectGl* gl = dynamic_cast<ObjectGl*>(param->object()))
                    gl->requestRender();
            });
        }
        pop->exec(QCursor::pos());
    }
}

void ParameterWidget::addTexParamButtons_(ParameterTexture* param, QHBoxLayout* l)
{
    if (param->inputType() != ParameterTexture::IT_INTERNAL)
    {
        auto but = new QToolButton(this);
        but->setText(param->inputTypeNames[param->inputType()]);
        l->addWidget(but);
        if (param->isInputTypeSelectable())
        connect(but, &QToolButton::clicked, [=]()
        {
            texParamPopup_<ParameterTexture::InputType>(
                            this, param, tr("texture source"),
                            ParameterTexture::inputTypeNames,
                            ParameterTexture::inputTypeValues,
                            param->inputType(),
                            [=](ParameterTexture::InputType m)
            {
                param->setInputType(m);
                but->setText(param->inputTypeNames[param->inputType()]);
                btexmm_->setVisible( m == ParameterTexture::IT_FILE );

                if (m == ParameterTexture::IT_INPUT)
                    param->setVisibleGraph(true);
            } );
        });
    }

    auto but = new QToolButton(this);
    but->setText(param->magModeNames[param->magMode()]);
    l->addWidget(but);
    connect(but, &QToolButton::clicked, [=]()
    {
        texParamPopup_<ParameterTexture::MagMode>(this, param, tr("magnification"),
                       ParameterTexture::magModeNames,
                       ParameterTexture::magModeValues,
                       param->magMode(),
                       [=](ParameterTexture::MagMode m)
        {
            param->setMagMode(m);
            but->setText(param->magModeNames[param->magMode()]);
        } );
    });

    but = new QToolButton(this);
    but->setText(param->minModeNamesShort[param->minMode()]);
    l->addWidget(but);
    connect(but, &QToolButton::clicked, [=]()
    {
        texParamPopup_<ParameterTexture::MinMode>(this, param, tr("minification"),
                       ParameterTexture::minModeNames,
                       ParameterTexture::minModeValues,
                       param->minMode(),
                       [=](ParameterTexture::MinMode m)
        {
            param->setMinMode(m);
            but->setText(param->minModeNamesShort[param->minMode()]);
        } );
    });

    but = new QToolButton(this);
    but->setText(param->wrapModeNames[param->wrapModeX()]);
    l->addWidget(but);
    connect(but, &QToolButton::clicked, [=]()
    {
        texParamPopup_<ParameterTexture::WrapMode>(this, param, tr("edge x"),
                       ParameterTexture::wrapModeNames,
                       ParameterTexture::wrapModeValues,
                       param->wrapModeX(),
                       [=](ParameterTexture::WrapMode m)
        {
            param->setWrapModeX(m);
            but->setText(param->wrapModeNames[param->wrapModeX()]);
        } );
    });


    but = new QToolButton(this);
    but->setText(param->wrapModeNames[param->wrapModeY()]);
    l->addWidget(but);
    connect(but, &QToolButton::clicked, [=]()
    {
        texParamPopup_<ParameterTexture::WrapMode>(this, param, tr("edge y"),
                       ParameterTexture::wrapModeNames,
                       ParameterTexture::wrapModeValues,
                       param->wrapModeY(),
                       [=](ParameterTexture::WrapMode m)
        {
            param->setWrapModeY(m);
            but->setText(param->wrapModeNames[param->wrapModeY()]);
        } );
    });

    but = new QToolButton(this);
    but->setText(param->wrapModeNames[param->wrapModeZ()]);
    l->addWidget(but);
    connect(but, &QToolButton::clicked, [=]()
    {
        texParamPopup_<ParameterTexture::WrapMode>(this, param, tr("edge z"),
                       ParameterTexture::wrapModeNames,
                       ParameterTexture::wrapModeValues,
                       param->wrapModeZ(),
                       [=](ParameterTexture::WrapMode m)
        {
            param->setWrapModeZ(m);
            but->setText(param->wrapModeNames[param->wrapModeZ()]);
        } );
    });

    if (param->inputType() != ParameterTexture::IT_INTERNAL)
    {
        QStringList names = { tr("no mipmap") };
        QList<uint> values = { 0 };
        for (uint i=1; i<10; ++i)
        {
            names << tr("%1").arg(i);
            values << i;
        }

        but = btexmm_ = new QToolButton(this);
        but->setText(names[param->mipmaps()]);
        l->addWidget(but);
        connect(but, &QToolButton::clicked, [=]()
        {
            texParamPopup_<uint>(this, param, tr("max generated mipmaps"),
                           names, values,
                           param->mipmaps(),
                           [=](uint l)
            {
                param->setMipmaps(l);
                but->setText(names[l]);
            } );
        });

        but->setVisible(param->inputType() == ParameterTexture::IT_FILE);
    }
}

void ParameterWidget::updateButtons()
{
    static QIcon iconModulateOn(":/icon/modulate_on.png");
    static QIcon iconModulateOff(":/icon/modulate_off.png");
    static QIcon iconVisibility(":/icon/visibility.png");
    static QIcon iconVisibilityOn(":/icon/visibility_on.png");

    bmod_->setEnabled(param_->isModulateable());
    bvis_->setEnabled(param_->isModulateable());

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

    if (param_->isVisibleInGraph() || param_->isVisibleInterface())
    {
        bvis_->setChecked(true);
        bvis_->setIcon(iconVisibilityOn);
    }
    else
    {
        bvis_->setChecked(false);
        bvis_->setIcon(iconVisibility);
    }
}

void ParameterWidget::openModulationPopup()
{
    if (!editor_)
        return;

    QMenu * menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addSection(tr("modulation for %1").arg(param_->name()));

    // --- float inputs ---

    if (dynamic_cast<ParameterFloat*>(param_)
      || dynamic_cast<ParameterInt*>(param_)
      || dynamic_cast<ParameterCallback*>(param_))
    {
        addCreateModMenuFloat_(menu, param_);

        // link to existing modulator
        addLinkModMenu_(menu, param_,
            param_->getModulatorTypes());

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

    connect(menu, SIGNAL(destroyed()), this, SLOT(updateButtons()));
    menu->popup(bmod_->mapToGlobal(QPoint(0,0)));
}

void ParameterWidget::addRemoveModMenu_(QMenu * menu, Parameter * param)
{
    if (!editor_)
        return;

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
            QString ids = a->data().toString();
            MO_DEBUG(ObjectMenu::getModulatorId(ids)<<" '"<<ObjectMenu::getOutputId(ids)<<"'");
            editor_->removeModulator(param, ObjectMenu::getModulatorId(ids),
                                            ObjectMenu::getOutputId(ids));
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
            editor_->removeAllModulators(param);
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
    if (!editor_)
        return;

    QMenu * linkMenu = ObjectMenu::createObjectChildMenu(
                param->object()->sceneObject(), objectTypeFlags, menu);
    if (linkMenu->isEmpty())
    {
        linkMenu->deleteLater();
        return;
    }

    // disable the entries that are already modulators
    QStringList ids;
    for (auto p : param->modulatorIds())
        ids << p.first;
    ObjectMenu::setEnabled(linkMenu, ids, false);

    QAction * a = menu->addMenu(linkMenu);
    a->setText(tr("Choose existing source"));
    a->setIcon(QIcon(":/icon/modulate_on.png"));
    connect(linkMenu, &QMenu::triggered, [=](QAction* a)
    {
        editor_->addModulator(param, a->data().toString(), "");
    });

}


void ParameterWidget::addCreateModMenuFloat_(QMenu * menu, Parameter * param)
{
    if (!editor_)
        return;

    // create track modulation
    QAction * a;
    menu->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("Create new float track"), menu) );
    connect(a, &QAction::triggered, [=]()
    {
        if (Object * o = editor_->createFloatTrack(param))
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
            if (Object * o = editor_->createFloatSequenceFor(param_))
            {
                // modulate
                editor_->addModulator(param_, o->idName(), "");

                emitObjectSelected_(o);
            }
        });

        sub->addSeparator();

        // in new clip
        sub->addAction( a = new QAction(QIcon(":/icon/new.png"), tr("in new clip"), sub) );
        connect(a, &QAction::triggered, [=]()
        {
            if (Object * o = editor_->createInClip(param, "SequenceFloat", 0))
            {
                // modulate
                editor_->addModulator(param, o->idName(), "");
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
            a->setData(QVariant::fromValue((void*)clip));
        }

        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            if (Clip * parent = (Clip*)a->data().value<void*>())
            if (Object * o = editor_->createInClip(param, "SequenceFloat", parent))
            {
                // modulate
                editor_->addModulator(param, o->idName(), "");
                o->setName(ObjectEditor::modulatorName(param, true));

                emitObjectSelected_(o);
            }

        });

}

void ParameterWidget::openVisibilityPopup()
{
    if (!editor_)
        return;

    QMenu * menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction * a;

    a = menu->addAction(tr("show connector"));
    a->setCheckable(true);
    a->setChecked(param_->isVisibleInGraph());
    connect(a, &QAction::triggered, [=]()
    {
        editor_->setParameterVisibleInGraph(param_, !param_->isVisibleInGraph());
        updateButtons();
    });
#ifndef MO_DISABLE_EXP
    a = menu->addAction(tr("show in interface"));
    a->setCheckable(true);
    a->setChecked(param_->isVisibleInterface());
    connect(a, &QAction::triggered, [=]()
    {
        editor_->setParameterVisibleInterface(param_, !param_->isVisibleInterface());
        updateButtons();
    });
#endif
    menu->popup(QCursor::pos());
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
        {
            if (pf->isFractional())
                spinFloat_->setValue(pf->baseValueFraction());
            else
                spinFloat_->setValue(pf->baseValue());
        }
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

    updateButtons();
}

} // namespace GUI
} // namespace MO
