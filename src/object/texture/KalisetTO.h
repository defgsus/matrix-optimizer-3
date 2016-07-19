/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/9/2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTURE_KALISETTO_H
#define MOSRC_OBJECT_TEXTURE_KALISETTO_H

#include "TextureObjectBase.h"
#include "object/interface/EvolutionEditInterface.h"

namespace MO {

/** The infamous Kali-Set 2d texture */
class KaliSetTO
        : public TextureObjectBase
{
public:

    MO_OBJECT_CONSTRUCTOR(KaliSetTO);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, const RenderTime& time) Q_DECL_OVERRIDE;

    // ---- ValueShaderSourceInterface -----

    virtual GL::ShaderSource valueShaderSource(uint channel) const override;

    // ---- EvolutionEditInterface ---------

    virtual const EvolutionBase* getEvolution(const QString& key) const override;
    virtual void setEvolution(const QString& key, const EvolutionBase*) override;

    // ---------- specific stuff -----------

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_KALISETTO_H
