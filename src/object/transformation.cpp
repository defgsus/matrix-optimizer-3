/** @file transformation.cpp

    @brief abstract object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "transformation.h"

namespace MO {

Transformation::Transformation(QObject *parent) :
    Object(parent)
{
    setName("Transformation");
}


} // namespace MO

