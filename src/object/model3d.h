/** @file model3d.h

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_MODEL3D_H
#define MOSRC_OBJECT_MODEL3D_H


#include "objectgl.h"

namespace MO {

class Model3d : public ObjectGl
{
    Q_OBJECT
public:
    enum LightMode
    {
        LM_PER_VERTEX,
        LM_PER_FRAGMENT
    };


    MO_OBJECT_CONSTRUCTOR(Model3d);

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, uint, Double time) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;

    /** Returns the current geometry settings. */
    const GEOM::GeometryFactorySettings& geometrySettings() const { return *geomSettings_; }

    /** Sets new geometry settings and creates the geometry on next render */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&);

signals:

private slots:

    void geometryCreated_();
    void geometryFailed_();

    void setupDrawable_();

private:
    GL::Drawable * draw_;
    GEOM::GeometryCreator * creator_;
    GEOM::GeometryFactorySettings * geomSettings_;
    GEOM::Geometry * nextGeometry_;

    ParameterFloat * cr_, *cg_, *cb_, *ca_,
        *diffExp_;
    ParameterSelect * lightMode_;

    GL::Uniform * u_diff_exp_;

    bool doRecompile_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MODEL3D_H
