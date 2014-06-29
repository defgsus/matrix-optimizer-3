/** @file object3d.cpp

    @brief positional Object base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "object3d.h"

namespace MO {



Object3d::Object3d(QObject *parent) :
    Object(parent),
    transformation_(1.0)
{
}



} // namespace MO
