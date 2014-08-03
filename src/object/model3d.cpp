/** @file model3d.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "geom/geometryfactory.h"
#include "gl/shadersource.h"
#include "gl/cameraspace.h"
#include "geom/geometrycreator.h"

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    : ObjectGl      (parent),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings),
      nextGeometry_ (0)
{
    setName("Model3D");
}

void Model3d::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("m3d", 2);

    // v2
    geomSettings_->serialize(io);
}

void Model3d::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    int ver = io.readHeader("m3d", 2);

    if (ver >= 2)
        geomSettings_->deserialize(io);
}


void Model3d::initGl(uint /*thread*/)
{
    draw_ = new GL::Drawable(idName());

    creator_ = new GEOM::GeometryCreator(this);
    connect(creator_, SIGNAL(finished()), this, SLOT(geometryCreated_()));
    connect(creator_, SIGNAL(failed(QString)), this, SLOT(geometryFailed_()));

    creator_->setSettings(*geomSettings_);
    creator_->start();

    //GEOM::GeometryFactory::createGridXZ(draw_->geometry(), 10, 10, true);
    //draw_->createOpenGl();
}

void Model3d::releaseGl(uint /*thread*/)
{
    if (draw_->isReady())
        draw_->releaseOpenGl();
    delete draw_;
    draw_ = 0;
}

void Model3d::geometryCreated_()
{
    nextGeometry_ = creator_->takeGeometry();
    creator_->deleteLater();

    requestRender();
}

void Model3d::geometryFailed_()
{
    creator_->deleteLater();
}

void Model3d::setGeometrySettings(const GEOM::GeometryFactorySettings & s)
{
    *geomSettings_ = s;
    requestReinitGl();
}

void Model3d::renderGl(const GL::CameraSpace& cam, uint thread, Double )
{
    Mat4 mat = cam.viewMatrix() * transformation(thread, 0);
//    glLoadMatrixf(&mat[0][0]);

    if (nextGeometry_)
    {
        draw_->setGeometry(nextGeometry_);
        draw_->createOpenGl();
        nextGeometry_ = 0;
    }

    if (draw_->isReady())
    {
        //draw_->renderAttribArrays();
        draw_->renderShader(cam.projectionMatrix(), mat);
    }
}





} // namespace MO
