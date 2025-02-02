/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#include "GeometryObject.h"
#include "object/Scene.h"
#include "io/DataStream.h"
#include "gl/Drawable.h"
#include "geom/Geometry.h"
#include "geom/GeometryFactorySettings.h"
#include "geom/GeometryCreator.h"
#include "geom/GeometryModifierChain.h"
/*#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "param/parameterint.h"
*/
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(GeometryObject)

GeometryObject::GeometryObject()
    : Object        (),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings(this)),
      geometry_     (0)
{
    setName("Geometry");
    setNumberOutputs(ST_GEOMETRY, 1);
}

GeometryObject::~GeometryObject()
{
    resetCreator_();
    if (geometry_)
        geometry_->releaseRef("GeometryObject destroy");
}

void GeometryObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("geomo", 1);

    geomSettings_->serialize(io);
}

void GeometryObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    /*const int ver = */io.readHeader("geomo", 1);

    geomSettings_->deserialize(io);
}

void GeometryObject::createParameters()
{
    Object::createParameters();

    /*
    params()->beginParameterGroup("renderset", tr("render settings"));

    params()->endParameterGroup();
    */
}

void GeometryObject::onParametersLoaded()
{
    createGeometry_();
}

void GeometryObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
}

void GeometryObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void GeometryObject::getNeededFiles(IO::FileList &files)
{
    Object::getNeededFiles(files);

    geomSettings_->getNeededFiles(files);
}

const GEOM::Geometry* GeometryObject::geometry() const
{
    return geometry_;
}

const GEOM::GeometryFactorySettings& GeometryObject::getGeometrySettings() const
{
    geomSettings_->setObject(const_cast<GeometryObject*>(this));
    return *geomSettings_;
}

void GeometryObject::createGeometry_()
{
    // lazy-creation of resources?
    const bool lazy = sceneObject() ? sceneObject()->lazyFlag() : false;

    // -- create resources --

    if (lazy) // instantly
    {
        if (geometry_)
            geometry_->releaseRef("GeometryObject create relprev");
        geometry_ = new GEOM::Geometry;
        geomSettings_->setObject(this);
        geomSettings_->modifierChain()->execute(geometry_, this);
    }
    else // or in background
    {
        resetCreator_();
        /** @todo crash when GeometryCreator is attached as child and
            object is moved!! */
        creator_ = new GEOM::GeometryCreator(/*this*/);
        QObject::connect(creator_, &GEOM::GeometryCreator::succeeded,
                         [=](){ geometryCreated_(); });
        QObject::connect(creator_, &GEOM::GeometryCreator::failed,
                         [=](const QString& e){ geometryFailed_(e); });

        geomSettings_->setObject(this);
        creator_->setSettings(*geomSettings_);
        creator_->start();
    }
}

void GeometryObject::resetCreator_()
{
    if (creator_)
    {
        if (creator_->isRunning())
        {
            QObject::connect(creator_, SIGNAL(finished()),
                             creator_, SLOT(deleteLater()));
            creator_->discard();
        }
        else
            // XXX Should be deleteLater(), see above
            delete creator_;
    }
    creator_ = 0;
}

void GeometryObject::geometryCreated_()
{
    if (geometry_)
        geometry_->releaseRef("GeometryObject creator-finished relprev");
    geometry_ = creator_->takeGeometry();

    creator_->deleteLater();
    creator_ = 0;

    /* emit geometryChanged(); */
}

void GeometryObject::geometryFailed_(const QString& e)
{
    setErrorMessage(tr("Failed to create geometry:\n%1").arg(e));

    creator_->deleteLater();
    creator_ = 0;

    if (geometry_)
        geometry_->releaseRef("GeometryObject creator-failed relprev");
    geometry_ = 0;

    /* emit geometryChanged(); */
}

void GeometryObject::setGeometrySettings(const GEOM::GeometryFactorySettings & s)
{
    *geomSettings_ = s;
    geomSettings_->setObject(this);

    createGeometry_();
}

void GeometryObject::setGeometry(const GEOM::Geometry & g)
{
    if (!geometry_)
        geometry_ = new GEOM::Geometry;
    *geometry_ = g;

    /* emit geometryChanged(); */
}

const GEOM::Geometry * GeometryObject::valueGeometry(uint channel, const RenderTime& time) const
{
    Q_UNUSED(channel);
    Q_UNUSED(time);
    // XXX Can't do it, this function is const
    //if (!geometry())
    //    createGeometry();
    return geometry();
}


} // namespace MO
