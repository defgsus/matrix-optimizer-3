/** @file projectormapper_gl.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.10.2014</p>
*/


#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <QSet>

#include "ProjectorMapper.h"
#include "io/log.h"
#include "gl/opengl.h"
#include "gl/Texture.h"
#include "gl/FrameBufferObject.h"
#include "geom/Geometry.h"
#include "gl/Drawable.h"
#include "ProjectionSystemSettings.h"


namespace MO {




GL::Texture * ProjectorMapper::renderBlendTexture(uint index, const ProjectionSystemSettings &settings)
{
    auto geom = new GEOM::Geometry();

    for (uint i=0; i<settings.numProjectors(); ++i)
    if (i != index)
    {
        ProjectorMapper other;
        other.setSettings(settings.domeSettings(), settings.projectorSettings(i));

        //getBlendGeometry(other, geom);
        getIntersectionGeometry(other, geom);
    }

    GL::FrameBufferObject fbo(set_.width(), set_.height(), gl::GL_RGBA, gl::GL_FLOAT);

    fbo.create();

    fbo.bind();

        MO_CHECK_GL( gl::glViewport(0,0,fbo.width(),fbo.height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        GL::Drawable draw("blend_geometry");
        draw.setGeometry(geom);
        draw.createOpenGl();

        const Mat4
                proj = glm::ortho(0.f,1.f, 0.f,1.f, 0.001f,2.f),
                trans = glm::translate(Mat4(1), Vec3(0,0,-0.5));
        draw.renderShader(proj, trans, trans, trans);

        draw.releaseOpenGl();

    fbo.unbind();

    GL::Texture * tex = fbo.takeColorTexture();

    fbo.release();

    return tex;
}








#if 0
GL::Texture * ProjectorMapper::renderBlendTexture(const ProjectorMapper &other)
{
    GL::FrameBufferObject fbo(set_.width(), set_.height(), gl::GL_RGBA, gl::GL_FLOAT);

    fbo.create();

    fbo.bind();

        MO_CHECK_GL( gl::glViewport(0,0,fbo.width(),fbo.height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        auto geom = new GEOM::Geometry();
        //getBlendGeometry(other, geom);
        getIntersectionGeometry(other, geom);

        GL::Drawable draw("blend_geometry");
        draw.setGeometry(geom);
        draw.createOpenGl();

        const Mat4
                proj = glm::ortho(0.f,1.f, 0.f,1.f, 0.001f,2.f),
                trans = glm::translate(Mat4(1), Vec3(0,0,-0.5));
        draw.renderShader(proj, trans, trans, trans);

    fbo.unbind();

    draw.releaseOpenGl();

    GL::Texture * tex = fbo.takeColorTexture();

    fbo.release();

    return tex;
}
#endif



#if 0
GL::Texture * ProjectorMapper::renderBlendTextureNew(const ProjectorMapper &other)
{
    GL::FrameBufferObject fbo(set_.width(), set_.height(), gl::GL_RGBA, gl::GL_FLOAT);

    fbo.create();

    fbo.bind();

        MO_CHECK_GL( gl::glViewport(0,0,fbo.width(),fbo.height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        auto geom = new GEOM::Geometry();
        //getBlendGeometry(other, geom);
        getIntersectionGeometry(other, geom);

        GL::Drawable draw("blend_geometry");
        draw.setGeometry(geom);
        draw.createOpenGl();

        const Mat4
                proj = glm::ortho(0.f,1.f, 0.f,1.f, 0.001f,2.f),
                trans = glm::translate(Mat4(1), Vec3(0,0,-0.5));
        draw.renderShader(proj, trans, trans, trans);

    fbo.unbind();

    draw.releaseOpenGl();

    GL::Texture * tex = fbo.takeColorTexture();

    fbo.release();

    return tex;
}
#endif


} // namespace MO


