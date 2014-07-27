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
#include "gl/shadersource.h"
#include "gl/cameraspace.h"

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
    const QString vertex_source =
            "#version 130\n"
            "// vertex attributes\n"
            "//in vec3 a_normal;\n"
            "//in vec4 a_color;\n"
            "in vec4 a_position;\n"
            "// shader uniforms (application specific)\n"
            "uniform mat4 u_projection;\n"
            "uniform mat4 u_view;\n"
            "// output of vertex shader\n"
            "//out vec3 v_normal;\n"
            "//out vec4 v_color;\n"
            "out vec3 v_pos;\n"
            "\n"
            "void main()\n"
            "{\n"
            "\t// pass attributes to fragment shader\n"
            "\t//v_normal = a_normal;\n"
            "\t//v_color = a_color;\n"
            "\tv_pos = a_position.xyz;\n"
            "\t// set final vertex position\n"
            "\tgl_Position = u_projection * u_view * a_position;\n"
            "}\n";

    const QString fragment_source =
            "#version 130\n"
            "// input from vertex shader\n"
            "//in vec3 v_normal;\n"
            "//in vec4 v_color;\n"
            "in vec3 v_pos;\n"
            "// shader uniforms (user)\n"
            "uniform vec3 u_light_pos;\n"
            "uniform vec3 u_light_color;\n"
            "uniform float u_shinyness;\n"
            "// output to rasterizer\n"
            "out vec4 color;\n"
            "\n"
            "void main()\n"
            "{\n"
            "\t// 'ambient color' or base color\n"
            "\t//vec3 ambcol = v_color.xyz;\n"
            "\t// normal to light source\n"
            "\t//vec3 light_normal = normalize( u_light_pos - v_pos );\n"
            "\t// dot-product of light normal and vertex normal gives linear light influence\n"
            "\t//float d = max(0.0, dot(v_normal, light_normal) );\n"
            "\t// shaping the linear light influence\n"
            "\t//float lighting = pow(d, 1.0 + u_shinyness);\n"
            "\t// adding the light to the base color\n"
            "\t//vec3 col = ambcol + lighting * u_light_color;\n"
            "\t// typical output of a fragment shader\n"
            "\t//color = vec4(clamp(col, 0.0, 1.0), 1.0);\n"
            "\tcolor = vec4(1.0, 0.5, 0.0, 1.0);\n"
            "}\n";

    draw_ = new GL::Drawable(this);
    GL::GeometryFactory::createCube(draw_->geometry(), 2);
    draw_->shaderSource()->setAttributeNamePosition("a_position");
    draw_->shaderSource()->setFragmentSource(fragment_source);
    draw_->shaderSource()->setVertextSource(vertex_source);
    draw_->createOpenGl();
}

void Model3d::renderGl(const GL::CameraSpace& cam, uint thread, Double )
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

    Mat4 mat = cam.viewMatrix() * transformation(thread, 0);
    glLoadMatrixf(&mat[0][0]);

    glColor3f(1,1,0);
    draw_->renderShader(cam.projectionMatrix(), mat);

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
