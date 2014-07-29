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

namespace MO {
namespace GUI {
namespace UTIL {

GeometryCreator::GeometryCreator(QObject *parent) :
    QThread     (parent),
    geometry_   (0),
    settings_   (new GEOM::GeometryFactorySettings()),
    loader_     (0)
{
}

GeometryCreator::~GeometryCreator()
{
    delete geometry_;
    delete settings_;
}

void GeometryCreator::setSettings(const GEOM::GeometryFactorySettings & s)
{
    *settings_ = s;
}

GEOM::Geometry * GeometryCreator::takeGeometry()
{
    auto g = geometry_;
    geometry_ = 0;
    return g;
}

void GeometryCreator::run()
{
    auto g = new GEOM::Geometry();

    // create a file loader if nescessary
    if (settings_->type == GEOM::GeometryFactorySettings::T_FILE)
        loader_ = new GEOM::ObjLoader();

    try
    {
        GEOM::GeometryFactory::createFromSettings(g, settings_, loader_);
    }
    catch (Exception & e)
    {
        emit failed(e.what());

        delete g;
    }

    delete loader_;
    loader_ = 0;

    delete geometry_;
    geometry_ = g;
}

} // namespace UTIL
} // namespace GUI
} // namespace MO
