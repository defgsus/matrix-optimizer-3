/** @file objectgl.cpp

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

//#include <QDebug>

#include <QFile>

#include "objectgl.h"
#include "io/error.h"
#include "io/log.h"
#include "gl/context.h"
#include "io/datastream.h"
#include "scene.h"
#include "textobject.h"
#include "param/parameters.h"
#include "param/parameterselect.h"

namespace MO {

const QStringList ObjectGl::depthTestModeNames =
{ tr("parent"), tr("on"), tr("off") };

const QStringList ObjectGl::depthWriteModeNames =
{ tr("parent"), tr("on"), tr("off") };

//const QStringList ObjectGl::alphaBlendModeNames =
//{ tr("parent"), tr("off"), tr("cross-fade"), tr("add") };

ObjectGl::ObjectGl(QObject *parent)
    : Object                    (parent),
      p_alphaBlend_             (this),
      p_numberLightSources_     (0),
      p_defaultDepthTestMode_   (DTM_PARENT),
      p_defaultDepthWriteMode_  (DWM_PARENT),
      p_defaultAlphaBlendMode_  (AlphaBlendSetting::M_PARENT),
      p_enableCreateRenderSettings_(true),
      p_updateModeVisible_      (true),
      p_updateRequest_          (true),
      p_paramDepthTest_         (0),
      p_paramDepthWrite_        (0),
      p_paramUpdateMode_        (0)
{
}

void ObjectGl::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("ogl", 1);
}

void ObjectGl::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("ogl", 1);
}

ObjectGl::UpdateMode ObjectGl::updateMode() const
{
     return p_paramUpdateMode_
             ? (UpdateMode)p_paramUpdateMode_->baseValue()
             : p_defaultUpdateMode_;
}

void ObjectGl::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("active", tr("activity"));

        p_paramUpdateMode_ = params()->createSelectParameter("rendset_update", tr("render"),
            tr("Selects when and how often the object is rendered"),
            { "always", "change" },
            { tr("every frame"), tr("on change") },
            { tr("The object is rendered every frame"),
              tr("The object is only rendered when it's parameters have been changed") },
            { UM_ALWAYS, UM_ON_CHANGE },
            p_defaultUpdateMode_, true, false);
        p_paramUpdateMode_->setVisible(p_updateModeVisible_);

    params()->endParameterGroup();


    if (p_enableCreateRenderSettings_)
    {
        params()->beginParameterGroup("renderset", tr("render settings"));

            p_paramDepthTest_ = params()->createSelectParameter("rendset_dtm", tr("depth testing"),
                tr("Selects whether a test against the depth-map should be performed"),
                { "p", "on", "off" },
                depthTestModeNames,
                { tr("Uses the same setting as the parent object"),
                  tr("Depth-testing is on - the object will be hidden by closer objects."),
                  tr("Depth-testing is off - "
                     "the object will paint itself regardless of other objects.") },
                { DTM_PARENT, DTM_ON, DTM_OFF },
                p_defaultDepthTestMode_, true, false);

            p_paramDepthWrite_ = params()->createSelectParameter("rendset_dwm", tr("depth writing"),
                tr("Selects whether the depth information of the object should be written"),
                { "p", "on", "off" },
                depthWriteModeNames,
                { tr("Uses the same setting as the parent object"),
                  tr("Depth-writing is on - the depth information can be used by other objects."),
                  tr("Depth-writing is off - the depth information will simply be discarded.") },
                { DWM_PARENT, DWM_ON, DWM_OFF },
                p_defaultDepthWriteMode_, true, false);

            p_alphaBlend_.createParameters(AlphaBlendSetting::M_PARENT, true, "_", "_OGl_");

        params()->endParameterGroup();
    }
}

void ObjectGl::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    p_updateRequest_ = true;

    if (   p == p_paramDepthTest_
        || p == p_paramDepthWrite_
        || p_alphaBlend_.hasParameter(p))
    {
        rootObject()->propagateRenderMode(0);
    }
}

/*void ObjectGl::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}*/

void ObjectGl::onDependency(Object * o)
{
    Object::onDependency(o);
    requestReinitGl();
}


QString ObjectGl::getGlslInclude(const QString &url, bool do_search)
{
    // --- built-in ---

    if (do_search)
    {
        QFile f_(":/shader/inc/" + url);
        if (f_.open(QFile::Text | QFile::ReadOnly))
            return QString::fromUtf8(f_.readAll());
    }


    // in objects
    Object * o = findObjectByNamePath(url);
    if (!o && sceneObject())
        o = sceneObject()->findObjectByNamePath(url);
    if (!o)
        return QString();

    if (o->isText())
    {
        auto to = static_cast<TextObject*>(o);
        auto tex = to->valueText(0, 0);
        if (tex.second == TT_GLSL)
        {
            if (sceneObject())
                sceneObject()->installDependency(this, o);
            return tex.first;
        }
    }

    return QString();
}




void ObjectGl::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("ObjectGl::setNumberThreads(" << num << ")");

    Object::setNumberThreads(num);

    uint oldnum = p_glContext_.size();
    p_glContext_.resize(num);
    p_needsInitGl_.resize(num);
    p_isGlInitialized_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        p_glContext_[i] = 0;
        p_needsInitGl_[i] = true;
        p_isGlInitialized_[i] = false;
    }
}

