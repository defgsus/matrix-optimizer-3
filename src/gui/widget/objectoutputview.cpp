/** @file objectoutputview.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 16.05.2015</p>
*/

#include <QLayout>
#include <QLabel>

#include "objectoutputview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/interface/valuetextureinterface.h"
#include "tool/generalimage.h"
#include "gl/texture.h"
#include "gl/manager.h"
#include "io/currenttime.h"
#include "io/log_gui.h"

namespace MO {
namespace GUI {


ObjectOutputView::ObjectOutputView(QWidget *parent)
    : QWidget       (parent)
    , object_       (nullptr)
    , manager_      (nullptr)
    , imgSize_      (devicePixelRatio() * QSize(128, 128))
{
    setObjectName("ObjectOutputView");
    createWidgets_();
    setObject(nullptr);
}

ObjectOutputView::~ObjectOutputView()
{
}

void ObjectOutputView::setObject(Object * o)
{
    MO_DEBUG_GUI("ObjectOutputView::setObject(" << (void*)o << ")");

    object_ = o;
    if (!object_)
        manager_ = nullptr;
    else
    if (auto s = object_->sceneObject())
    {
        auto m = s->manager();
        if (m && m != manager_)
        {
            connect(m, &GL::Manager::imageFinished, [=](const GL::Texture* tex,
                                                        const QImage& img)
            {
                for (auto& p : labels_)
                if (p.second == tex)
                {
                    p.first->setPixmap(QPixmap::fromImage(img));
                    break;
                }
            });
            manager_ = m;
        }
    }

    updateLabels_();
}

void ObjectOutputView::updateObject()
{
    if (object_)
        updateLabels_();
}

void ObjectOutputView::createWidgets_()
{
    auto l0 = new QHBoxLayout(this);

    for (int i=0; i<16; ++i)
    {
        auto l = new QLabel(this);
        l0->addWidget(l);
        labels_ << QPair<QLabel*, const GL::Texture*>(l, nullptr);
    }

    l0->addStretch(2);
}

void ObjectOutputView::updateLabels_()
{
    setUpdatesEnabled(false);

    auto label = labels_.begin();

    if (object_)
    {

        Double time = CurrentTime::time();

        auto map = object_->getNumberOutputs();

        // textures
        if (auto ti = dynamic_cast<ValueTextureInterface*>(object_))
        {
            uint num = map.value(ST_TEXTURE);
            for (uint i=0; i<num && label != labels_.end(); ++i, ++label)
            {
                setLabel_(*label, ti, i, time);
                (*label).first->setVisible(true);
            }
        }
    }

    for (; label != labels_.end(); ++label)
        (*label).first->setVisible(false);

    setUpdatesEnabled(true);
}

void ObjectOutputView::setLabel_(QPair<QLabel*,const GL::Texture*>& label,
                                 ValueTextureInterface * ti,
                                 uint channel, Double time)
{
    if (!manager_)
    {
        label.first->clear();
        return;
    }

    if (auto tex = ti->valueTexture(channel, RenderTime(time, MO_GFX_THREAD)))
    {
        label.second = tex;
        manager_->renderImage(tex, imgSize_);
    }
    else
        label.first->clear();
}


} // namespace GUI
} // namespace MO
