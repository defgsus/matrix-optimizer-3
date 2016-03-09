/** @file testprojectionrenderer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.10.2014</p>
*/

#include <QVector>

#include "testprojectionrenderer.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "gl/drawable.h"
#include "gl/screenquad.h"
#include "geom/geometry.h"
#include "geom/objloader.h"
#include "projectormapper.h"

namespace MO {

class TestProjectionRenderer::Private
{
public:

    GL::Drawable * stuff;
    ProjectionSystemSettings set, oldset;

    struct Projector
    {
        Projector() : fbo(0), warp(0) { }
        Projector(const Projector&) : fbo(0), warp(0) { }
        ~Projector()
        {
            if (fbo && fbo->isCreated())
                fbo->release();
            delete fbo;
            if (warp)
                warp->release();
            delete warp;
        }

        GL::FrameBufferObject * fbo;
        GL::ScreenQuad * warp;
    };
    QVector<Projector> projector;

    Private()
      : stuff     (0)
    { }

    ~Private()
    {
        delete stuff;
        projector.clear();
    }

    /** To be called with gl context */
    void recalc();
};



TestProjectionRenderer::TestProjectionRenderer()
    : p_    (new Private())
{
}

TestProjectionRenderer::~TestProjectionRenderer()
{
    delete p_;
}

const ProjectionSystemSettings& TestProjectionRenderer::settings() const
{
    return p_->set;
}

void TestProjectionRenderer::setSettings(const ProjectionSystemSettings & set)
{
    p_->oldset = p_->set;
    p_->set = set;
}

void TestProjectionRenderer::Private::recalc()
{
    // opengl objects

    projector.resize(set.numProjectors());

    bool samedome =
               set.numProjectors() == oldset.numProjectors()
            && set.domeSettings() == oldset.domeSettings();

    for (uint i=0; i<set.numProjectors(); ++i)
    {
        Projector & proj = projector[i];

        // update size
        if (proj.fbo && (proj.fbo->width() != (uint)set.cameraSettings(i).width()
                        || proj.fbo->height() != (uint)set.cameraSettings(i).height()))
        {
            if (proj.fbo->isCreated())
                proj.fbo->release();
            delete proj.fbo;
            proj.fbo = 0;
        }

        // init framebuffer for each object
        if (!proj.fbo)
        {
            const CameraSettings cam = set.cameraSettings(i);

            proj.fbo = new GL::FrameBufferObject(cam.width(), cam.height(),
                                                 gl::GL_RGBA,
                                                 gl::GL_FLOAT);
            proj.fbo->create();
        }

        // force recreation of warp geom
        if (!samedome
            || !(set.projectorSettings(i) == oldset.projectorSettings(i))
            || !(set.cameraSettings(i) == oldset.cameraSettings(i)))
        {
            if (proj.warp)
                proj.warp->release();
            delete proj.warp;
            proj.warp = 0;
        }

        // init warp quad
        if (!proj.warp)
        {
            ProjectorMapper mapper;
            mapper.setSettings(set.domeSettings(), set.projectorSettings(i));
            proj.warp = new GL::ScreenQuad(QString("warp_%1").arg(i));
            mapper.getWarpDrawable(
                        set.cameraSettings(i),
                        proj.warp);
        }
    }

    oldset = set;
}


void TestProjectionRenderer::releaseGl()
{
    if (p_->stuff)
    {
        if (p_->stuff->isReady())
            p_->stuff->releaseOpenGl();
        delete p_->stuff;
        p_->stuff = 0;
    }

    p_->projector.clear();
}

void TestProjectionRenderer::renderSlice(uint index)
{
    MO_ASSERT(index < p_->set.numProjectors(), "TextProjectionRenderer::renderSlice(" << index << ") "
              "index out of range (" << p_->set.numProjectors() << ")");

    // create test scene
    if (!p_->stuff)
    {
        GEOM::ObjLoader loader;
        loader.loadFile(":/model/projectortestdome.obj");

        auto geom = new GEOM::Geometry;
        loader.getGeometry(geom);

        p_->stuff = new GL::Drawable("_project_test");
        p_->stuff->setGeometry(geom);

        p_->stuff->createOpenGl();
    }

    // update each projector's data
    p_->recalc();

    Private::Projector & proj = p_->projector[index];

    // render into buffer
    MO_ASSERT(proj.fbo, "TestProjectionRenderer::renderSlice(" << index << ") with no slice framebuffer");
    proj.fbo->bind();

        MO_CHECK_GL( gl::glViewport(0,0,p_->set.cameraSettings(index).width(),
                                        p_->set.cameraSettings(index).height()) );
        MO_CHECK_GL( gl::glClearColor(0.1,0.2,0.3,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT) );

        const Mat4
                pro = p_->set.cameraSettings(index).getProjectionMatrix(),
                trans = p_->set.cameraSettings(index).getViewMatrix();

        p_->stuff->renderShader(pro, trans, trans, trans);

    proj.fbo->unbind();

}

GL::Texture * TestProjectionRenderer::renderSliceTexture(uint index)
{
    // render the slice in it's framebuffer
    renderSlice(index);

    // create another framebuffer to produce the warp in
    GL::FrameBufferObject fbo(p_->set.cameraSettings(index).width(),
                              p_->set.cameraSettings(index).height(),
                              gl::GL_RGBA, gl::GL_FLOAT);

    fbo.create();

    fbo.bind();

        MO_CHECK_GL( gl::glViewport(0,0,fbo.width(),fbo.height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        Private::Projector & proj = p_->projector[index];

        GL::Texture::setActiveTexture(0);
        proj.fbo->colorTexture()->bind();

        proj.warp->draw(fbo.width(), fbo.height());

    fbo.unbind();

    GL::Texture * tex = fbo.takeColorTexture();

    fbo.release();

    return tex;
}

} // namespace MO
