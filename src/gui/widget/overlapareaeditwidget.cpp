/** @file overlapareaeditwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include "overlapareaeditwidget.h"
#include "projection/projectionsystemsettings.h"
#include "projection/projectormapper.h"
#include "projection/projectorblender.h"
#include "geom/geometry.h"
#include "geom/tesselator.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"
#include "gl/texture.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "math/interpol.h"

namespace MO {
namespace GUI {


OverlapAreaEditWidget::OverlapAreaEditWidget(QWidget *parent)
    : Basic3DWidget (RM_DIRECT, parent),
      settings_     (new ProjectionSystemSettings()),
      projectorIndex_(0),
      triangleGeom_ (0),
      lineGeom_     (0),
      blendGeom_    (0),
      triangles_    (0),
      lines_        (0),
      blends_       (0),
      blendTex_     (0),
      showTesselation_(true),
      showBlend_    (true),
      setNewBlendTexture_(true)
{
    setObjectName("_OverlapAreaEditWidget");

    //createGeometry_();

    viewSet(VD_FRONT, 1);
    viewSetOrthoScale(1);
}

OverlapAreaEditWidget::~OverlapAreaEditWidget()
{
    delete blends_;
    delete lines_;
    delete triangles_;
    if (blendGeom_)
        blendGeom_->releaseRef();
    if (lineGeom_)
        lineGeom_->releaseRef();
    if (triangleGeom_)
        triangleGeom_->releaseRef();
    delete settings_;
}

void OverlapAreaEditWidget::setSettings(const ProjectionSystemSettings& set, uint idx)
{
    MO_ASSERT(set.numProjectors(), "empty ProjectionSystemSettings given to OverlapAreaEditWidget");

    *settings_ = set;
    projectorIndex_ = std::min(set.numProjectors()-1u, idx);

    //Float aspect = set.projectorSettings(projectorIndex_).aspect();
    //viewSetOrthoScale(1.f * aspect, 1.f);

    updateFboSize_();

    createGeometry_();

    update();
}

void OverlapAreaEditWidget::resizeGL(int w, int h)
{
    Basic3DWidget::resizeGL(w, h);
    updateFboSize_();
}

void OverlapAreaEditWidget::updateFboSize_()
{
    const ProjectorSettings& proj = settings_->projectorSettings(projectorIndex_);

    Float aspect = (Float)proj.width() / proj.height();
    setFboSize(height() * aspect, height());
}

void OverlapAreaEditWidget::createGeometry_()
{
    const ProjectorSettings& proj = settings_->projectorSettings(projectorIndex_);
    const DomeSettings& dome = settings_->domeSettings();

    GEOM::Tesselator tess;
    ProjectorMapper mapper;
    mapper.setSettings(dome, proj);

    lineGeom_ = new GEOM::Geometry();
    blendGeom_ = new GEOM::Geometry();

    // -- outline --
    lineGeom_->setColor(0.3,0.3,0.3,1);
    const GEOM::Geometry::IndexType
            p1 = lineGeom_->addVertex(-1,-1,0),
            p2 = lineGeom_->addVertex( 1,-1,0),
            p3 = lineGeom_->addVertex( 1, 1,0),
            p4 = lineGeom_->addVertex(-1, 1,0);
    lineGeom_->addLine(p1,p2);
    lineGeom_->addLine(p2,p3);
    lineGeom_->addLine(p3,p4);
    lineGeom_->addLine(p4,p1);

    // -- overlap area --
/*
    for (uint idx = 0; idx < proj.numOverlapAreas(); ++idx)
    {
        lineGeom_->setColor(0.5,1,0.5,1);

        const QVector<Vec2> & area = proj.overlapArea(idx);

        if (!showTesselation_)
        {
            // draw outline
            GEOM::Geometry::IndexType p1=0, p2;
            for (int i=0; i<=area.size(); ++i)
            {
                p2 = lineGeom_->addVertex(area[i%area.size()][0], area[i%area.size()][1], 0);
                if (i>0)
                    lineGeom_->addLine(p1, p2);
                p1 = p2;
            }
        }
        else
        {
            tess.tesselate(area);
            tess.getGeometry(*lineGeom_, false);
        }

        if (showBlend_)
        {
            blendGeom_ = new GEOM::Geometry();
            blendGeom_->clear();
            mapper.getBlendGeometry(area, blendGeom_);
        }
    }

*/

    for (uint idx = 0; idx < settings_->numProjectors(); ++idx)
    if (idx != projectorIndex_)
    {
        ProjectorMapper omapper;
        omapper.setSettings(dome, settings_->projectorSettings(idx));

        QVector<Vec3> domec;
        QVector<Vec2> slicec;

        omapper.mapToDome(omapper.createOutline(0.05), domec);
        mapper.mapFromDome(domec, slicec);

        // draw outline
        GEOM::Geometry::IndexType p1=0, p2;
        Float ldist = 0;
        for (int j=0; j<=slicec.size(); ++j)
        {
            const Vec2 p = Vec2(
                        slicec[j % slicec.size()][0],
                        slicec[j % slicec.size()][1]);

            // distance from slice
            const Float dist = std::max(std::abs(p[0]), std::abs(p[1])),
            // fadeout for outsiders
                        colf = 1.f - MATH::smoothstep(1.f, 1.3f, std::max(dist, ldist));

            lineGeom_->setColor(0.5,1,0.5, colf);

            p2 = lineGeom_->addVertex(p[0], p[1], 0);
            if (j>0)
                lineGeom_->addLine(p1, p2);
            p1 = p2;
            ldist = dist;
        }

        // XXX currently a little hack to display a blend edge

        domec.clear();
        slicec.clear();
        omapper.mapToDome(omapper.createOutline(0.05, 0.25), domec);
        mapper.mapFromDome(domec, slicec);

        // draw inner outline
        p1=0;
        ldist = 0;
        for (int j=0; j<=slicec.size(); ++j)
        {
            const Vec2 p = Vec2(
                        slicec[j % slicec.size()][0],
                        slicec[j % slicec.size()][1]);

            // distance from slice
            const Float dist = std::max(std::abs(p[0]), std::abs(p[1])),
            // fadeout for outsiders
                        colf = 1.f - MATH::smoothstep(1.f, 1.3f, std::max(dist, ldist));

            lineGeom_->setColor(0.6,0.3,0.1, colf);

            p2 = lineGeom_->addVertexAlways(p[0], p[1], 0);
            if (j>0)
                lineGeom_->addLine(p1, p2);
            p1 = p2;
            ldist = dist;
        }


        // the blend geometry

        //mapper.getBlendGeometry(omapper, blendGeom_);
        //mapper.getIntersectionGeometry(omapper, blendGeom_);

    }

    setNewBlendTexture_ = true;

    GEOM::GeometryFactory::createQuad(blendGeom_, 2, 2);

    Float aspect = settings_->projectorSettings(projectorIndex_).aspect();
    lineGeom_->scale(aspect, 1, 1);
    blendGeom_->scale(aspect, 1, 1);
}

