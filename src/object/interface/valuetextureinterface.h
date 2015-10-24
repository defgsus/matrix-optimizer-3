/** @file ValueTextureInterface.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H

#include "types/time.h"

namespace MO {
namespace GL { class Texture; }

/** A texture output. */
class ValueTextureInterface
{
public:
    virtual ~ValueTextureInterface() { }

    /** Return a ready-to-use texture or NULL.
        Texture is not required to be bound on return. */
    virtual const GL::Texture * valueTexture(uint channel, const RenderTime& time) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUETEXTUREINTERFACE_H
