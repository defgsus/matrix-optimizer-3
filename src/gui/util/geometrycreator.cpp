/** @file geometrycreator.cpp

    @brief Threaded GeometryFactory::createFromSettings invoker

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include "geometrycreator.h"
#include "gl/geometry.h"
#include "gl/geometryfactory.h"

namespace MO {
namespace GUI {
namespace UTIL {

GeometryCreator::GeometryCreator(QObject *parent) :
    QThread     (parent),
    geometry_   (0),
    settings_   (new GL::GeometryFactorySettings())
{
}

GeometryCreator::~GeometryCreator()
{
    delete geometry_;
    delete settings_;
}

void GeometryCreator::setSettings(const GL::GeometryFactorySettings & s)
{
    *settings_ = s;
}

GL::Geometry * GeometryCreator::takeGeometry()
{
    auto g = geometry_;
    geometry_ = 0;
    return g;
}

void GeometryCreator::run()
{
    auto g = new GL::Geometry();

    try
    {
        GL::GeometryFactory::createFromSettings(g, settings_);
    }
    catch (Exception & e)
    {
        emit failed(e.what());

        delete g;
    }

    delete geometry_;
    geometry_ = g;
}

} // namespace UTIL
} // namespace GUI
} // namespace MO
