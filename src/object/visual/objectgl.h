/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_OBJECTGL_H
#define MOSRC_OBJECT_VISUAL_OBJECTGL_H

#include "object/object.h"
#include "gl/opengl.h"
#include "object/util/alphablendsetting.h"

#undef CM_NONE // windows..

namespace MO {
namespace GL { class Context; class Drawable; }

/** Base of all OpenGL objects. */
class ObjectGl : public Object
{
    Q_OBJECT

    // for gl context handling
    friend class Scene;

public:

    enum DepthTestMode
    {
        DTM_PARENT,
        DTM_ON,
        DTM_OFF
    };

    static const QStringList depthTestModeNames;

    enum DepthWriteMode
    {
        DWM_PARENT,
        DWM_ON,
        DWM_OFF
    };

    static const QStringList depthWriteModeNames;

    enum UpdateMode
    {
        UM_ALWAYS,
        UM_ON_CHANGE
    };

    enum CullingMode
    {
        CM_PARENT,
        CM_NONE,
        CM_FRONT,
        CM_BACK
    };


    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ObjectGl)

    virtual Type type() const Q_DECL_OVERRIDE { return T_OBJECT; }
    bool isGl() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    //virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onDependency(Object *) Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // ---------------- opengl --------------------

    /** Returns the current GL::Context */
    GL::Context * glContext(uint thread) { return p_glContext_[thread]; }
    /** Returns the current GL::Context */
    const GL::Context * glContext(uint thread) const { return p_glContext_[thread]; }

    /** Used by scene */
    bool needsInitGl(uint thread) const { return p_needsInitGl_[thread]; }

    /** Used by scene */
    bool isGlInitialized(uint thread) const
        { return thread < p_isGlInitialized_.size() && p_isGlInitialized_[thread]; }

    /** Tell scene that this object wants to be painted */
    void requestRender();

    /** Requests a releaseGl() - initGl() sequence */
    void requestReinitGl();

    /** Number of lights, the shader should support */
    uint numberLightSources() const { return p_numberLightSources_; }

    /** The currently active update mode */
    UpdateMode updateMode() const;

    /** This will return true when the object wants to render */
    bool isUpdateRequest() const { return p_updateRequest_; }
    /** Clears the update request flag.
        @note This is not done automatically in render function,
        because e.g. models may need to be rendered per cubeface. */
    void clearUpdateRequest() { p_updateRequest_ = false; }

    /** Number of times p_renderGl_() was called */
    unsigned long renderCount() const { return p_renderCount_; }

    /** Returns the source for the include url, or an empty string */
    QString getGlslInclude(const QString& url, bool include_system_defaults);

    // ------------- opengl virtual interface -----------

    /** Override to initialize opengl resources.
        Currently, there's only one opengl thread, so in most cases
        the thread parameter can be ignored. */
    virtual void initGl(uint thread) = 0;

    /** Override to release opengl resources */
    virtual void releaseGl(uint thread) = 0;

    /** Override to render your stuff */
    virtual void renderGl(const GL::RenderSettings& rs, uint thread, Double time) = 0;

    /** Called by Scene when the number of lights have changed.
        numberLightSources() will contain the new value.
        If you need to recompile the shader, do it in your renderGl() function
        or call requestReinitGl() */
    virtual void numberLightSourcesChanged(uint thread) { Q_UNUSED(thread); }

    // ----------------- render state -------------------

    /* these must all be called before createParameters, e.g. in object constructor */
    void initCreateRenderSettings(bool enable) { p_enableCreateRenderSettings_ = enable; }
    void initDefaultDepthTestMode(DepthTestMode m) { p_defaultDepthTestMode_ = m; }
    void initDefaultDepthWriteMode(DepthWriteMode m) { p_defaultDepthWriteMode_ = m; }
    void initDefaultAlphaBlendMode(AlphaBlendSetting::Mode m) { p_defaultAlphaBlendMode_ = m; }
    void initDefaultUpdateMode(UpdateMode m, bool visible = true)
        { p_defaultUpdateMode_ = m; p_updateModeVisible_ = visible; }

    bool isRenderSettingsEnabled() const { return p_enableCreateRenderSettings_; }

    DepthTestMode defaultDepthTestMode() const { return p_defaultDepthTestMode_; }
    DepthWriteMode defaultDepthWriteMode() const { return p_defaultDepthWriteMode_; }
    AlphaBlendSetting::Mode defaultAlphaBlendMode() const { return p_defaultAlphaBlendMode_; }

    DepthTestMode depthTestMode() const { return p_curDepthTestMode_; }
    DepthWriteMode depthWriteMode() const { return p_curDepthWriteMode_; }
    AlphaBlendSetting::Mode alphaBlendMode() const { return p_curAlphaBlendMode_; }
    CullingMode cullingMode() const { return p_curCullingMode_; }

    virtual void propagateRenderMode(ObjectGl * parent) Q_DECL_OVERRIDE;

private:

    /** Sets the OpenGL Context.
     *  Only Scene needs access here.
     *  XXX please refacture this and all those friend dependencies */
    void p_setGlContext_(uint thread, GL::Context *);

    void p_initGl_(uint thread);
    void p_releaseGl_(uint thread);
    void p_renderGl_(const GL::RenderSettings& rs, uint thread, Double time);

    std::vector<GL::Context*> p_glContext_;
    std::vector<int> p_needsInitGl_, p_isGlInitialized_;

    AlphaBlendSetting p_alphaBlend_;

    /** Number of lights that this object's shader is currently compiled for */
    uint p_numberLightSources_;

    unsigned long p_renderCount_;

    // --- render state ---

    DepthTestMode p_defaultDepthTestMode_, p_curDepthTestMode_;
    DepthWriteMode p_defaultDepthWriteMode_, p_curDepthWriteMode_;
    AlphaBlendSetting::Mode p_defaultAlphaBlendMode_, p_curAlphaBlendMode_;
    UpdateMode p_defaultUpdateMode_;
    CullingMode p_curCullingMode_;

    bool p_enableCreateRenderSettings_,
         p_updateModeVisible_,
         p_updateRequest_;
    ParameterSelect
        *p_paramDepthTest_,
        *p_paramDepthWrite_,
        *p_paramUpdateMode_,
        *p_paramCullMode_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_OBJECTGL_H
