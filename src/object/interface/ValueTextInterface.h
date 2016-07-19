/** @file ValueTextInterface.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.05.2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUETEXTINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUETEXTINTERFACE_H

#include <QString>
#include <QPair>

#include "types/time.h"
#include "object/Object_fwd.h"

namespace MO {
namespace GL { class Texture; }

/** A texture value output. */
class ValueTextInterface
{
public:
    virtual ~ValueTextInterface() { }

    /** Return a text. */
    virtual QPair<QString, TextType> valueText(uint channel, const RenderTime& time) const = 0;

    /** Respond with connected editors */
    virtual void valueTextAddErrorMessage(int line, const QString& text) = 0;
};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUETEXTINTERFACE_H
