/** @file renderengine.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.01.2015</p>
*/

#include "renderengine.h"
#include "object/scene.h"
//#include "object/util/scenelock_p.h"
#include "object/util/objectglpath.h"
#include "gl/context.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

// ################################## RenderEngine::Private ########################################

class RenderEngine::Private
{
public:

    Private(RenderEngine * e)
        : engine        (e),
          scene         (0),
          threadIdx     (0),
          lastTime      (0)
    { }

    void setup();

    RenderEngine * engine;
    GL::Context * context;
    Scene * scene;
    ObjectGlPath path;
    uint threadIdx;
    Double lastTime;
};





// ################################## RenderEngine implementation ########################################


RenderEngine::RenderEngine(QObject *parent)
    : QObject       (parent),
      p_            (new Private(this))
{
}

RenderEngine::~RenderEngine()
{
    delete p_;
}


Scene * RenderEngine::scene() const
{
    return p_->scene;
}

uint RenderEngine::thread() const
{
    return p_->threadIdx;
}


void RenderEngine::setScene(Scene * s, GL::Context * context, uint thread)
{
    if (p_->path.isGlInitialized())
        p_->path.releaseGl();

    p_->scene = s;
    p_->threadIdx = thread;
    p_->context = context;
    p_->setup();
}

void RenderEngine::Private::setup()
{
    MO_DEBUG("RenderEngine::setup()");

    path.createPath(scene, context, threadIdx);

#ifdef MO_ENABLE_DEBUG
    path.dump(std::cout);
#endif
}


void RenderEngine::render(Double time)
{
    p_->context->makeCurrent();

    if (!p_->path.isGlInitialized())
        p_->path.initGl();

    p_->path.render(time);
}

void RenderEngine::release()
{
    if (p_->path.isGlInitialized())
    {
        p_->context->makeCurrent();
        p_->path.releaseGl();
    }
}

} // namespace MO