void OverlapAreaEditWidget::initGL()
{
    triangles_ = new GL::Drawable(objectName() + "_triangles");
    lines_ = new GL::Drawable(objectName() + "_lines");
    blends_ = new GL::Drawable(objectName() + "_blends");
}

void OverlapAreaEditWidget::releaseGL()
{
    if (triangles_ && triangles_->isReady())
        triangles_->releaseOpenGl();
    if (lines_ && lines_->isReady())
        lines_->releaseOpenGl();
    if (blends_ && blends_->isReady())
        blends_->releaseOpenGl();
    if (blendTex_ && blendTex_->isAllocated())
        blendTex_->release();
    delete blendTex_;
    blendTex_ = 0;
}

void OverlapAreaEditWidget::drawGL(const Mat4& projection,
                                   const Mat4& cubeViewTrans,
                                   const Mat4& viewTrans,
                                   const Mat4& trans)
{
    if (lineGeom_)
    {
        lines_->setGeometry(lineGeom_);
        lineGeom_ = 0;

        lines_->createOpenGl();
    }

    if (triangleGeom_)
    {
        triangles_->setGeometry(triangleGeom_);
        triangleGeom_ = 0;

        triangles_->createOpenGl();
    }

    if (blendGeom_)
    {
        blends_->setGeometry(blendGeom_);
        blendGeom_ = 0;

        auto src = new GL::ShaderSource();
        src->loadDefaultSource();
        src->addDefine(
                    "#define MO_ENABLE_TEXTURE");
        blends_->setShaderSource(src);
        blends_->createOpenGl();
    }

    if (setNewBlendTexture_)
    {
        if (blendTex_ && blendTex_->isAllocated())
            blendTex_->release();
        delete blendTex_;
        blendTex_ = 0;
        setNewBlendTexture_ = false;
    }

    if (!blendTex_)
    {
        ProjectorBlender blender(settings_);
        blendTex_ = blender.renderBlendTexture(projectorIndex_);
    }


    using namespace gl;

    int pixelsize = devicePixelRatio();
    MO_CHECK_GL( gl::glViewport(0,0,pixelsize * width(), pixelsize * height()) );
    MO_CHECK_GL( gl::glClearColor(0.5,0.5,0.5,1) );
    MO_CHECK_GL( gl::glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( gl::glDisable(GL_DEPTH_TEST) );
    MO_CHECK_GL( gl::glEnable(GL_BLEND) );

    if (blends_->isReady())
    {
        //MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE) );
        MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

        blendTex_->bind();
        blends_->renderShader(projection, cubeViewTrans, viewTrans, trans);
    }

    if (triangles_->isReady())
        triangles_->renderShader(projection, cubeViewTrans, viewTrans, trans);

    MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

    if (lines_->isReady())
        lines_->renderShader(projection, cubeViewTrans, viewTrans, trans);
}

} // namespace GUI
} // namespace MO
