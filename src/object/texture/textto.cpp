/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/21/2015</p>
*/

#include <QImage>
#include <QPainter>

#include "textto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parametertext.h"
#include "object/param/parameterselect.h"
#include "gl/texture.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(TextTO)

TextTO::TextTO(QObject *parent)
    : TextureObjectBase (parent)
    , text_             ("texti")
    , pText_            (0)
    , tex_              (0)
    , doRenderText_     (true)
{
    setName("Text");
    initMaximumTextureInputs(0);
}

TextTO::~TextTO()
{
    delete tex_;
}

void TextTO::serialize(IO::DataStream & io) const
{
    TextureObjectBase::serialize(io);
    io.writeHeader("totxt", 1);
}

void TextTO::deserialize(IO::DataStream & io)
{
    TextureObjectBase::deserialize(io);
    io.readHeader("totxt", 1);
}

void TextTO::createParameters()
{
    TextureObjectBase::createParameters();

    params()->beginParameterGroup("text", tr("text"));
    initParameterGroupExpanded("text");

        pText_ = params()->createTextParameter(
                    "text", tr("text"), tr("The text to render"),
                    TT_PLAIN_TEXT,
                    text_);

        pSize_ = params()->createFloatParameter(
                    "size", tr("size"),
                    tr("A size parameter in relation to texture resolution"),
                    1., 0.01, true, false);

        pAlignH_ = params()->createSelectParameter("alignment_h", tr("alignment horiz."),
                tr("Selects the alignment for the images"),
                { "l", "r", "c" },
                { tr("left"), tr("right"), tr("center") },
                { tr("left"), tr("right"), tr("center") },
                { Qt::AlignLeft, Qt::AlignRight, Qt::AlignHCenter },
                Qt::AlignHCenter, true, false);

        pAlignV_ = params()->createSelectParameter("alignment_v", tr("alignment vert."),
                tr("Selects the alignment for the images"),
                { "t", "b", "c" },
                { tr("top"), tr("bottom"), tr("center") },
                { tr("top"), tr("bottom"), tr("center") },
                { Qt::AlignTop, Qt::AlignBottom, Qt::AlignVCenter },
                Qt::AlignVCenter, true, false);

        pMipmaps_ = params()->createIntParameter(
                    "mipmaps", tr("mip-map levels"),
                    tr("The number of mip-map levels to create, "
                       "where each level is half the size of the previous level, "
                       "0 means no mip-maps"),
                    0, true, false);
        pMipmaps_->setMinValue(0);

    params()->endParameterGroup();


    params()->beginParameterGroup("text", tr("text"));

        pR_ = params()->createFloatParameter(
                    "red", tr("red"), tr("Red amount of text color"),
                    1., 0.0, 1.0, 0.1, true, false);
        pG_ = params()->createFloatParameter(
                    "green", tr("green"), tr("Green amount of text color"),
                    1., 0.0, 1.0, 0.1, true, false);
        pB_ = params()->createFloatParameter(
                    "blue", tr("blue"), tr("Blue amount of text color"),
                    1., 0.0, 1.0, 0.1, true, false);
        pA_ = params()->createFloatParameter(
                    "alpha", tr("alpha"), tr("Alpha amount of text color"),
                    1., 0.0, 1.0, 0.1, true, false);

        pbR_ = params()->createFloatParameter(
                    "back_red", tr("backg. red"), tr("Red amount of background color"),
                    0., 0.0, 1.0, 0.1, true, false);
        pbG_ = params()->createFloatParameter(
                    "back_green", tr("backg. green"), tr("Green amount of background color"),
                    0., 0.0, 1.0, 0.1, true, false);
        pbB_ = params()->createFloatParameter(
                    "back_blue", tr("backg. blue"), tr("Blue amount of background color"),
                    0., 0.0, 1.0, 0.1, true, false);
        pbA_ = params()->createFloatParameter(
                    "back_alpha", tr("backg. alpha"), tr("Alpha amount of background color"),
                    1., 0.0, 1.0, 0.1, true, false);

    params()->endParameterGroup();
}

void TextTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (p == pText_
            || p == pMipmaps_
            || p == pAlignH_
            || p == pAlignV_
            || p == pSize_)
        doRenderText_ = true;
}

void TextTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void TextTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();
}

void TextTO::setText(const QString &text)
{
    if (pText_)
        pText_->setValue(text);
    doRenderText_ = true;
}

void TextTO::getNeededFiles(IO::FileList & files)
{
    TextureObjectBase::getNeededFiles(files);
}

void TextTO::initGl(uint thread)
{
    TextureObjectBase::initGl(thread);
}

void TextTO::releaseGl(uint thread)
{
    if (tex_ && tex_->isAllocated())
        tex_->release();
    tex_ = 0;

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * TextTO::valueTexture(uint chan, Double , uint ) const
{
    if (chan != 0)
        return 0;

    if (doRenderText_)
    {
        if (tex_)
        {
            if (tex_->isHandle())
                tex_->release();
            delete tex_;
        }

        // create user-defined format
        //tex_ = createTexture();

        QImage img(getDesiredResolution(), QImage::Format_ARGB32_Premultiplied);
        img.fill(QColor(std::max(0, std::min(255, int(255 * pbR_->baseValue()))),
                          std::max(0, std::min(255, int(255 * pbG_->baseValue()))),
                          std::max(0, std::min(255, int(255 * pbB_->baseValue()))),
                          std::max(0, std::min(255, int(255 * pbA_->baseValue())))
                          ));

        QPainter p(&img);

        QFont font = p.font();
        font.setPixelSize(std::max(1, int(img.height() * pSize_->baseValue())));
        p.setFont(font);

        p.setPen(QColor(std::max(0, std::min(255, int(255 * pR_->baseValue()))),
                        std::max(0, std::min(255, int(255 * pG_->baseValue()))),
                        std::max(0, std::min(255, int(255 * pB_->baseValue()))),
                        std::max(0, std::min(255, int(255 * pA_->baseValue())))
                        ));

        p.drawText(QRectF(img.rect()),
                   pAlignH_->baseValue() | pAlignV_->baseValue(),
                   pText_->baseValue());

        p.end();

        try
        {
            tex_ = GL::Texture::createFromImage(img,
                                                getDesiredTextureFormat(),
                                                pMipmaps_->baseValue());
        }
        catch (Exception& e)
        {
            setErrorMessage(e.what());
            return 0;
        }

    }

    return tex_->isAllocated() ? tex_ : 0;
}



} // namespace MO
