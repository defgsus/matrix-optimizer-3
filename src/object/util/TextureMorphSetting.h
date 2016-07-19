/** @file texturemorphsetting.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_TEXTUREMORPHSETTING_H
#define MOSRC_OBJECT_UTIL_TEXTUREMORPHSETTING_H


#include <QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS()

#include "object/Object_fwd.h"
#include "gl/opengl_fwd.h"
#include "types/time.h"

namespace MO {

class TextureMorphSetting : public QObject
{
    Q_DECLARE_TR_FUNCTIONS(TextureMorphSetting)
public:
    /** Creates the settings for the given object */
    explicit TextureMorphSetting(Object *parent = 0);

    // ---------- parameters -----------

    /** Creates the related parameters in parent Object.
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

    void setVisible(bool);

    // ---------- opengl ---------------

    /** Returns true if processing is enabled by the Parameter setting */
    bool isTransformEnabled() const;
    bool isSineMorphEnabled() const;

    /** Querries all the uniforms from the shader.
        @note The uniforms are expected to exist! */
    void getUniforms(GL::Shader *, const QString & uniform_name_suffix = "");

    /** Sets the uniform values (cpu).
        The next GL::Shader::sendUniforms() will update the value on gpu. */
    void updateUniforms(const RenderTime& time);

private:

    Object * object_;

    ParameterSelect *pTrans_, *pSineMorph_;

    ParameterFloat *pTransX_, *pTransY_, *pTransSX_, *pTransSY_,
            *pSineMorphAmpX_, *pSineMorphFreqX_, *pSineMorphPhaseX_,
            *pSineMorphAmpY_, *pSineMorphFreqY_, *pSineMorphPhaseY_;

    GL::Uniform
        *u_tex_transform_, *u_tex_morphx_, *u_tex_morphy_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_TEXTUREMORPHSETTING_H
