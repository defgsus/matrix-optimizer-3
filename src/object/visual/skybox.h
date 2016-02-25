/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/24/2016</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_SKYBOX_H
#define MOSRC_OBJECT_VISUAL_SKYBOX_H

#include "objectgl.h"

namespace MO {

class Skybox
        : public ObjectGl
{
    Q_OBJECT
public:

    enum ContentMode
    {
        CM_TEXTURE,
        CM_GLSL,
    };

    enum ShapeMode
    {
        SM_SPHERE,
        SM_PLANE,
        SM_CYLINDER
    };

    enum Axis
    {
        A_POS_X,
        A_NEG_X,
        A_POS_Y,
        A_NEG_Y,
        A_POS_Z,
        A_NEG_Z
    };

    MO_OBJECT_CONSTRUCTOR(Skybox);
    ~Skybox();

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

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_SKYBOX_H
