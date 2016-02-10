/** @file geometrycreator.cpp

    @brief Threaded GeometryFactory::createFromSettings invoker

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

#include "geometrycreator.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "geom/geometryfactorysettings.h"
#include "geom/geometrymodifierchain.h"
#include "io/log.h"

namespace MO {
namespace GEOM {

int GeometryCreator::instanceCounter_ = 0;

namespace { static QMutex instanceCounterMutex_; }

GeometryCreator::GeometryCreator(QObject *parent) :
    QThread     (parent),
    geometry_   (0),
    curGeometry_(0),
    settings_   (new GeometryFactorySettings(0)),
    timer_      (new QTimer(this)),
    mutex_      (new QMutex()),
    doStop_     (false),
    doDiscard_  (false)
{
    timer_->setSingleShot(false);
    timer_->setInterval(1000 / 10);
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer_()));

    connect(this, SIGNAL(started()), timer_, SLOT(start()));
}

GeometryCreator::~GeometryCreator()
{
    if (geometry_)
        geometry_->releaseRef();
    delete settings_;
    delete mutex_;
}

void GeometryCreator::setSettings(const GeometryFactorySettings & s)
{
    QMutexLocker lock(mutex_);
    *settings_ = s;
}

Geometry * GeometryCreator::takeGeometry()
{
    Geometry * g;
    {
        QMutexLocker lock(mutex_);
        g = geometry_;
        geometry_ = 0;
    }
    return g;
}

void GeometryCreator::run()
{
    doStop_ = doDiscard_ = false;

    // count thread instance and set theadname
    int instanceCounter;
    {
        QMutexLocker lock(&instanceCounterMutex_);
        instanceCounter = ++instanceCounter_;
    }
    setCurrentThreadName(QString("GEOM%1").arg(instanceCounter));

    MO_DEBUG_GL("GeometryCreator::run() instance = " << instanceCounter);

    GeometryFactorySettings settings(0);

    {
        QMutexLocker lock(mutex_);

        curGeometry_ = new Geometry();

        settings = *settings_;

    }

    bool success = false;

    try
    {
        settings.modifierChain()->execute(curGeometry_, settings.object());

        success = true;
    }
    catch (Exception & e)
    {
        MO_WARNING("GeometryFactory failed with:\n" << e.what());
        emit failed(e.what());

        QMutexLocker lock(mutex_);

        if (curGeometry_)
            curGeometry_->releaseRef();
        curGeometry_ = 0;
    }

    QMutexLocker lock(mutex_);

    // get geometry back
    if (geometry_)
        geometry_->releaseRef();
    geometry_ = curGeometry_;

    MO_DEBUG_GL("GeometryCreator::run() finished (instance = " << instanceCounter << ")");

    if (success && !doStop_ && !doDiscard_)
    {
        MO_DEBUG_GL("GeometryCreator::run() suceeded (instance = " << instanceCounter << ")");
        emit succeeded();
    }
}

void GeometryCreator::stop()
{
    doStop_ = true;

    QMutexLocker lock(mutex_);

    settings_->modifierChain()->stop();
}

void GeometryCreator::discard()
{
    doStop_ = doDiscard_ = true;
}

void GeometryCreator::onTimer_()
{
    if (!isRunning())
    {
        timer_->stop();
        return;
    }

    QMutexLocker lock(mutex_);

    /*if (loader_ && loader_->isLoading())
    {
        emit progress(loader_->progress());
        return;
    }*/

    if (settings_->modifierChain())
        emit settings_->modifierChain()->progress();
    else
    if (curGeometry_)
        emit progress(curGeometry_->progress());
}

} // namespace GEOM
} // namespace MO
