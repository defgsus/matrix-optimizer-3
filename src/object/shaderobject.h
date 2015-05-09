/** @file shaderobject.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.03.2015</p>
*/

#ifndef MOSRC_OBJECT_SHADEROBJECT_H
#define MOSRC_OBJECT_SHADEROBJECT_H

#include "objectgl.h"
#include "object/interface/valuetextureinterface.h"

namespace MO {

class UserUniformSetting;

class ShaderObject : public ObjectGl, public ValueTextureInterface
{
    Q_OBJECT
public:


    MO_OBJECT_CONSTRUCTOR(ShaderObject);
    ~ShaderObject();

    virtual Type type() const Q_DECL_OVERRIDE { return T_SHADER; }
    virtual bool isShader() const Q_DECL_OVERRIDE { return true; }

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE;

    /** texture output interface */
    virtual const GL::Texture * valueTexture(Double time, uint thread) const Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

    /** Draws the contents of the framebuffer on a [-1,1] quad.
        @p width and @p height are the size of the viewport of the current context. */
    void drawFramebuffer(uint thread, Double time, int width, int height);

    /** Returns the internal framebuffer on which the shader renders, or NULL */
    GL::FrameBufferObject * fbo() const { return fbo_; }

signals:

public slots:

private:

    GL::FrameBufferObject *fbo_;
    GL::ScreenQuad *shaderQuad_, *screenQuad_;
    GL::Texture * swapTex_;

    ParameterFloat * p_out_r_, * p_out_g_, * p_out_b_, * p_out_a_;
    ParameterSelect * p_magInterpol_, * p_enableOut_,
                * p_texType_, * p_texFormat_;
    ParameterInt * p_width_, * p_height_, * p_aa_, * p_split_;
    ParameterText * p_fragment_;

    GL::Uniform
        * u_resolution_,
        * u_time_,
        * u_transformation_,
        * u_fb_tex_,
        * u_out_color_, * u_out_resolution_;

    Float aspectRatio_;
    AlphaBlendSetting alphaBlend_;
    UserUniformSetting * userUniforms_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SHADEROBJECT_H
