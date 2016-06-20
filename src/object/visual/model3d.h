/** @file model3d.h

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_MODEL3D_H
#define MOSRC_OBJECT_VISUAL_MODEL3D_H


#include "objectgl.h"
#include "object/interface/valuegeometryinterface.h"
#include "object/interface/valueshadersourceinterface.h"
#include "object/interface/geometryeditinterface.h"
#include "gl/shadersource.h"

namespace MO {

class TextureMorphSetting;
class UserUniformSetting;

class Model3d
        : public ObjectGl
        , public ValueGeometryInterface
        , public ValueShaderSourceInterface
        , public GeometryEditInterface
{

public:
    enum LightMode
    {
        LM_NONE,
        LM_PER_VERTEX,
        LM_PER_FRAGMENT
    };

    MO_OBJECT_CONSTRUCTOR(Model3d);

    /** Returns the current geometry settings. */
    const GEOM::GeometryFactorySettings& getGeometrySettings() const override;

    /** Sets new geometry settings and creates the geometry on next render */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&) override;

    /** Overwrite current geometry */
    void setGeometry(const GEOM::Geometry& );

    const GEOM::Geometry * geometry() const;
    Vec4 modelColor(const RenderTime & time) const;

    /** Shadersource interface */
    GL::ShaderSource valueShaderSource(uint channel) const override;

    /** Geometry interface */
    const GEOM::Geometry * valueGeometry(
            uint channel, const RenderTime& time) const Q_DECL_OVERRIDE
    { Q_UNUSED(time); return channel == 0 ? geometry() : 0; }

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

    void geometryCreated_();
    void geometryFailed_(const QString & e);

    void setupDrawable_();
    /** Discards the current thread, if any, and sets creator_=0. */
    void resetCreator_();

    void updateCodeVersion_();

    GL::Drawable * draw_;
    GEOM::GeometryCreator * creator_;
    GEOM::GeometryFactorySettings * geomSettings_;
    GEOM::Geometry * nextGeometry_;

    TextureSetting * texture_, *textureBump_,
                    * textureEnv_;
    ColorPostProcessingSetting * texturePostProc_;
    TextureMorphSetting
        *textureMorph_,
        *textureBumpMorph_;
    UserUniformSetting * uniformSetting_;

    ParameterFloat * cr_, *cg_, *cb_, *ca_, *cbright_,
        *diffAmt_, *diffExp_, *specAmt_, *specExp_,
        *bumpScale_,
        *vertexExtrude_, *paramLineWidth_,
        *paramPointSize_, *paramPointSizeMax_, *paramPointSizeDistFac_,
        *envMapAmt_, *envMapAmt2_, *envMapAmt3_;
    ParameterSelect * fixPosition_, * lightMode_, *vertexFx_,
        *glslDoOverride_, *glslDoGeometry_, *paramLineSmooth_,
                    * usePointCoord_, *pointSizeAuto_, *paramPolySmooth_;
    ParameterText * glslVertex_, *glslTransform_, *glslVertexOut_, *glslFragmentOut_,
                *glslNormal_, *glslLight_, *glslGeometry_;
    ParameterInt * paramNumInstance_;

    GL::Uniform * u_cam_pos_, * u_light_amt_, * u_bump_scale_,
                * u_vertex_extrude_, * u_pointsize_, * u_instance_count_,
                * u_tex_0_, *u_texn_0_, *u_tex_env_0_,
                * u_env_map_amt_;

    bool doRecompile_;
    int loadedVersion_;

    GL::Texture * xxx_2d, * xxx_cube;
    GL::Uniform * xxx_u_2d, * xxx_u_cube;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_MODEL3D_H
