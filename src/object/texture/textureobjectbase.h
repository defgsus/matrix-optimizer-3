/** @file textureobjectbase.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H
#define MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H

#include <QStringList>

#include "object/visual/objectgl.h"
#include "object/interface/valuetextureinterface.h"
#include "object/interface/valueshadersourceinterface.h"
#include "object/interface/masteroutinterface.h"
#include "gl/shader.h"

namespace MO {

/** Base class for texture processors */
class TextureObjectBase
        : public ObjectGl
        , public ValueTextureInterface
        , public ValueShaderSourceInterface
        , public MasterOutInterface
{
public:

    enum ResolutionMode
    {
        RM_CUSTOM,
        RM_INPUT,
        RM_INPUT_SCALED,
        RM_INPUT_FIX_WIDTH,
        RM_INPUT_FIX_HEIGHT
    };

    MO_ABSTRACT_OBJECT_CONSTRUCTOR(TextureObjectBase);

    virtual Type type() const Q_DECL_OVERRIDE { return T_TEXTURE; }
    virtual bool isTexture() const Q_DECL_OVERRIDE { return true; }

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void getNeededFiles(IO::FileList& l) Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    //virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE;


    // -------- TextureObject ------------
    // --- getter ----

    /** Returns the resolution of the internal Framebuffer,
        or, if not present, the getDesiredResolution() */
    QSize resolution() const;

    /** Returns aspect ratio of the internal Framebuffer */
    Float aspectRatio() const;

    /** Number of maximum inputs for this object */
    uint numberTextureInputs() const;
    /** Returns true, when the @p index'th parameter is attached to a texture output */
    bool hasTextureInput(uint index) const;

    /** Returns the list of last compile messages, e.g. from createShaderQuad() */
    const QList<GL::Shader::CompileMessage>& compileMessages() const;

    /** Returns the currently set resolution mode */
    ResolutionMode getResolutionMode() const;

    /** Returns the currently set resolution (from parameters) */
    QSize getDesiredResolution() const;
    /** Returns the currently set texture format (from parameters) */
    gl::GLenum getDesiredTextureFormat() const;

    /** For a given input resolution, returns the resolution
        according to current ResolutionMode setting.
        For RM_CUSTOM the returned resolution is unchanged.
        For all other modes, the resolution is scaled accordingly. */
    QSize adjustResolution(const QSize& s) const;

    // ---- MasterOutInterface ----

    bool isMasterOutputEnabled() const Q_DECL_OVERRIDE;
    void setMasterOutputEnabled(bool enable, bool sendGui = false) Q_DECL_OVERRIDE;

    // ---- setter -----

    void setResolutionMode(ResolutionMode mode, bool sendGui = false);

    // ------ texture connections --------

protected:

    /** Call this in constructor to provide a hint for the maximum number
        of texture inputs. Default is 0 */
    void initMaximumTextureInputs(uint num);
    void initMaximumTextureInputs(const QStringList& names);

    /** Call in constructor to disable internal fbo creation and respective parameters.
        Used with renderShaderQuad(GL::FrameBufferObject*, ...).
        Default is true. */
    void initInternalFbo(bool);

    /** Call this in constructor to get the color range parameters.
        Default is false. */
    void initEnableColorRange(bool);

    /** Call this in constructor to set the framebuffer color-texture to
        a 3d texture. Default is 1 (2d texture). */
    void init3dFramebuffer(uint depth);

    /** Call in constructor to allow number of passes
        to be adjusted by user, default is false. */
    void initAllowMultiPass(bool = true);

    /** Call in constructor to disallow user-change of resolution */
    void initEnableResolutionChange(bool enable);

    /** Returns the texture parameters (e.g. to change names and visibility) */
    const QList<ParameterTexture*>& textureParams() const;

    /** Returns the index'th texture from the texture inputs, or NULL */
    const GL::Texture* inputTexture(uint index, const RenderTime& rt) const;

    /** Returns true if any of the input textures has changed since their
        respective last calls to ParameterTexture::value() */
    bool hasInputTextureChanged(const RenderTime& time) const;

    /** Returns a string for defining the uniforms (sampler2D or samplerCube),
        for connected inputs.
        @note sampleCube can only be determined after the first renderpass
        because inputTexture() or ParameterTexture::value() are the first
        to see the actual texture. */
    QString getInputTextureDeclarations(const QStringList& names) const;

    // ---------- opengl stuff -----------
public:

    /** texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

    /** Draws the contents of the framebuffer on a [-1,1] quad.
        @p width and @p height are the size of the viewport of the current context.
        Used for master output by scene renderer.
        Override to draw your own output.
        By default the first valueTexture() will be rendered with settings defined
        in the 'master output' section of the parameters. */
    virtual void drawFramebuffer(const RenderTime & time, int width, int height);

    /** Returns the internal framebuffer on which the shader renders, or NULL */
    GL::FrameBufferObject * fbo() const;

    /** Returns a copy of the source for the @p index'th shaderquad, or
        empty source if not defined/compiled.
        Override to return source of your internal shaders not compiled with
        createShaderQuad(). */
    virtual GL::ShaderSource valueShaderSource(uint index) const override;

protected:

    /** Creates a texture with common-parameter specified settings.
        Ownership is on caller. The texture is only created, not opengl initialized.
        Use a variant of GL::Texture::create() afterwards.
        @note Returns NULL when initInternalFbo(false) */
    GL::Texture * createTexture() const;

    /** Creates a new quad with attached shader.
        Can be called multiple times. Instances can later also be retrieved with shaderQuad().
        Source includes will be resolved.
        Supply a list of texture uniform names to order the slots.
        @note to be called from initGl() only.
        @throws GlException */
    GL::ScreenQuad * createShaderQuad(const GL::ShaderSource & src,
                                      const QList<QString>& texUniformNames = QList<QString>());

    /** Returns the previously created quad, or NULL */
    GL::ScreenQuad * shaderQuad(uint index) const;

    /** Draws the index'th quad.
        Input textures are automatically bound.
        @p texSlot determines the first texture slot to bind the input textures to,
        and returns the next free slot.
        @note does nothing if initInternalFbo(false) */
    void renderShaderQuad(uint index, const RenderTime & time, uint* texSlot);

    /** Renders the first quad, starting at texture slot 0.
        @note does nothing if initInternalFbo(false) */
    void renderShaderQuad(const RenderTime& time)
        { uint texSlot = 0; renderShaderQuad(0, time, &texSlot); }

    /** Render the index'th quad onto the given framebuffer.
        u_time, u_resolution and other uniforms are updated.
        Otherwise OpenGL state is not really touched
        @note Fbo and input textures must be bound! */
    void renderShaderQuad(GL::FrameBufferObject* fbo,
                          uint index, const RenderTime& time,
                          bool doClear = true) const;
private:

    struct PrivateTO;
    PrivateTO * p_to_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H
