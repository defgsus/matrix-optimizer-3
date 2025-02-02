/** @file shaderobject.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.03.2015</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_SHADEROBJECT_H
#define MOSRC_OBJECT_VISUAL_SHADEROBJECT_H

#include "ObjectGl.h"
#include "object/interface/ValueTextureInterface.h"
#include "object/interface/MasterOutInterface.h"

namespace MO {

class UserUniformSetting;

/** A free editable GLSL shader object with position in space. */
class ShaderObject
        : public ObjectGl
        , public ValueTextureInterface
        , public MasterOutInterface
{
public:

    MO_OBJECT_CONSTRUCTOR(ShaderObject);

    virtual Type type() const Q_DECL_OVERRIDE { return T_SHADER; }
    virtual bool isShader() const Q_DECL_OVERRIDE { return true; }

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& time) Q_DECL_OVERRIDE;

    // ------- ValueTextureInterface --------

    /** texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

    // ------- MasterOutInterfacce ----------

    virtual bool isMasterOutputEnabled() const Q_DECL_OVERRIDE;
    virtual void setMasterOutputEnabled(bool enable, bool sendGui = false) Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

    /** Draws the contents of the framebuffer on a [-1,1] quad.
        @p width and @p height are the size of the viewport of the current context. */
    void drawFramebuffer(const RenderTime& time, int width, int height);

    /** Returns the internal framebuffer on which the shader renders, or NULL */
    GL::FrameBufferObject * fbo() const { return fbo_; }

    /** Returns the resolution of the internal framebuffer */
    QSize resolution() const;

private:

    GL::FrameBufferObject *fbo_;
    GL::ScreenQuad *shaderQuad_, *screenQuad_;
    GL::Texture * swapTex_;

    ParameterFloat * p_out_r_, * p_out_g_, * p_out_b_, * p_out_a_;
    ParameterSelect * p_magInterpol_, * p_enableOut_,
                * p_texType_, * p_texFormat_;
    ParameterInt * p_width_, * p_height_, * p_aa_, * p_split_, * p_passes_;
    ParameterText * p_fragment_;

    GL::Uniform
        * u_resolution_,
        * u_time_,
        * u_transformation_,
        * u_fb_tex_,
        * u_out_color_, * u_out_resolution_,
        * u_pass_;

    Float aspectRatio_;
    AlphaBlendSetting alphaBlend_;
    UserUniformSetting * userUniforms_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_SHADEROBJECT_H
