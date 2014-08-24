/** @file colorpostprocessingsetting.cpp

    @brief General post-processing parameters and uniform handling

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include "colorpostprocessingsetting.h"

#include "object/object.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "gl/shader.h"

namespace MO {

ColorPostProcessingSetting::ColorPostProcessingSetting(Object * parent) :
    QObject (parent),
    object_ (parent)
{
}


void ColorPostProcessingSetting::createParameters(const QString &id_suffix)
{
    paramPost_ = object_->createSelectParameter("postenable" + id_suffix, tr("post-processing"),
                                       tr("Enables or disables color post-processing"),
                                        { "off", "on" }, { tr("off"), tr("on") },
                                        { tr("Color-post-processing is enabled"),
                                          tr("Color-post-processing is disabled") },
                                        { 0, 1 }, 0, true, false);

    postAlpha_ = object_->createFloatParameter("postalpha" + id_suffix, "color to alpha",
                                    tr("Makes the choosen color transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaR_ = object_->createFloatParameter("postalphar" + id_suffix, "red",
                                    tr("Red component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaG_ = object_->createFloatParameter("postalphag" + id_suffix, "green",
                                    tr("Green component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaB_ = object_->createFloatParameter("postalphab" + id_suffix, "blue",
                                    tr("Blue component of the color to make transparent"),
                                    0.0, 0.0, 1.0, 0.02);
    postAlphaEdge_ = object_->createFloatParameter("postalphae" + id_suffix, "edge",
                                    tr("Edge of the alpha transformation - the lower the smoother"),
                                    0.0, 0.0, 1.0, 0.02);

    postBright_ = object_->createFloatParameter("postbright" + id_suffix, "brightness",
                                    tr("Brightness (simply offset to all colors)"),
                                    0.0, -1.0, 1.0, 0.02);

    postContrast_ = object_->createFloatParameter("postcontr" + id_suffix, "contrast",
                                    tr("Contrast (a multiplier around threshold)"),
                                    1.0, 0.0, 100000.0, 0.02);

    postContrastThresh_ = object_->createFloatParameter("postcontrt" + id_suffix, "threshold",
                                    tr("The threshold or edge between dark and "
                                       "bright for contrast setting"),
                                    0.5, 0.0, 1.0, 0.02);

    postShift_ = object_->createFloatParameter("postshift" + id_suffix, "rgb shift",
                                    tr("Shifts the rgb colors right (postive) or left (negative)"),
                                    0.0, -1.0, 1.0, 0.05);

    postInv_ = object_->createFloatParameter("postinv" + id_suffix, "negative",
                                    tr("Mix between normal and negative colors [0,1]"),
                                    0.0, 0.0, 1.0, 0.05);

    postGray_ = object_->createFloatParameter("postgray" + id_suffix, "grayscale",
                                    tr("Removes the saturation [0,1]"),
                                    0.0, 0.0, 1.0, 0.05);
}

bool ColorPostProcessingSetting::needsRecompile(Parameter *p) const
{
    return (p == paramPost_);
}

void ColorPostProcessingSetting::updateParameterVisibility()
{
    // XXX
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

void ColorPostProcessingSetting::updateUniforms(Double time, uint thread)
{
    u_post_trans_->floats[0] = postGray_->value(time, thread);
    u_post_trans_->floats[1] = postInv_->value(time, thread);
    u_post_trans_->floats[2] = postShift_->value(time, thread);
    u_post_bright_->floats[0] = postBright_->value(time, thread);
    u_post_bright_->floats[1] = postContrast_->value(time, thread);
    u_post_bright_->floats[2] = postContrastThresh_->value(time, thread);
    u_post_alpha_->setFloats(
                postAlphaR_->value(time, thread),
                postAlphaG_->value(time, thread),
                postAlphaB_->value(time, thread),
                postAlpha_->value(time, thread));
    u_post_alpha_edge_->floats[0] = postAlphaEdge_->value(time, thread);
}


} // namespace MO
