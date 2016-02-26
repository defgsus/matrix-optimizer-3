/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/26/2016</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUESHADERSOURCEINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUESHADERSOURCEINTERFACE_H


namespace MO {
namespace GL { class ShaderSource; }

/** An output for shader sources */
class ValueShaderSourceInterface
{
public:
    virtual ~ValueShaderSourceInterface() { }

    /** Return the shader source for the x'th pass or whatever. */
    virtual GL::ShaderSource valueShaderSource(uint channel) const = 0;
};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUESHADERSOURCEINTERFACE_H

