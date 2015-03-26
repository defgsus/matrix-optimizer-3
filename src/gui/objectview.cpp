/** @file objectview.cpp

    @brief Object display / editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/


#include <QIcon>
#include <QToolButton>
#include <QLayout>
#include <QLabel>
#include <QDialog>


#include "objectview.h"
#include "util/appicons.h"
#include "tool/stringmanip.h"
#include "parameterview.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/trackfloat.h"
#include "objectinfodialog.h"
#include "widget/objectlistwidget.h"

namespace MO {
namespace GUI {


ObjectView::ObjectView(QWidget *parent) :
    QWidget (parent),
    object_ (0)
{
    setObjectName("_ObjectView");

    layout_ = new QVBoxLayout(this);
    layout_->setMargin(1);

        auto lh = new QHBoxLayout();
        layout_->addLayout(lh);

            icon_ = new QToolButton(this);
            lh->addWidget(icon_);
            icon_->setToolButtonStyle(Qt::ToolButtonIconOnly);
            icon_->setFixedSize(devicePixelRatio() * QSize(48, 48));
            icon_->setIconSize(devicePixelRatio() * QSize(42, 42));
            //icon_->setAutoFillBackground(true);
            connect(icon_, SIGNAL(clicked()), this, SLOT(infoPopup_()));

            label_ = new QLabel(this);
            lh->addWidget(label_);

        label2_ = new QLabel(this);
        layout_->addWidget(label2_);

        list_ = new ObjectListWidget(this);
        list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        layout_->addWidget(list_);
        connect(list_, SIGNAL(objectSelected(MO::Object*)),
                this, SLOT(onObjectListSelected(MO::Object*)));
        connect(list_, SIGNAL(objectClicked(MO::Object*)),
                this, SLOT(onObjectListClicked(MO::Object*)));

        paramView_ = new ParameterView(this);
        paramView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout_->addWidget(paramView_);
        connect(paramView_, SIGNAL(objectSelected(MO::Object*)),
                this, SIGNAL(objectSelected(MO::Object*)));
        connect(paramView_, SIGNAL(statusTipChanged(QString)),
                this, SIGNAL(statusTipChanged(QString)));
        connect(paramView_, SIGNAL(objectActivityChanged(MO::Object*)),
                this, SIGNAL(objectActivityChanged(MO::Object*)));
}

void ObjectView::setObject(Object * object)
{
    object_ = object;

    if (object_)
    {
        // XXX no f**ng working
        /*auto p = icon_->palette();
        p.setColor(QPalette::Window, ObjectFactory::colorForObject(object_));
        icon_->setPalette(p);
        */
        icon_->setIcon(AppIcons::iconForObject(object_));
    }
    else
    {
        icon_->setIcon(QIcon());
    }

    updateNameLabel_();

    paramView_->setObject(object_);
    list_->setParentObject(object_);
}

void ObjectView::selectObject(Object * o)
{
    list_->setSelectedObject(o);
}

void ObjectView::updateParameterVisibility(Parameter * p)
{
    paramView_->updateParameterVisibility(p);
}

void ObjectView::updateObjectName()
{
    updateNameLabel_();
}

void ObjectView::updateNameLabel_()
{
    if (object_)
    {
        // XXX what is a good value here?
        int maxWidth = width() - icon_->width() - 30;
        QString shortName = fontMetrics().elidedText(
                    object_->name(), Qt::ElideMiddle, maxWidth);
        QString shortPath = fontMetrics().elidedText(
                    object_->namePath(), Qt::ElideMiddle, maxWidth);
        QString shortIdPath = fontMetrics().elidedText(
                    object_->idNamePath(), Qt::ElideMiddle, maxWidth);

        label_->setText(QString("<html><b>%1</b><br/>%2<br/>%3</html>")
                        .arg(shortName)
                        .arg(shortPath)
                        .arg(shortIdPath)
                        );

        // additional info
        QString info;
        /*
        QList<Object*> mods = object_->getModulatingObjects();
        for (auto m : mods)
            info += m->idName() + " ";
        */
        /*if (TrackFloat * track = qobject_cast<TrackFloat*>(object_))
        {
            for (auto &s : track->sequenceIds())
                info += s + "\n";
        }*/

        label2_->setText(info);
    }
    else
    {
        label_->setText(QString());
        label2_->setText(QString());
    }
}

void ObjectView::resizeEvent(QResizeEvent *)
{
    if (object_)
        updateNameLabel_();
}

void ObjectView::infoPopup_()
{
    if (!object_)
        return;

    ObjectInfoDialog diag;

    diag.setObject(object_);

    diag.exec();
}

void ObjectView::onObjectListSelected(Object * o)
{
    setObject(o);
    emit objectSelected(o);
}

void ObjectView::onObjectListClicked(Object * o)
{
    paramView_->setObject(o);
    emit objectSelected(o);
}

} // namespace GUI
} // namespace MO
