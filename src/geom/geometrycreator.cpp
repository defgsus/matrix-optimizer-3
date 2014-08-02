/** @file geometrycreator.cpp

    @brief Threaded GeometryFactory::createFromSettings invoker

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include "geometrycreator.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "geom/objloader.h"
#include "io/log.h"

namespace MO {
namespace GEOM {

GeometryCreator::GeometryCreator(QObject *parent) :
    QThread     (parent),
    geometry_   (0),
    settings_   (new GeometryFactorySettings()),
    loader_     (0)
{
}

GeometryCreator::~GeometryCreator()
{
    delete geometry_;
    delete settings_;
}

void GeometryCreator::setSettings(const GeometryFactorySettings & s)
{
    *settings_ = s;
}

Geometry * GeometryCreator::takeGeometry()
{
    auto g = geometry_;
    geometry_ = 0;
    return g;
}

void GeometryCreator::run()
{
    MO_DEBUG_GL("GeometryCreator::run()");

    auto g = new Geometry();

    // create a file loader if nescessary
    if (settings_->type == GeometryFactorySettings::T_FILE)
        loader_ = new ObjLoader();

    try
    {
        GeometryFactory::createFromSettings(g, settings_, loader_);
    }
    catch (Exception & e)
    {
        emit failed(e.what());

        delete g;
        g = 0;
    }

    delete loader_;
    loader_ = 0;

    delete geometry_;
    geometry_ = g;

    MO_DEBUG_GL("GeometryCreator::run() finished");
}

} // namespace GEOM
} // namespace MO
