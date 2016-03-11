/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/10/2016</p>
*/

#ifndef MOSRC_GL_GL_STATE_H
#define MOSRC_GL_GL_STATE_H

#include <QString>

namespace MO {
namespace GL {

class GlState
{
public:
    GlState();

    void get();

    QString toString() const;

    static GlState current();

    int activeTexture,
        arrayBufferBinding,
        dispatchIndirectBufferBinding,
        contextFlags,
        currentProgram,
        drawFramebufferBinding,
        readFramebufferBinding,
        elementArrayBufferBinding,
        pixelPackBufferBinding,
        pixelUnpackBufferBinding,
        programPipelineBinding,
        renderbufferBinding,
        sampleBuffers,
        shaderStorageBufferBinding,
        textureBinding1d,
        textureBinding1dArray,
        textureBinding2d,
        textureBinding2dArray,
        textureBinding2dMultisample,
        textureBinding2dMultisampleArray,
        textureBinding3d,
        textureBindingBuffer,
        textureBindingCubemap,
        textureBindingRectangle,
        uniformBufferBinding,
        vertexArrayBinding
    ;

};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_GL_STATE_H
