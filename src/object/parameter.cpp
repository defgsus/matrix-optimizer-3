/** @file parameter.cpp

    @brief General purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "parameter.h"

namespace MO {

MO_REGISTER_OBJECT(Parameter)

Parameter::Parameter(QObject *parent) :
    Object(parent)
{
}


} // namespace MO
