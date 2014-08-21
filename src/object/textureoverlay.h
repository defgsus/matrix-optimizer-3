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

    enum ProjectionType
    {
        PT_FLAT,
        PT_FISHEYE,
        PT_EQUIRECT
    };

    MO_OBJECT_CONSTRUCTOR(TextureOverlay);

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings &rs, uint, Double time) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

signals:

private slots:

private:

    ProjectionType ptype_, actualPtype_;

    TextureSetting * texture_;

    //ParameterFilename * paramFilename_;
    ParameterSelect * paramPType_, *paramPost_;
    ParameterFloat * cr_, *cg_, *cb_, *ca_, *pos_influence_,
            * postInv_, *postBright_, *postContrast_, *postContrastThresh_,
            *postGray_, *postShift_, *postAlpha_, *postAlphaR_, *postAlphaG_, *postAlphaB_,
            *postAlphaEdge_;
    //GL::Texture * tex_;
    GL::ScreenQuad * quad_;
    GL::Uniform * u_color_,
        *u_dir_matrix_, *u_cam_angle_, *u_sphere_offset_,
        *u_local_transform_, *u_cube_hack_,
        *u_post_trans_, *u_post_bright_, *u_post_alpha_, *u_post_alpha_edge_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTUREOVERLAY_H
