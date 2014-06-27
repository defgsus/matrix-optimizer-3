/** @file parameter.cpp

    @brief General purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "parameter.h"

namespace MO {

Parameter::Parameter(const QString &idName, QObject *parent) :
    Object(idName, parent)
{
}


} // namespace MO
