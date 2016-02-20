/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/21/2015</p>
*/

#include <QImage>
#include <QPainter>
#include <QFontMetricsF>

#include "textto.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterfont.h"
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
    , text_             ("coffee\n&\ncigarettes")
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

        pFont_ = params()->createFontParameter(
                    "font", tr("font"), tr("The font to render"));

        pSize_ = params()->createFloatParameter(
                    "size", tr("size"),
                    tr("A size parameter in relation to texture resolution"),
                    1., 0.01, true, false);

        pFit_ = params()->createSelectParameter(
                    "fit", tr("fitting"),
                    tr("Selects the way the size is interpreted"),
        { "none", "w", "h", "best" },
        { tr("none"), tr("width"), tr("height"), tr("best") },
        { tr("The font size is related to the texture height"),
          tr("The font size is made to fit the text horizontally"),
          tr("The font size is made to fit the text vertically"),
          tr("The font size is made to fit the text vertically and horizontally") },
        { 0, 1, 2, 3 },
          3, true, false);

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

        pPadX_ = params()->createFloatParameter(
                    "pad_x", tr("padding x"),
                    tr("Distance to left or right border"),
                    0.05,  -1., 1.,
                    0.01, true, false);
        pPadY_ = params()->createFloatParameter(
                    "pad_y", tr("padding y"),
                    tr("Distance to top or bottom border"),
                    0.05,  -1., 1.,
                    0.01, true, false);
        pMipmaps_ = params()->createIntParameter(
                    "mipmaps", tr("mip-map levels"),
                    tr("The maximum number of mip-map levels to create, "
                       "where each level is half the size of the previous level, "
                       "0 means no mip-maps"),
                    0, true, false);
        pMipmaps_->setMinValue(0);
        pMipmaps_->setDefaultEvolvable(false);

    params()->endParameterGroup();


    params()->beginParameterGroup("color", tr("color"));

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
        pA_->setDefaultEvolvable(false);

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


    params()->beginParameterGroup("border", tr("border"));

        pBorderSize_ = params()->createFloatParameter(
                    "border_size", tr("border width"),
                    tr("Width of the border in relation to resolution"),
                    0., 0.0, 1.0, 0.01, true, false);

        pJoinStyle_ = params()->createSelectParameter("border_join", tr("corner style"),
                tr("The way the corners of the border are drawn"),
                { "bevel", "miter", "round" },
                { tr("flat"), tr("sharp"), tr("round") },
                { tr("Flat corners"), tr("Sharp corners"), tr("Round corners") },
                { Qt::BevelJoin, Qt::MiterJoin, Qt::RoundJoin },
                Qt::MiterJoin, true, false);

        pCornerRad_ = params()->createFloatParameter(
                    "corner_radius", tr("corner radius"),
                    tr("Radius of the rounded corner in relation to resolution"),
                    0.1, 0.0, 1.0, 0.01, true, false);

        pBackAlpha_ = params()->createBooleanParameter(
                    "back_trans", tr("clip border"),
                    tr("Makes the corners outside the frame transparent"),
                    tr("Corners are drawn with the background color"),
                    tr("Corners are transparent"),
                    true,
                    true, false);

    params()->endParameterGroup();

    // XXX There should be an initResolutionMode() for constructor
    setResolutionMode(RM_CUSTOM);
}

void TextTO::onParameterChanged(Parameter * p)
{
    TextureObjectBase::onParameterChanged(p);

    if (       p == pText_
            || p == pR_ || p == pG_ || p == pB_ || p == pA_
            || p == pbR_ || p == pbG_ || p == pbB_ || p == pbA_
            || p == pMipmaps_
            || p == pAlignH_
            || p == pAlignV_
            || p == pPadX_
            || p == pPadY_
            || p == pSize_
            || p == pFont_
            || p == pFit_
            || p == pBorderSize_
            || p == pJoinStyle_
            || p == pCornerRad_
            || p == pBackAlpha_
            )
        doRenderText_ = true;
}

void TextTO::onParametersLoaded()
{
    TextureObjectBase::onParametersLoaded();

}

void TextTO::updateParameterVisibility()
{
    TextureObjectBase::updateParameterVisibility();

    pCornerRad_->setVisible( pJoinStyle_->baseValue() == Qt::RoundJoin );
    pBackAlpha_->setVisible( pJoinStyle_->baseValue() != Qt::MiterJoin );

    pPadX_->setVisible( pAlignH_->baseValue() != Qt::AlignHCenter);
    pPadY_->setVisible( pAlignV_->baseValue() != Qt::AlignVCenter);
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

    doRenderText_ = true;
}

