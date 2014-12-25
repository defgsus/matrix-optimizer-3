/** @file modulatoroutput.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include "modulatoroutput.h"

namespace MO {

ModulatorOutput::ModulatorOutput(Object * o, const QString& id)
    : p_object_     (o),
      p_id_         (id),
      p_channel_    (0)
{
}


} // namespace MO
