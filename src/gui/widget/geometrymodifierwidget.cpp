/** @file geometrymodifierwidget.cpp

    @brief Widget for GeometryModifier classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QIcon>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include "geometrymodifierwidget.h"
#include "io/error.h"
#include "io/files.h"
#include "doublespinbox.h"
#include "spinbox.h"
#include "groupwidget.h"
#include "equationeditor.h"
#include "gui/propertiesview.h"
#include "types/properties.h"
#include "geom/geometry.h"
#ifndef MO_DISABLE_ANGELSCRIPT
#include "geom/geometrymodifierangelscript.h"
#include "script/angelscript_geometry.h"
#include "angelscriptwidget.h"
#endif

namespace MO {
namespace GUI {

GeometryModifierWidget::GeometryModifierWidget(GEOM::GeometryModifier * geom, bool expanded, QWidget *parent) :
    QWidget                 (parent),
    modifier_               (geom),
    funcUpdateFromWidgets_  (0),
    funcUpdateWidgets_      (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    createWidgets_(expanded);

    updateWidgetValues();
}


void GeometryModifierWidget::createWidgets_(bool expanded)
{
    auto l0 = new QVBoxLayout(this);
    l0->setMargin(0);
    group_ = new GroupWidget(modifier_->guiName(), expanded, this);
    l0->addWidget(group_);

    group_->setStatusTip(modifier_->statusTip());
    group_->setHeaderStatusTip(modifier_->statusTip());

    auto butMute = new QToolButton(this);
    group_->addHeaderWidget(butMute);
    butMute->setText(tr("M"));
    butMute->setStatusTip(tr("Enables or disables the modifier while keeping it in the list"));
    butMute->setFixedSize(20,20);
    butMute->setCheckable(true);
    butMute->setChecked(!modifier_->isEnabled());
    connect(butMute, &QToolButton::clicked, [=]()
    {
        emit requestMuteChange(modifier_, butMute->isChecked());
    });

    group_->addHeaderSpacing(5);

    auto butUp = new QToolButton(this);
    group_->addHeaderWidget(butUp);
    butUp->setArrowType(Qt::UpArrow);
    butUp->setFixedSize(20,20);
    butUp->setStatusTip(tr("Moves the modifier up in the list"));
    connect(butUp, &QToolButton::clicked, [=](){ emit requestUp(modifier_); });

    auto butDown = new QToolButton(this);
    group_->addHeaderWidget(butDown);
    butDown->setArrowType(Qt::DownArrow);
    butDown->setFixedSize(20,20);
    butDown->setStatusTip(tr("Moves the modifier down in the list"));
    connect(butDown, &QToolButton::clicked, [=](){ emit requestDown(modifier_); });

    auto butInsert = new QToolButton(this);
    group_->addHeaderWidget(butInsert);
    butInsert->setIcon(QIcon(":/icon/new_letters.png"));
    butInsert->setFixedSize(20,20);
    butInsert->setStatusTip(tr("Creates a new modifier above this one"));
    connect(butInsert, &QToolButton::clicked, [=](){ emit requestInsertNew(modifier_); });

    group_->addHeaderSpacing(5);

    auto butRemove = new QToolButton(this);
    group_->addHeaderWidget(butRemove);
    butRemove->setIcon(QIcon(":/icon/delete.png"));
    butRemove->setFixedSize(20,20);
    butRemove->setStatusTip(tr("Permanently deletes the modifier"));
    connect(butRemove, &QToolButton::clicked, [=](){ emit requestDelete(modifier_); });

    connect(group_, &GroupWidget::expanded, [=]()
    {
        emit expandedChange(modifier_, true);
    });
    connect(group_, &GroupWidget::collapsed, [=]()
    {
        emit expandedChange(modifier_, false);
    });

    propView_ = new PropertiesView();
    propView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    group_->addWidget(propView_);


    propView_->setProperties( modifier_->properties() );
    connect(propView_, SIGNAL(propertyChanged(QString)),
            this, SLOT(updateFromWidgets_()));
    funcUpdateFromWidgets_ = [=]()
    {
        modifier_->properties() = propView_->properties();
    };

    funcUpdateWidgets_ = [=]()
    {
        propView_->setProperties( modifier_->properties() );
    };


#if 0
#ifndef MO_DISABLE_ANGELSCRIPT
    if (auto script = dynamic_cast<GEOM::GeometryModifierAngelScript*>(modifier_))
    {
        auto edit = new AngelScriptWidget(this);
        //edit->setMinimumHeight(500);
        edit->setUpdateOptional();
        group_->addWidget(edit);
        edit->setScriptEngine( GeometryEngineAS::createNullEngine(true) );
        connect(edit, SIGNAL(scriptTextChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            script->setScript(edit->scriptText());
        };

        funcUpdateWidgets_ = [=]()
        {
            edit->setScriptText(script->script());
        };
    }
#endif
#endif
}


void GeometryModifierWidget::updateWidgetValues()
{
    MO_ASSERT(funcUpdateWidgets_, "no update function defined");

    if (funcUpdateWidgets_)
        funcUpdateWidgets_();

}

void GeometryModifierWidget::updateFromWidgets_()
{
    MO_ASSERT(funcUpdateFromWidgets_, "no update function defined");

    if (funcUpdateFromWidgets_)
        funcUpdateFromWidgets_();

    emit valueChanged(modifier_);
}


} // namespace GUI
} // namespace MO
