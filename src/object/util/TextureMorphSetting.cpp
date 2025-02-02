/** @file texturemorphsetting.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#include "TextureMorphSetting.h"
#include "object/Object.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "gl/Shader.h"
#include "math/constants.h"

namespace MO {

TextureMorphSetting::TextureMorphSetting(Object * parent) :
    object_ (parent),
    pTrans_ (0),
    pSineMorph_ (0)
{
}


void TextureMorphSetting::createParameters(const QString &id_suffix)
{
    auto params = object_->params();
    pTrans_ = params->createBooleanParameter("_texmph_trans" + id_suffix,
                                    tr("transformation"),
                                    tr("Enables moving and scaling the texture"),
                                    tr("Disabled"),
                                    tr("Enabled"),
                                    false,
                                    true, false);

    pTransX_ = params->createFloatParameter("_texmph_transx" + id_suffix,
                                    tr("x"),
                                    tr("The x translation"),
                                    0.0, 0.02);
    pTransY_ = params->createFloatParameter("_texmph_transy" + id_suffix,
                                    tr("y"),
                                    tr("The y translation"),
                                    0.0, 0.02);
    pTransSX_ = params->createFloatParameter("_texmph_transsx" + id_suffix,
                                    tr("scale x"),
                                    tr("The scale on x axis"),
                                    1.0, 0.02);
    pTransSY_ = params->createFloatParameter("_texmph_transsy" + id_suffix,
                                    tr("scale y"),
                                    tr("The scale on y axis"),
                                    1.0, 0.02);

    pSineMorph_ = params->createBooleanParameter("_texmph_sinm" + id_suffix,
                                    tr("sine morph"),
                                    tr("Enables morphing the texture coordinates by waves"),
                                    tr("Disabled"),
                                    tr("Enabled"),
                                    false,
                                    true, false);

    pSineMorphAmpX_ = params->createFloatParameter("_texmph_sinm_ax" + id_suffix,
                                    tr("ampltiude"),
                                    tr("Amplitude of sine modulation on x axis"),
                                    0.25, 0.025);
    pSineMorphFreqX_ = params->createFloatParameter("_texmph_sinm_fx" + id_suffix,
                                    tr("frequency"),
                                    tr("Frequency of sine modulation on x axis"),
                                    1.0, 0.125);
    pSineMorphPhaseX_ = params->createFloatParameter("_texmph_sinm_px" + id_suffix,
                                    tr("phase"),
                                    tr("Phase/time of sine modulation on x axis"),
                                    0.0, 0.025);

    pSineMorphAmpY_ = params->createFloatParameter("_texmph_sinm_ay" + id_suffix,
                                    tr("ampltiude"),
                                    tr("Amplitude of sine modulation on y axis"),
                                    0.25, 0.025);
    pSineMorphFreqY_ = params->createFloatParameter("_texmph_sinm_fy" + id_suffix,
                                    tr("frequency"),
                                    tr("Frequency of sine modulation on y axis"),
                                    1.0, 0.125);
    pSineMorphPhaseY_ = params->createFloatParameter("_texmph_sinm_py" + id_suffix,
                                    tr("phase"),
                                    tr("Phase/time of sine modulation on y axis"),
                                    0.0, 0.025);
}

bool TextureMorphSetting::needsRecompile(Parameter *p) const
{
    return (p == pTrans_ || p == pSineMorph_);
}

void TextureMorphSetting::setVisible(bool v)
{
    pTrans_->setVisible(v);
    pSineMorph_->setVisible(v);
    updateParameterVisibility();
}

void TextureMorphSetting::updateParameterVisibility()
{
    const bool
            trans = pTrans_->isVisible() && pTrans_->baseValue(),
            morph = pSineMorph_->isVisible() && pSineMorph_->baseValue();

    pTransX_->setVisible( trans );
    pTransY_->setVisible( trans );
    pTransSX_->setVisible( trans );
    pTransSY_->setVisible( trans );
    pSineMorphAmpX_->setVisible( morph );
    pSineMorphFreqX_->setVisible( morph );
    pSineMorphPhaseX_->setVisible( morph );
    pSineMorphAmpY_->setVisible( morph );
    pSineMorphFreqY_->setVisible( morph );
    pSineMorphPhaseY_->setVisible( morph );
}

bool TextureMorphSetting::isTransformEnabled() const
{
    return pTrans_ && pTrans_->baseValue();
}

bool TextureMorphSetting::isSineMorphEnabled() const
{
    return pSineMorph_ && pSineMorph_->baseValue();
}

void TextureMorphSetting::getUniforms(GL::Shader * shader, const QString& id_suffix)
{
    // Info: DO NOT EXPECT the uniforms to be present because
    // they might not if there is no light source in the scene
    if (isTransformEnabled())
        u_tex_transform_ = shader->getUniform("u_tex_transform" + id_suffix, false);

    if (isSineMorphEnabled())
    {
        u_tex_morphx_ = shader->getUniform("u_tex_morphx" + id_suffix, false);
        u_tex_morphy_ = shader->getUniform("u_tex_morphy" + id_suffix, false);
    }
}

void TextureMorphSetting::updateUniforms(const RenderTime& time)
{
    if (isTransformEnabled() && u_tex_transform_)
    {
        u_tex_transform_->setFloats(
                    pTransX_->value(time),
                    pTransY_->value(time),
                    pTransSX_->value(time),
                    pTransSY_->value(time));
    }

    if (isSineMorphEnabled())
    {
        if (u_tex_morphx_)
            u_tex_morphx_->setFloats(
                pSineMorphAmpX_->value(time),
                pSineMorphFreqX_->value(time) * TWO_PI,
                pSineMorphPhaseX_->value(time) * TWO_PI, 0);
        if (u_tex_morphy_)
            u_tex_morphy_->setFloats(
                pSineMorphAmpY_->value(time),
                pSineMorphFreqY_->value(time) * TWO_PI,
                pSineMorphPhaseY_->value(time) * TWO_PI, 0);
    }
}


} // namespace MO
