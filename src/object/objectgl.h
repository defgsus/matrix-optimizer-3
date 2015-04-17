/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object.h"
#include "gl/opengl.h"
#include "util/alphablendsetting.h"

namespace MO {
namespace GL { class Context; class Drawable; }

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


    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ObjectGl)

    virtual Type type() const Q_DECL_OVERRIDE { return T_OBJECT; }
    bool isGl() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;

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

    /** Returns the source for the include url, or an empty string */
    QString getGlslInclude(const QString& url, bool do_search) const;

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
    void setCreateRenderSettings(bool enable) { p_enableCreateRenderSettings_ = enable; }
    void setDefaultDepthTestMode(DepthTestMode m) { p_defaultDepthTestMode_ = m; }
    void setDefaultDepthWriteMode(DepthWriteMode m) { p_defaultDepthWriteMode_ = m; }
    void setDefaultAlphaBlendMode(AlphaBlendSetting::Mode m) { p_defaultAlphaBlendMode_ = m; }

    DepthTestMode defaultDepthTestMode() const { return p_defaultDepthTestMode_; }
    DepthWriteMode defaultDepthWriteMode() const { return p_defaultDepthWriteMode_; }
    AlphaBlendSetting::Mode defaultAlphaBlendMode() const { return p_defaultAlphaBlendMode_; }

    DepthTestMode depthTestMode() const { return p_curDepthTestMode_; }
    DepthWriteMode depthWriteMode() const { return p_curDepthWriteMode_; }
    AlphaBlendSetting::Mode alphaBlendMode() const { return p_curAlphaBlendMode_; }

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

    // --- render state ---

    DepthTestMode p_defaultDepthTestMode_, p_curDepthTestMode_;
    DepthWriteMode p_defaultDepthWriteMode_, p_curDepthWriteMode_;
    AlphaBlendSetting::Mode p_defaultAlphaBlendMode_, p_curAlphaBlendMode_;

    bool p_enableCreateRenderSettings_;
    ParameterSelect *p_paramDepthTest_, *p_paramDepthWrite_;//, *p_paramAlphaBlend_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
