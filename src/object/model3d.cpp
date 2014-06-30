/** @file model3d.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    :   ObjectGl(parent)
{
    setName("Model3D");
}


void Model3d::initGl()
{

}

void Model3d::renderGl(Double )
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

    glLoadMatrixf(&transformation()[0][0]);

    glColor3f(1,1,1);
    glBegin(GL_LINES);
        for (int i=-10; i<=10; ++i)
        {
            glVertex3f(i,0,-10);
            glVertex3f(i,0,10);
            glVertex3f(-10,0,i);
            glVertex3f(10,0,i);

            glVertex3f(-10,i,-10);
            glVertex3f(10,i,-10);
            glVertex3f(i,-10,-10);
            glVertex3f(i,10,-10);
        }
    glEnd();
}





} // namespace MO
