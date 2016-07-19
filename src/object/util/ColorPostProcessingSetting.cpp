/** @file colorpostprocessingsetting.cpp

    @brief General post-processing parameters and uniform handling

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include "ColorPostProcessingSetting.h"

#include "object/Object.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "gl/Shader.h"

namespace MO {

ColorPostProcessingSetting::ColorPostProcessingSetting(Object * parent)
    : object_ (parent)
{
}


void ColorPostProcessingSetting::createParameters(const QString &id_suffix)
{
    auto params = object_->params();

    paramPost_ = params->createBooleanParameter("postenable" + id_suffix, tr("color effects"),
                                    tr("Enables or disables color post-processing"),
                                    tr("Color-post-processing is disabled"),
                                    tr("Color-post-processing is enabled"),
                                    false,
                                    true, false);

    postAlpha_ = params->createFloatParameter("postalpha" + id_suffix,
                                    tr("color to alpha"),
                                    tr("Makes the choosen color transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaR_ = params->createFloatParameter("postalphar" + id_suffix,
                                    tr("red"),
                                    tr("Red component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaG_ = params->createFloatParameter("postalphag" + id_suffix,
                                    tr("green"),
                                    tr("Green component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaB_ = params->createFloatParameter("postalphab" + id_suffix,
                                    tr("blue"),
                                    tr("Blue component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaEdge_ = params->createFloatParameter("postalphae" + id_suffix,
                                    tr("edge"),
                                    tr("Edge of the alpha transformation - the lower the smoother"),
                                    0.0, 0.0, 1.0, 0.02);

    postBright_ = params->createFloatParameter("postbright" + id_suffix,
                                    tr("brightness"),
                                    tr("Brightness (simply offset to all colors)"),
                                    0.0, -100000.0, 100000.0, 0.02);

    postContrast_ = params->createFloatParameter("postcontr" + id_suffix,
                                    tr("contrast"),
                                    tr("Contrast (a multiplier around threshold)"),
                                    1.0, 0.0, 100000.0, 0.02);

    postContrastThresh_ = params->createFloatParameter("postcontrt" + id_suffix,
                                    tr("threshold"),
                                    tr("The threshold or edge between dark and "
                                       "bright for contrast setting"),
                                    0.5, 0.0, 1.0, 0.02);

    postShift_ = params->createFloatParameter("postshift" + id_suffix,
                                    tr("rgb shift"),
                                    tr("Shifts the rgb colors right (postive) or left (negative)"),
                                    0.0, -1.0, 1.0, 0.05);

    postInv_ = params->createFloatParameter("postinv" + id_suffix,
                                    tr("negative"),
                                    tr("Mix between normal and negative colors [0,1]"),
                                    0.0, 0.0, 1.0, 0.05);

    postGray_ = params->createFloatParameter("postgray" + id_suffix,
                                    tr("grayscale"),
                                    tr("Removes the saturation [0,1]"),
                                    0.0, 0.0, 1.0, 0.05);
}

bool ColorPostProcessingSetting::needsRecompile(Parameter *p) const
{
    return (p == paramPost_);
}

void ColorPostProcessingSetting::setVisible(bool v)
{
    paramPost_->setVisible(v);
    updateParameterVisibility();
}

void ColorPostProcessingSetting::updateParameterVisibility()
{
    bool act = isEnabled() && paramPost_->isVisible();

    postAlpha_->setVisible( act );
    postAlphaR_->setVisible( act );
    postAlphaG_->setVisible( act );
    postAlphaB_->setVisible( act );
    postAlphaEdge_->setVisible( act );
    postBright_->setVisible( act );
    postContrast_->setVisible( act );
    postContrastThresh_->setVisible( act );
    postShift_->setVisible( act );
    postInv_->setVisible( act );
    postGray_->setVisible( act );
}

bool ColorPostProcessingSetting::isEnabled() const
{
    return paramPost_->baseValue() != 0;
}


void ColorPostProcessingSetting::getUniforms(GL::Shader * shader, const QString& id_suffix)
{
    u_post_trans_ = shader->getUniform("u_post_transform" + id_suffix, true);
    u_post_bright_ = shader->getUniform("u_post_bright" + id_suffix, true);
    u_post_alpha_ = shader->getUniform("u_post_alpha" + id_suffix, true);
    u_post_alpha_edge_ = shader->getUniform("u_post_alpha_edge" + id_suffix, true);
}

void ColorPostProcessingSetting::updateUniforms(const RenderTime & time)
{
    u_post_trans_->floats[0] = postGray_->value(time);
    u_post_trans_->floats[1] = postInv_->value(time);
    u_post_trans_->floats[2] = postShift_->value(time);
    u_post_bright_->floats[0] = postBright_->value(time);
    u_post_bright_->floats[1] = postContrast_->value(time);
    u_post_bright_->floats[2] = postContrastThresh_->value(time);
    u_post_alpha_->setFloats(
                postAlphaR_->value(time),
                postAlphaG_->value(time),
                postAlphaB_->value(time),
                postAlpha_->value(time));
    u_post_alpha_edge_->floats[0] = postAlphaEdge_->value(time);
}


} // namespace MO
