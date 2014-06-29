/** @file transformation.cpp

    @brief object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "transformation.h"

namespace MO {

MO_REGISTER_OBJECT(Transformation)

Transformation::Transformation(QObject *parent) :
    Object(parent)
{
    setName("Transformation");
#if 0
    rotX_ = createFloatParameter("rotx", "rotation x", 0);
    rotX_.getValue(time);
#endif
}


} // namespace MO

