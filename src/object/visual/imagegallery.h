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

    MO_OBJECT_CONSTRUCTOR(ImageGallery);
    ~ImageGallery();

    Vec4 imageColor(Double time, uint thread) const;
    Vec4 frameColor(Double time, uint thread) const;

    /** Returns a copy of the shader code after replacements, includes, etc.. */
    GL::ShaderSource shaderSource() const;

protected:

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, uint, Double time) Q_DECL_OVERRIDE;
    virtual void numberLightSourcesChanged(uint thread) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

signals:

private slots:

    void setupShader_();
    void setupVaos_();
    void releaseVaos_();
    void releaseAll_();

private:

    struct VAO_;

    GL::Shader * shader_;
    QList<VAO_*> vaos_;

    ParameterFloat
        *mr_, *mg_, *mb_, *ma_, *mbright_,
        *fr_, *fg_, *fb_, *fa_, *fbright_,
        *diffAmt_, *diffExp_, *specAmt_, *specExp_;
    ParameterSelect *arrangement_, *lightMode_,
        * keepImageAspect_;
    ParameterImageList *imageList_;
    ParameterGeometry *paramImageGeom_;

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
         doCreateVaos_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_IMAGEGALLERY_H
