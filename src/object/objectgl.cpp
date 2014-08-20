/** @file objectgl.cpp

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

//#include <QDebug>

#include "objectgl.h"
#include "io/error.h"
#include "io/log.h"
#include "gl/context.h"
#include "io/datastream.h"
#include "scene.h"
#include "param/parameterselect.h"

namespace MO {

const QStringList ObjectGl::depthTestModeNames =
{ tr("parent"), tr("on"), tr("off") };

const QStringList ObjectGl::depthWriteModeNames =
{ tr("parent"), tr("on"), tr("off") };

const QStringList ObjectGl::alphaBlendModeNames =
{ tr("parent"), tr("off"), tr("cross-fade"), tr("add") };

ObjectGl::ObjectGl(QObject *parent)
    : Object                (parent),
      defaultDepthTestMode_ (DTM_PARENT),
      defaultDepthWriteMode_(DWM_PARENT),
      defaultAlphaBlendMode_(ABM_PARENT),
      enableCreateRenderSettings_(true),
      paramDepthTest_       (0),
      paramDepthWrite_      (0),
      paramAlphaBlend_      (0)
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

void ObjectGl::createParameters()
{
    Object::createParameters();

    if (enableCreateRenderSettings_)
    {
        beginParameterGroup("renderset", tr("render settings"));

        paramDepthTest_ = createSelectParameter("rendset_dtm", tr("depth testing"),
            tr("Selects whether a test against the depth-map should be performed"),
            { "p", "on", "off" },
            depthTestModeNames,
            { tr("Uses the same setting as the parent object"),
              tr("Depth-testing is on - the object will be hidden by closer objects."),
              tr("Depth-testing is off - "
                 "the object will paint itself regardless of other objects.") },
            { DTM_PARENT, DTM_ON, DTM_OFF },
            defaultDepthTestMode_, true, false);

        paramDepthWrite_ = createSelectParameter("rendset_dwm", tr("depth writing"),
            tr("Selects whether the depth information of the object should be written"),
            { "p", "on", "off" },
            depthWriteModeNames,
            { tr("Uses the same setting as the parent object"),
              tr("Depth-writing is on - the depth information can be used by other objects."),
              tr("Depth-writing is off - the depth information will simply be discarded.") },
            { DWM_PARENT, DWM_ON, DWM_OFF },
            defaultDepthWriteMode_, true, false);

        paramAlphaBlend_ = createSelectParameter("rendset_abm", tr("alpha blending"),
            tr("Selects how semi-transparent objects are composed on screen"),
            { "p", "off", "mix", "add" },
            alphaBlendModeNames,
            { tr("Uses the same setting as the parent object"),
              tr("No alpha blending occures - the alpha value is ignored"),
              tr("The colors are cross-faded depending on the alpha value"),
              tr("The colors are simply added on top") },
            { ABM_PARENT, ABM_OFF, ABM_MIX, ABM_ADD },
            defaultAlphaBlendMode_, true, false);

        endParameterGroup();
    }
}

void ObjectGl::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == paramDepthTest_ || p == paramDepthWrite_ || p == paramAlphaBlend_)
    {
        rootObject()->propagateRenderMode(0);
    }
}


void ObjectGl::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("ObjectGl::setNumberThreads(" << num << ")");

    Object::setNumberThreads(num);

    uint oldnum = glContext_.size();
    glContext_.resize(num);
    needsInitGl_.resize(num);
    isGlInitialized_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        glContext_[i] = 0;
        needsInitGl_[i] = true;
        isGlInitialized_[i] = false;
    }
}

void ObjectGl::setGlContext_(uint thread, GL::Context * c)
{
    MO_ASSERT(thread < glContext_.size(),
              "setGlContext_(" << thread << ", " << c << ") but "
              "glContext_.size() == " << glContext_.size());

    if (c != glContext_[thread])
    {
        needsInitGl_[thread] = true;
    }

    glContext_[thread] = c;
}

void ObjectGl::initGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::initGl_(" << thread << ")");

    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    MO_EXTEND_EXCEPTION(
        initGl(thread),
                "in ObjectGl '" << idName() << "', thread=" << thread
                );

    needsInitGl_[thread] = false;
    isGlInitialized_[thread] = true;
}


void ObjectGl::releaseGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::releaseGl_(" << thread << ")");

    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");

    MO_EXTEND_EXCEPTION(
        releaseGl(thread),
                "in ObjectGl '" << idName() << "', thread=" << thread
                );

    isGlInitialized_[thread] = false;
}

void ObjectGl::renderGl_(const GL::RenderSettings &rs, uint thread, Double time)
{
    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    // ---- set render modes -----

    if (depthTestMode() == DTM_OFF)
        MO_CHECK_GL( glDisable(GL_DEPTH_TEST) )
    else
        MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

    if (depthWriteMode() == DWM_OFF)
        MO_CHECK_GL( glDepthMask(false) )
    else
        MO_CHECK_GL( glDepthMask(true) );

    if (alphaBlendMode() == ABM_OFF)
        MO_CHECK_GL( glDisable(GL_BLEND) )
    else
    {
        MO_CHECK_GL( glEnable(GL_BLEND) );

        if (alphaBlendMode() == ABM_ADD)
            MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) )
        else
            MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
    }


    MO_EXTEND_EXCEPTION(
                renderGl(rs, thread, time),
                "in ObjectGl '" << idName() << "', thread=" << thread
                );

}

void ObjectGl::requestRender()
{
    Scene * scene = sceneObject();
    if (!scene)
        return;

    scene->render();
}

void ObjectGl::requestReinitGl()
{
    for (uint i=0; i<numberThreads(); ++i)
    {
        needsInitGl_[i] = true;
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

    if (!paramDepthTest_)
        curDepthTestMode_ = DTM_ON;
    else
    {
        if (paramDepthTest_->baseValue() == DTM_PARENT)
            curDepthTestMode_ = parent ? parent->depthTestMode() : DTM_ON;
        else
            curDepthTestMode_ = (DepthTestMode)paramDepthTest_->baseValue();
    }

    if (!paramDepthWrite_)
        curDepthWriteMode_ = DWM_ON;
    else
    {
        if (paramDepthWrite_->baseValue() == DWM_PARENT)
            curDepthWriteMode_ = parent ? parent->depthWriteMode() : DWM_ON;
        else
            curDepthWriteMode_ = (DepthWriteMode)paramDepthWrite_->baseValue();
    }

    if (!paramAlphaBlend_)
        curAlphaBlendMode_ = ABM_MIX;
    else
    {
        if (paramAlphaBlend_->baseValue() == ABM_PARENT)
            curAlphaBlendMode_ = parent ? parent->alphaBlendMode() : ABM_MIX;
        else
            curAlphaBlendMode_ = (AlphaBlendMode)paramAlphaBlend_->baseValue();
    }

    // pass to children

    for (auto c : childObjects())
        c->propagateRenderMode(this);
}

} // namespace MO