void TextTO::releaseGl(uint thread)
{
    if (tex_ && tex_->isAllocated())
        tex_->release();
    tex_ = 0;

    TextureObjectBase::releaseGl(thread);
}

const GL::Texture * TextTO::valueTexture(uint chan, const RenderTime& ) const
{
    if (chan != 0)
        return 0;

    if (doRenderText_)
    {
        doRenderText_ = false;

        if (tex_)
        {
            if (tex_->isHandle())
                tex_->release();
            delete tex_;
        }

        // create image
        QImage img(getDesiredResolution(), QImage::Format_ARGB32_Premultiplied);

        const int flags = pAlignH_->baseValue() | pAlignV_->baseValue();
        const int msize = std::min(img.width(), img.height());
        const qreal border = msize * .5 * pBorderSize_->baseValue(),
                    hborder = .5 * border,
                    padX = pPadX_->baseValue() * (img.width() - border),
                    padY = pPadY_->baseValue() * (img.height() - border);
        const QRectF irect = QRectF(img.rect().adjusted(border, border, -border, -border));
        QRectF frect = irect;
        if (flags & Qt::AlignLeft)
            frect.moveLeft(frect.left() + padX);
        if (flags & Qt::AlignRight)
            frect.moveRight(frect.right() - padX);
        if (flags & Qt::AlignTop)
            frect.moveTop(frect.top() + padY);
        if (flags & Qt::AlignBottom)
            frect.moveBottom(frect.bottom() - padY);

        // init background
        const QColor fillColor = QColor(std::max(0, std::min(255, int(255 * pbR_->baseValue()))),
                                        std::max(0, std::min(255, int(255 * pbG_->baseValue()))),
                                        std::max(0, std::min(255, int(255 * pbB_->baseValue()))),
                                        std::max(0, std::min(255, int(255 * pbA_->baseValue()))));
        const bool backAlpha = pBackAlpha_->baseValue()
                && pJoinStyle_->baseValue() != Qt::MiterJoin
                && border > 0.;
        if (backAlpha)
            img.fill(QColor(255,255,255,0));
        else
            img.fill(fillColor);

        // get font (to change size)
        QFont font = pFont_->baseValue();

        // -- choose size --

        Double si = pSize_->baseValue();
        if (pFit_->baseValue() == 0)
            si *= irect.height();
        else
        {
            font.setPixelSize(1);
            QRectF br = QFontMetricsF(font).boundingRect(irect, flags, pText_->baseValue());

            if (br.width() < .1)
                br.setWidth(.1);
            if (br.height() < .1)
                br.setHeight(.1);
            switch (pFit_->baseValue())
            {
                case 1: si *= irect.width() / br.width(); break;
                case 2: si *= irect.height() / br.height(); break;
                case 3: si *= std::min(irect.width() / br.width(),
                                       irect.height() / br.height()); break;
            }
        }

        font.setPixelSize(std::max(1, int(si)));

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setFont(font);

        const QPen txtpen = QPen(
                        QColor(std::max(0, std::min(255, int(255 * pR_->baseValue()))),
                               std::max(0, std::min(255, int(255 * pG_->baseValue()))),
                               std::max(0, std::min(255, int(255 * pB_->baseValue()))),
                               std::max(0, std::min(255, int(255 * pA_->baseValue())))
                        ));

        // border
        if (border > 0.)
        {
            QPen pen(txtpen);
            pen.setWidthF(border);
            pen.setJoinStyle(Qt::PenJoinStyle(pJoinStyle_->baseValue()));
            p.setPen(pen);
            if (backAlpha)
                p.setBrush(QBrush(fillColor));
            else
                p.setBrush(Qt::NoBrush);
            if (pJoinStyle_->baseValue() == Qt::RoundJoin)
            {
                const qreal rr = msize * pCornerRad_->baseValue();
                p.drawRoundedRect(irect.adjusted(-hborder, -hborder, hborder, hborder),
                                  rr, rr);
            }
            else
                p.drawRect(irect.adjusted(-hborder, -hborder, hborder, hborder));
        }

        // text
        p.setPen(txtpen);
        p.drawText(frect,
                   flags,
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

    return tex_ && tex_->isAllocated() ? tex_ : 0;
}



} // namespace MO
