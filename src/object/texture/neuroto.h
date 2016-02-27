/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/5/2016</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_NEUROTO_H
#define MOSRC_OBJECT_TEXTURE_NEUROTO_H

#include "textureobjectbase.h"

namespace MO {

/** An neuronal network texture processor */
class NeuroTO: public TextureObjectBase
{
public:

    enum Mode
    {
        M_BYPASS,
        M_ERROR,
        M_FPROP,
        M_BPROP,
        /** Full input->output processor with back-prop */
        M_FULL_BP,
        M_WEIGHT_INIT
    };

    MO_OBJECT_CONSTRUCTOR(NeuroTO);
    ~NeuroTO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual QString getOutputName(SignalType, uint channel) const Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime&) Q_DECL_OVERRIDE;

    virtual GL::ShaderSource valueShaderSource(uint index) const Q_DECL_OVERRIDE;

    /* texture output interface */
    virtual const GL::Texture * valueTexture(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

    /** Returns current component mode, or default if parameter is not created */
    Mode getNeuroMode() const;

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_NEUROTO_H
