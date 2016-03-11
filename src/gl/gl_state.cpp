/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/10/2016</p>
*/

#include <iomanip>
#include <sstream>

#include "opengl.h"
#include "gl_state.h"


namespace MO {
namespace GL {

GlState::GlState()
{
    memset(this, 0, sizeof(GlState));
}

GlState GlState::current()
{
    GlState state;
    state.get();
    return state;
}

void GlState::get()
{
    using namespace gl;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBufferBinding);
    glGetIntegerv(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &dispatchIndirectBufferBinding);
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebufferBinding);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebufferBinding);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBufferBinding);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pixelPackBufferBinding);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &pixelUnpackBufferBinding);
    glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, &programPipelineBinding);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbufferBinding);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &shaderStorageBufferBinding);
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &textureBinding1d);
    glGetIntegerv(GL_TEXTURE_BINDING_1D_ARRAY, &textureBinding1dArray);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding2d);
    glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &textureBinding2dArray);
    glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &textureBinding2dMultisample);
    glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, &textureBinding2dMultisampleArray);
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &textureBinding3d);
    glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &textureBindingBuffer);
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &textureBindingCubemap);
    glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &textureBindingRectangle);
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &uniformBufferBinding);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);
}

QString GlState::toString() const
{
    std::stringstream s;
    s << std::left;
#define MO__PRINT(name__, val__) \
    s << std::setw(40) << #name__ << " "; \
    if (val__) s << val__; s << std::endl;

    MO__PRINT(GL_ACTIVE_TEXTURE, activeTexture);
    MO__PRINT(GL_ARRAY_BUFFER_BINDING, arrayBufferBinding);
    MO__PRINT(GL_DISPATCH_INDIRECT_BUFFER_BINDING, dispatchIndirectBufferBinding);
    MO__PRINT(GL_CONTEXT_FLAGS, contextFlags);
    MO__PRINT(GL_CURRENT_PROGRAM, currentProgram);
    MO__PRINT(GL_DRAW_FRAMEBUFFER_BINDING, drawFramebufferBinding);
    MO__PRINT(GL_READ_FRAMEBUFFER_BINDING, readFramebufferBinding);
    MO__PRINT(GL_ELEMENT_ARRAY_BUFFER_BINDING, elementArrayBufferBinding);
    MO__PRINT(GL_PIXEL_PACK_BUFFER_BINDING, pixelPackBufferBinding);
    MO__PRINT(GL_PIXEL_UNPACK_BUFFER_BINDING, pixelUnpackBufferBinding);
    MO__PRINT(GL_PROGRAM_PIPELINE_BINDING, programPipelineBinding);
    MO__PRINT(GL_RENDERBUFFER_BINDING, renderbufferBinding);
    MO__PRINT(GL_SAMPLE_BUFFERS, sampleBuffers);
    MO__PRINT(GL_SHADER_STORAGE_BUFFER_BINDING, shaderStorageBufferBinding);
    MO__PRINT(GL_TEXTURE_BINDING_1D, textureBinding1d);
    MO__PRINT(GL_TEXTURE_BINDING_1D_ARRAY, textureBinding1dArray);
    MO__PRINT(GL_TEXTURE_BINDING_2D, textureBinding2d);
    MO__PRINT(GL_TEXTURE_BINDING_2D_ARRAY, textureBinding2dArray);
    MO__PRINT(GL_TEXTURE_BINDING_2D_MULTISAMPLE, textureBinding2dMultisample);
    MO__PRINT(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, textureBinding2dMultisampleArray);
    MO__PRINT(GL_TEXTURE_BINDING_3D, textureBinding3d);
    MO__PRINT(GL_TEXTURE_BINDING_BUFFER, textureBindingBuffer);
    MO__PRINT(GL_TEXTURE_BINDING_CUBE_MAP, textureBindingCubemap);
    MO__PRINT(GL_TEXTURE_BINDING_RECTANGLE, textureBindingRectangle);
    MO__PRINT(GL_UNIFORM_BUFFER_BINDING, uniformBufferBinding);
    MO__PRINT(GL_VERTEX_ARRAY_BINDING, vertexArrayBinding);

#undef MO__PRINT

    return QString::fromStdString(s.str());
}

} // namespace GL
} // namespace MO