void ObjectGl::p_setGlContext_(uint thread, GL::Context * c)
{
    MO_ASSERT(thread < p_glContext_.size(),
              "setGlContext_(" << thread << ", " << c << ") but "
              "glContext_.size() == " << p_glContext_.size());

    if (c != p_glContext_[thread])
    {
        p_needsInitGl_[thread] = true;
    }

    p_glContext_[thread] = c;
}

void ObjectGl::p_initGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::initGl_(" << thread << ") "
                "p_glContext_.size() == " << p_glContext_.size());

    if (!p_glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!p_glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    // clear text dependency before shader compilation
    if (sceneObject())
        sceneObject()->removeDependencies(this);

    MO_EXTEND_EXCEPTION(
        initGl(thread),
                "in ObjectGl '" << idName() << "', thread=" << thread
                );

    p_needsInitGl_[thread] = false;
    p_isGlInitialized_[thread] = true;
}


void ObjectGl::p_releaseGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::releaseGl_(" << thread << ")");

    if (!p_glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");

    MO_EXTEND_EXCEPTION(
        releaseGl(thread),
                "in ObjectGl '" << idName() << "', thread=" << thread
                );

    p_isGlInitialized_[thread] = false;
}

void ObjectGl::p_renderGl_(const GL::RenderSettings &rs, uint thread, Double time)
{
    using namespace gl;

    if (!p_glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!p_glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    // ---- set render modes -----

    if (depthTestMode() == DTM_OFF)
        MO_CHECK_GL( glDisable(GL_DEPTH_TEST) )
    else
        MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

    if (depthWriteMode() == DWM_OFF)
        MO_CHECK_GL( glDepthMask(GL_FALSE) )
    else
        MO_CHECK_GL( glDepthMask(GL_TRUE) );

    p_alphaBlend_.apply(alphaBlendMode());

    MO_EXTEND_EXCEPTION(

        renderGl(rs, thread, time);

        , "in ObjectGl '" << idName() << "', thread=" << thread
    );

}

void ObjectGl::requestRender()
{
    p_updateRequest_ = true;

    Scene * scene = sceneObject();
    if (!scene)
        return;

    scene->render();
}

void ObjectGl::requestReinitGl()
{
    for (uint i=0; i<numberThreads(); ++i)
    {
        p_needsInitGl_[i] = true;
    }

    requestRender();
}

/*
ObjectGl::DepthTestMode ObjectGl::depthTestMode() const
{
    return paramDepthTest_? (DepthTestMode)paramDepthTest_->baseValue() : DTM_ON;
}

ObjectGl::DepthWriteMode ObjectGl::depthWriteMode() const
{
    return paramDepthWrite_? (DepthWriteMode)paramDepthWrite_->baseValue() : DWM_ON;
}

ObjectGl::AlphaBlendMode ObjectGl::alphaBlendMode() const
{
    return paramAlphaBlend_? (AlphaBlendMode)paramAlphaBlend_->baseValue() : ABM_MIX;
}
*/

/*
void ObjectGl::updateRenderMode_() const
{
    // find parent gl object
    ObjectGl * p = 0;
    Object * prnt = parentObject();
    while (!p && prnt)
    {
        if (ObjectGl * pgl = qobject_cast<ObjectGl*>(prnt))
            p = pgl;
        else
            prnt = prnt->parentObject();
    }

    DepthTestMode dtm = depthTestMode();
}
*/

void ObjectGl::propagateRenderMode(ObjectGl *parent)
{
    // -- determine settings from parameters and/or from parent

    if (!p_paramDepthTest_)
        p_curDepthTestMode_ = DTM_ON;
    else
    {
        if (p_paramDepthTest_->baseValue() == DTM_PARENT)
            p_curDepthTestMode_ = parent ? parent->depthTestMode() : DTM_ON;
        else
            p_curDepthTestMode_ = (DepthTestMode)p_paramDepthTest_->baseValue();
    }

    if (!p_paramDepthWrite_)
        p_curDepthWriteMode_ = DWM_ON;
    else
    {
        if (p_paramDepthWrite_->baseValue() == DWM_PARENT)
            p_curDepthWriteMode_ = parent ? parent->depthWriteMode() : DWM_ON;
        else
            p_curDepthWriteMode_ = (DepthWriteMode)p_paramDepthWrite_->baseValue();
    }

    if (!p_alphaBlend_.parametersCreated())
        p_curAlphaBlendMode_ = AlphaBlendSetting::M_MIX;
    else
    {
        if (p_alphaBlend_.mode() == AlphaBlendSetting::M_PARENT)
            // get parent's value
            p_curAlphaBlendMode_ = parent ? parent->alphaBlendMode()
                                            // mix is default when no parent
                                          : AlphaBlendSetting::M_MIX;
        else
            p_curAlphaBlendMode_ =
                    (AlphaBlendSetting::Mode)p_alphaBlend_.mode();

        p_alphaBlend_.setParentMode(p_curAlphaBlendMode_);
    }


    // pass to children

    for (auto c : childObjects())
        c->propagateRenderMode(this);
}

} // namespace MO
