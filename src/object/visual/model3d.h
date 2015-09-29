/** @file model3d.h

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_MODEL3D_H
#define MOSRC_OBJECT_VISUAL_MODEL3D_H


#include "objectgl.h"
#include "gl/shadersource.h"

namespace MO {

class TextureMorphSetting;
class UserUniformSetting;

class Model3d : public ObjectGl
{
    Q_OBJECT
public:
    enum LightMode
    {
        LM_NONE,
        LM_PER_VERTEX,
        LM_PER_FRAGMENT
    };


    MO_OBJECT_CONSTRUCTOR(Model3d);
    ~Model3d();

    /** Returns the current geometry settings. */
    const GEOM::GeometryFactorySettings& geometrySettings() const;

    /** Sets new geometry settings and creates the geometry on next render */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&);

    /** Overwrite current geometry */
    void setGeometry(const GEOM::Geometry& );

    const GEOM::Geometry * geometry() const;
    Vec4 modelColor(Double time, uint thread) const;
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

    void geometryCreated_();
    void geometryFailed_(const QString & e);

    void setupDrawable_();
    /** Discards the current thread, if any, and sets creator_=0. */
    void resetCreator_();

private:

    void updateCodeVersion_();

    GL::Drawable * draw_;
    GEOM::GeometryCreator * creator_;
    GEOM::GeometryFactorySettings * geomSettings_;
    GEOM::Geometry * nextGeometry_;

    TextureSetting * texture_, *textureBump_;
    ColorPostProcessingSetting * texturePostProc_;
    TextureMorphSetting
        *textureMorph_,
        *textureBumpMorph_;
    UserUniformSetting * uniformSetting_;

    ParameterFloat * cr_, *cg_, *cb_, *ca_, *cbright_,
        *diffAmt_, *diffExp_, *specAmt_, *specExp_,
        *bumpScale_,
        *vertexExtrude_, *paramLineWidth_,
        *paramPointSize_, *paramPointSizeMax_, *paramPointSizeDistFac_;
    ParameterSelect * fixPosition_, * lightMode_, *vertexFx_, *glslDoOverride_, *paramLineSmooth_,
                    * usePointCoord_, *pointSizeAuto_, *paramPolySmooth_;
    ParameterText * glslVertex_, *glslTransform_, *glslVertexOut_, *glslFragmentOut_,
                *glslNormal_, *glslLight_;
    ParameterInt * paramNumInstance_;

    GL::Uniform * u_cam_pos_, * u_light_amt_, * u_bump_scale_,
                * u_vertex_extrude_, * u_pointsize_, * u_instance_count_,
                * u_tex_0_, *u_texn_0_;

    bool doRecompile_;
    int loadedVersion_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_MODEL3D_H
