/** @file textureobjectbase.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H
#define MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H

#include "object/objectgl.h"
#include "object/interface/valuetextureinterface.h"

namespace MO {

/** Base class for texture processors */
class TextureObjectBase : public ObjectGl, public ValueTextureInterface
{
    Q_OBJECT
public:

    enum ResolutionMode
    {
        RM_CUSTOM,
        RM_INPUT
    };

    MO_ABSTRACT_OBJECT_CONSTRUCTOR(TextureObjectBase);
    ~TextureObjectBase();

    virtual Type type() const Q_DECL_OVERRIDE { return T_TEXTURE; }
    virtual bool isTexture() const Q_DECL_OVERRIDE { return true; }

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    //virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE;

    // -------- TextureObject ------------

    QSize resolution() const;
    Float aspectRatio() const;
    /** Number of maximum inputs for this object */
    uint numberTextureInputs() const;
    /** Returns true, when the @p index'th parameter is attached to a texture output */
    bool hasTextureInput(uint index) const;

    /** Sets the master-out parameter.
        if @p sendGui, this will be done via ObjectEditor */
    void setEnableMasterOut(bool enable, bool sendGui = false);
    void setResolutionMode(ResolutionMode mode, bool sendGui = false);

    // ------ texture connections --------

protected:

    /** Calls this in constructor to provide a hint for the maximum number
        of texture inputs. Default is 0 */
    void initMaximumTextureInputs(uint num);
    /** Calls this in constructor to get the color range parameters.
        Default is false. */
    void initEnableColorRange(bool);

    /** Returns the texture parameters (e.g. to change names and visibility) */
    const QList<ParameterTexture*>& textureParams();

    // ---------- opengl stuff -----------
public:

    /** texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, Double time, uint thread) const Q_DECL_OVERRIDE;

    /** Draws the contents of the framebuffer on a [-1,1] quad.
        @p width and @p height are the size of the viewport of the current context. */
    void drawFramebuffer(uint thread, Double time, int width, int height);

    /** Returns the internal framebuffer on which the shader renders, or NULL */
    GL::FrameBufferObject * fbo() const;

protected:

    /** Creates a new quad with attached shader.
        Can be called multiple times. Instances can later also be retrieved with shaderQuad().
        Source includes will be resolved.
        Supply a list of texture uniform names to order the slots.
        @note to be called from initGl() only.
        @throws GlException */
    GL::ScreenQuad * createShaderQuad(const GL::ShaderSource & src,
                                      const QList<QString>& texUniformNames = QList<QString>());

    /** Returns the previously created quad, or NULL */
    GL::ScreenQuad * shaderQuad(uint index);

    /** Draws the index'th quad.
        @p texSlot determines the first texture slot to bind the input textures to,
        and returns the next free slot. */
    void renderShaderQuad(uint index, Double time, uint thread, uint& texSlot);

    /** Renders the first quad, starting at texture slot 0 */
    void renderShaderQuad(Double time, uint thread)
        { uint texSlot = 0; renderShaderQuad(0, time, thread, texSlot); }

private:

    struct PrivateTO;
    PrivateTO * p_to_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_TEXTUREOBJECTBASE_H
