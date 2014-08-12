/** @file textureoverlay.h

    @brief Texture overlay object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#ifndef MOSRC_OBJECT_TEXTUREOVERLAY_H
#define MOSRC_OBJECT_TEXTUREOVERLAY_H
#include "objectgl.h"

namespace MO {

class TextureOverlay : public ObjectGl
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(TextureOverlay);

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings &rs, uint, Double time) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;

signals:

private slots:

private:

    ParameterFloat * cr_, *cg_, *cb_, *ca_;
    GL::Texture * tex_;
    GL::ScreenQuad * quad_;
    GL::Uniform * u_color_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTUREOVERLAY_H
