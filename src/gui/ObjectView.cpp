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

#include "ObjectView.h"
#include "util/AppIcons.h"
#include "tool/stringmanip.h"
#include "ParameterView.h"
#include "ObjectInfoDialog.h"
#include "widget/ObjectListWidget.h"
#include "widget/ObjectTreeView.h"
#include "model/ObjectTreeModel.h"
#include "object/Object.h"
#include "object/Scene.h"
#include "object/util/ObjectFactory.h"
#include "object/control/TrackFloat.h"
#include "object/interface/ValueTextureInterface.h"
#include "gl/Manager.h"
#include "gl/Texture.h"
#include "io/CurrentTime.h"
#include "io/log.h"
#include "io/log_gui.h"

namespace MO {
namespace GUI {


ObjectView::ObjectView(QWidget *parent)
    : QWidget         (parent)
    , object_         (nullptr)
    , manager_        (nullptr)
{
    setObjectName("_ObjectView");

    layout_ = new QVBoxLayout(this);
    layout_->setMargin(1);

        auto lh = new QHBoxLayout();
        layout_->addLayout(lh);

            labelImg_ = new QLabel(this);
            lh->addWidget(labelImg_);

            icon_ = new QToolButton(this);
            lh->addWidget(icon_);
            icon_->setToolButtonStyle(Qt::ToolButtonIconOnly);
            icon_->setFixedSize(devicePixelRatio() * QSize(48, 48));
            icon_->setIconSize(devicePixelRatio() * QSize(42, 42));
            //icon_->setAutoFillBackground(true);
            connect(icon_, SIGNAL(clicked()), this, SLOT(infoPopup_()));

            label_ = new QLabel(this);
            lh->addWidget(label_, 2);

        label2_ = new QLabel(this);
        layout_->addWidget(label2_);
        /*
        list_ = new ObjectListWidget(this);
        list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        layout_->addWidget(list_);
        connect(list_, SIGNAL(objectSelected(MO::Object*)),
                this, SLOT(onObjectListSelected(MO::Object*)));
        connect(list_, SIGNAL(objectClicked(MO::Object*)),
                this, SLOT(onObjectListClicked(MO::Object*)));
        list_->setVisible(false);
        */
        /*
        treeView_ = new ObjectTreeView(this);
        treeModel_ = new ObjectTreeModel(nullptr, this);
        treeView_->setModel(treeModel_);
        layout_->addWidget(treeView_);
        */

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

ObjectView::~ObjectView()
{

}

void ObjectView::setObject(Object * object)
{
    MO_DEBUG_GUI("ObjectView::setObject(" << (void*)object << ")");

    object_ = object;

    if (object_)
    {
        // XXX not working with qss
        /*auto p = icon_->palette();
        p.setColor(QPalette::Window, ObjectFactory::colorForObject(object_));
        icon_->setPalette(p);
        */
        icon_->setIcon(AppIcons::iconForObject(object_));

        // get GL::Manager
        if (auto s = object_->sceneObject())
        {
            auto m = s->manager();
            if (m && m != manager_)
            {
                connect(m, &GL::Manager::imageFinished, [=](const GL::Texture* ,
                                                            const QString& id,
                                                            const QImage& img)
                {
                    if (id == "objectview")
                        labelImg_->setPixmap(QPixmap::fromImage(img));
                });
            }
            manager_ = m;
        }
    }
    else
    {
        manager_ = nullptr;
        icon_->setIcon(QIcon());
    }

    updateImage();
    updateNameLabel_();

    if (paramView_->object() != object_)
        paramView_->setObject(object_);
    //list_->setParentObject(object_);
    //treeModel_->setRootObject(object_);

    MO_DEBUG_GUI("ObjectView::setObject(" << (void*)object << ") finished");
}

void ObjectView::selectObject(Object * o)
{
    //list_->setSelectedObject(o);
}

void ObjectView::updateParameterVisibility(Parameter * p)
{
    paramView_->updateParameterVisibility(p);
}

void ObjectView::updateParameters()
{
    paramView_->setObject(object_);
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

void ObjectView::updateImage()
{
    const QSize imgs = devicePixelRatio() * QSize(48, 48);
    const uint channel = 0;

    if (object_)
    // object has texture?
    if (auto ti = dynamic_cast<ValueTextureInterface*>(object_))
    {
        if (manager_ && manager_->isWindowVisible())
        //if (auto tex = ti->valueTexture(
        //            channel, RenderTime(CurrentTime::time(), MO_GFX_THREAD)))
        {
            manager_->renderImage(ti, channel, imgs, "objectview");
            return;
        }

        // set black
        labelImg_->setPixmap(QPixmap(imgs));
        return;
    }

    // set empty
    labelImg_->clear();
}

} // namespace GUI
} // namespace MO
