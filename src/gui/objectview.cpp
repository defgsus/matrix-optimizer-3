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
#include "tool/stringmanip.h"
#include "parameterview.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/trackfloat.h"
#include "objectinfodialog.h"

namespace MO {
namespace GUI {


ObjectView::ObjectView(QWidget *parent) :
    QWidget (parent),
    object_ (0)
{
    layout_ = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        layout_->addLayout(lh);

            icon_ = new QToolButton(this);
            lh->addWidget(icon_);
            icon_->setToolButtonStyle(Qt::ToolButtonIconOnly);
            connect(icon_, SIGNAL(clicked()), this, SLOT(infoPopup_()));

            label_ = new QLabel(this);
            lh->addWidget(label_);

        label2_ = new QLabel(this);
        layout_->addWidget(label2_);

        paramView_ = new ParameterView(this);
        layout_->addWidget(paramView_);
        connect(paramView_, SIGNAL(objectSelected(MO::Object*)),
                this, SIGNAL(objectSelected(MO::Object*)));
        connect(paramView_, SIGNAL(statusTipChanged(QString)),
                this, SIGNAL(statusTipChanged(QString)));
        connect(paramView_, SIGNAL(objectActivityChanged(MO::Object*)),
                this, SIGNAL(objectActivityChanged(MO::Object*)));
}

void ObjectView::setSceneSettings(SceneSettings *s)
{
    sceneSettings_ = s;
    paramView_->setSceneSettings(s);
}

void ObjectView::setObject(Object * object)
{
    object_ = object;

    if (object_)
    {
        icon_->setIcon(ObjectFactory::iconForObject(object_));
    }
    else
    {
        icon_->setIcon(QIcon());
    }

    updateNameLabel_();

    paramView_->setObject(object_);
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
                    object_->namePath() + "/" + object_->name(), Qt::ElideMiddle, maxWidth);
        QString shortIdPath = fontMetrics().elidedText(
                    object_->idNamePath() + "/" + object_->idName(), Qt::ElideMiddle, maxWidth);

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

} // namespace GUI
} // namespace MO
