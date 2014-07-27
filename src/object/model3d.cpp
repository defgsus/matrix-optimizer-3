/** @file model3d.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "gl/geometryfactory.h"

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    :   ObjectGl(parent)
{
    setName("Model3D");
}

void Model3d::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);
    io.writeHeader("m3d", 1);
}

void Model3d::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);
    io.readHeader("m3d", 1);
}


void Model3d::initGl(uint /*thread*/)
{
    draw_ = new GL::Drawable(this);
    GL::GeometryFactory::createCube(draw_->geometry(), 2);
    draw_->createOpenGl();
}

void Model3d::renderGl(const Mat4& camMatrix, uint thread, Double )
{
    /*
    glColor3f(1,0,0);
    glBegin(GL_QUADS);
        glVertex2f(0,0);
        glVertex2f(10+sin(time),0);
        glVertex2f(10,10);
        glVertex2f(0,10);
    glEnd();
    */

    Mat4 mat = camMatrix * transformation(thread, 0);
    glLoadMatrixf(&mat[0][0]);

    glColor3f(1,1,0);
    draw_->render();

#if (1)
    glBegin(GL_LINES);
        glColor3f(1,1,1);
        for (int i=-10; i<=10; ++i)
        {
            glVertex3f(i,0,-10);
            glVertex3f(i,0,10);
            glVertex3f(-10,0,i);
            glVertex3f(10,0,i);
        }
        glColor3f(1,0,0);
        glVertex3f(0, 0.1, 0);
        glVertex3f(10, 0.1, 0);
        glColor3f(0,1,0);
        glVertex3f(0, 0.1, 0);
        glVertex3f(0, 10.1, 0);
        glColor3f(0,0,1);
        glVertex3f(0, 0.1, 0);
        glVertex3f(0, 0.1, 10);
    glEnd();
#endif
}





} // namespace MO
