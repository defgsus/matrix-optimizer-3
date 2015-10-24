/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_IMAGEGALLERY_H
#define MOSRC_OBJECT_VISUAL_IMAGEGALLERY_H

#include "objectgl.h"
#include "gl/shadersource.h"

namespace MO {

class ImageGallery : public ObjectGl
{
    Q_OBJECT
public:
    enum LightMode
    {
        LM_NONE,
        LM_PER_VERTEX,
        LM_PER_FRAGMENT
    };

    enum Arrangement
    {
        A_ROW,
        A_COLUMN,
        A_CIRCLE,
        A_CLOCK,
        A_CYLINDER_H,
        A_CYLINDER_V,
        A_RANDOM_PLANE,
        A_RANDOM_3D
    };

    MO_OBJECT_CONSTRUCTOR(ImageGallery);
    ~ImageGallery();

    Vec4 imageColor(const RenderTime & time) const;
    Vec4 frameColor(const RenderTime & time) const;

    /** Returns a copy of the shader code after replacements, includes, etc.. */
    GL::ShaderSource shaderSource() const;

protected:

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, const RenderTime& time) Q_DECL_OVERRIDE;
    virtual void numberLightSourcesChanged(uint thread) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

private:

    void setupShader_();
    void setupVaos_();
    void releaseVaos_();
    void releaseAll_();

    void calcEntityBaseTransform_();
    void calcEntityTransform_(const RenderTime& time);

    struct Entity_;

    GL::Shader * shader_;
    QList<Entity_*> entities_;
    GL::VertexArrayObject * vaoImage_, * vaoFrame_;
    Vec3 extentMin_, extentMax_, extent_;

    ParameterFloat
        *mr_, *mg_, *mb_, *ma_, *mbright_,
        *fr_, *fg_, *fb_, *fa_, *fbright_,
        *diffAmt_, *diffExp_, *specAmt_, *specExp_,
        *diffAmtF_, *diffExpF_, *specAmtF_, *specExpF_,
        *spacing_, *radius_, *scale_,
        *radiusX_, *radiusY_, *radiusZ_,
        *rotation_, *rotX_, *rotY_, *rotZ_,
        *pickIndex_, *pickMix_,
        *pickPosX_, *pickPosY_, *pickPosZ_,
        *pickScale_,
        *pickRot_, *pickRotX_, *pickRotY_, *pickRotZ_;
    ParameterInt
        *randomSeed_, * mipmaps_;
    ParameterSelect *arrangement_, *lightMode_,
        * keepImageAspect_, * keepFrameAspect_,
        * alignH_, * alignV_,
        * doPickScale_, * doPickPos_, * doPickRot_,
        * paramMin_, * paramMag_;
    ParameterImageList *imageList_;
    ParameterGeometry *paramImageGeom_,
                        *paramFrameGeom_;
    TextureSetting * frameTexSet_;

    GL::Uniform * u_cam_pos_, * u_light_amt_, * u_tex_,
            * uniformProj_,
            * uniformCVT_,
            * uniformVT_,
            * uniformT_,
            * uniformLightPos_,
            * uniformLightColor_,
            * uniformLightDirection_,
            * uniformLightDirectionParam_,
            * uniformLightDiffuseExp_,
            * uniformSceneTime_,
            * uniformColor_;

    bool doRecompile_,
         doCalcBaseTransform_,
         doCreateVaos_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_IMAGEGALLERY_H
