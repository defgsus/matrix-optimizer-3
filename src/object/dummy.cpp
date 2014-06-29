/** @file dummy.cpp

    @brief Dummy object for skipping unknown objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "dummy.h"

namespace MO {

MO_REGISTER_OBJECT(Dummy)

Dummy::Dummy(QObject *parent) :
    Object(parent)
{
    setName("Dummy");
}


} // namespace MO
