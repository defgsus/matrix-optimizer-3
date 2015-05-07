/** @file ValueTextureInterface.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H

#include "types/float.h"

namespace MO {
namespace GL { class Texture; }

/** A texture value output.
    *Currently* single channel... */
class ValueTextureInterface
{
public:
    virtual ~ValueTextureInterface() { }

    /** Return a ready-to-use texture or NULL.
        Texture is not required to be bound on return. */
    virtual const GL::Texture * valueTexture(Double time, uint thread) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H
