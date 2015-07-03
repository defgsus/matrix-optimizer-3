/** @file Generate3dTO.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2015</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_TEXTURE_GENERATE3DTO_H
#define MOSRC_OBJECT_TEXTURE_GENERATE3DTO_H

#include "textureobjectbase.h"

namespace MO {

/** A 3D-Texture generator sampling a scalar-field function */
class Generate3dTO : public TextureObjectBase
{
    Q_OBJECT
public:

    MO_OBJECT_CONSTRUCTOR(Generate3dTO);
    ~Generate3dTO();

    //virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE;

    // ---------- specific stuff -----------

signals:

public slots:

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTURE_GENERATE3DTO_H

#endif // #ifndef MO_DISABLE_EXP
