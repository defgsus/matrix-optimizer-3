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
#include "object/interface/valuetextureinterface.h"
#include "tool/generalimage.h"
#include "gl/texture.h"
#include "gl/texturerenderer.h"
#include "io/currenttime.h"
#include "io/log_gui.h"

namespace MO {
namespace GUI {


ObjectOutputView::ObjectOutputView(QWidget *parent)
    : QWidget       (parent)
    , texRender_    (0)
    , imgSize_      (devicePixelRatio() * QSize(128, 128))
{
    setObjectName("ObjectOutputView");
    createWidgets_();
    setObject(0);
}

ObjectOutputView::~ObjectOutputView()
{
    if (texRender_)
        texRender_->releaseGl();
    delete texRender_;
}

void ObjectOutputView::setObject(Object * o)
{
    MO_DEBUG_GUI("ObjectOutputView::setObject(" << (void*)o << ")");

    object_ = o;

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

    for (int i=0; i<20; ++i)
    {
        auto l = new QLabel(this);
        l0->addWidget(l);
        labels_ << l;
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
                (*label)->setVisible(true);
            }
        }
    }

    for (; label != labels_.end(); ++label)
        (*label)->setVisible(false);

    setUpdatesEnabled(true);
}

void ObjectOutputView::setLabel_(QLabel * label, ValueTextureInterface * ti, uint channel, Double time)
{
    if (auto tex = ti->valueTexture(channel, RenderTime(time, MO_GUI_THREAD)))
    {
        // create resampler
        if (!texRender_)
        {
            texRender_ = new GL::TextureRenderer(imgSize_.width(), imgSize_.height() );
        }

        try
        {
#if 1
            // gl-resize
            texRender_->render(tex, true);
            // download image
            if (auto stex = texRender_->texture())
            {
                stex->bind();
                QImage img = stex->toQImage();
                label->setPixmap(QPixmap::fromImage(img));
                return;
            }
#endif
        }
        catch (const Exception& e)
        {
            MO_WARNING("In ObjectOutputView: " << e.what());
        }

        // set error tex
        label->setPixmap(QPixmap::fromImage(GeneralImage::getErrorImage(
                                     tr(" error "), imgSize_)));
    }
    else
        label->clear();
}


} // namespace GUI
} // namespace MO
