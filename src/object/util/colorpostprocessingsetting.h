/** @file colorpostprocessingsetting.h

    @brief General post-processing parameters and uniform handling

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_COLORPOSTPROCESSINGSETTING_H
#define MOSRC_OBJECT_UTIL_COLORPOSTPROCESSINGSETTING_H

#include <QCoreApplication>

#include "object/object_fwd.h"
#include "gl/opengl_fwd.h"
#include "types/time.h"

namespace MO {

class ColorPostProcessingSetting
{
    Q_DECLARE_TR_FUNCTIONS(ColorPostProcessingSetting)
public:
    /** Creates the settings for the given object */
    explicit ColorPostProcessingSetting(Object *parent = 0);

    // ---------- parameters -----------

    /** Creates the post-processing-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one option set for an Object. */
    void createParameters(const QString& id_suffix);

    /** Returns true when a change to parameter @p p requires
        a recompilation of the shader.
        Call in Object::onParameterChanged() and e.g. call
        requestReinitGl() when this returns true. */
    bool needsRecompile(Parameter * p) const;

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    // ---------- opengl ---------------

    /** Returns true if post-processing is enabled by the Parameter setting */
    bool isEnabled() const;

    /** Querries all the uniforms from the shader.
        @note The uniforms are expected to exist! */
    void getUniforms(GL::Shader *, const QString & uniform_name_suffix = "");

    /** Sets the uniform values (cpu).
        The next GL::Shader::sendUniforms() will update the value on gpu. */
    void updateUniforms(const RenderTime& time);

private:

    Object * object_;

    ParameterSelect *paramPost_;

    ParameterFloat *postInv_, *postBright_, *postContrast_, *postContrastThresh_,
    *postGray_, *postShift_, *postAlpha_, *postAlphaR_, *postAlphaG_, *postAlphaB_,
    *postAlphaEdge_;

    GL::Uniform
        *u_post_trans_, *u_post_bright_, *u_post_alpha_, *u_post_alpha_edge_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_COLORPOSTPROCESSINGSETTING_H
