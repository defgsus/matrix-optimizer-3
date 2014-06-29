/** @file microphone.cpp

    @brief basic microphone object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "microphone.h"

namespace MO {

MO_REGISTER_OBJECT(Microphone)

Microphone::Microphone(QObject *parent) :
    Object(parent)
{
    setName("Microphone");
}


} // namespace MO
