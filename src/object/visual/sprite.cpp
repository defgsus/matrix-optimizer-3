/** @file sprite.cpp

    @brief A sprite, always facing the camera

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/19/2014</p>
*/

#ifndef MO_DISABLE_EXP

#include "sprite.h"
#include "io/datastream.h"
#include "io/log.h"
#include "math/vector.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/texture.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
//#include "param/parameterselect.h"
#include "util/texturesetting.h"

namespace MO {

MO_REGISTER_OBJECT(Sprite)

Sprite::Sprite(QObject * parent)
    : ObjectGl      (parent),
      draw_         (0),
      texture_      (new TextureSetting(this))
{
    setName("Sprite");

    //setDefaultDepthTestMode(DTM_OFF);
    initDefaultDepthWriteMode(DWM_OFF);
}

void Sprite::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("sprite", 2);

    // v2
    texture_->serialize(io);
}

void Sprite::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    const int ver = io.readHeader("sprite", 2);

    if (ver >= 2)
        texture_->deserialize(io);
}

void Sprite::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col");

    params()->endParameterGroup();

    params()->beginParameterGroup("color", tr("color"));
    initParameterGroupExpanded("color");

        cr_ = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        cg_ = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        cb_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ca_ = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("repeat", tr("repeat"));

        numRep_ = params()->createFloatParameter("numrep", "number instances",
                                   tr("The number of instances of the sprite to draw"), 1.0, 1.0);
        numRep_->setMinValue(1.0);
        timeSpan_ = params()->createFloatParameter("timespan", "time span",
                                    tr("The time in seconds from the first to the last instance"),
                                    -1.0, 0.05);
    params()->endParameterGroup();
}

void Sprite::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (texture_->needsReinit(p))
        requestReinitGl();
}

void Sprite::getNeededFiles(IO::FileList &files)
{
    texture_->getNeededFiles(files, IO::FT_TEXTURE);
}

void Sprite::initGl(uint /*thread*/)
{
    texture_->initGl();

    draw_ = new GL::Drawable(idName());

    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadVertexSource(":/shader/default.vert");
    src->loadFragmentSource(":/shader/default.frag");

    src->addDefine("#define MO_ENABLE_TEXTURE");
    //src->addDefine("#define MO_ENABLE_BILLBOARD");

    draw_->setShaderSource(src);

    GEOM::GeometryFactory::createQuad(draw_->geometry(), 2, 2, true);

    draw_->createOpenGl();
}

void Sprite::releaseGl(uint /*thread*/)
{
    texture_->releaseGl();

    if (draw_->isReady())
        draw_->releaseOpenGl();

    delete draw_;
    draw_ = 0;
}


void Sprite::renderGl(const GL::RenderSettings& rs, uint thread, Double orgtime)
{
    if (draw_->isReady())
    {
        texture_->bind();

        const uint numrep = numRep_->value(orgtime, thread);
        //const uint numrep0 = numrep>1? numrep-1 : 1;
        const Float timespan = timeSpan_->value(orgtime, thread);

        // paint each instance
        for (uint i=0; i<numrep; ++i)
        {
            const Float t = (Float)i / numrep;

            const Float time = orgtime + timespan * t;

#if (1) // billboarding on cpu

            // get parent transformation
            Mat4 orgtrans = parentObject()->transformation();
            // apply local transformation with time-offset
            calculateTransformation(orgtrans, time, thread);

            const Mat4 cam = rs.cameraSpace().viewMatrix();
            /*adj_cam[0] = Vec4(1,0,0,0);
            adj_cam[1] = Vec4(0,1,0,0);
            adj_cam[2] = Vec4(0,0,1,0);
            const Mat4 viewtrans = adj_cam * orgtrans;//rs.cameraSpace().viewMatrix() * orgtrans;
            */
            // -------- billboard matrix ----------

            // extract current position
            Vec3 pos = Vec3(orgtrans[3][0], orgtrans[3][1], orgtrans[3][2])
                       + Vec3(cam[3][0], cam[3][1], cam[3][2]);
                        //Vec3(viewtrans * Vec4(0,0,0,1));
                        //Vec3(viewtrans[3][0], viewtrans[3][1], viewtrans[3][2]);

            // forward vector
            Vec3 f = MATH::normalize_safe(pos);
            // up vector
            Vec3 u = Vec3(0,1,0);//Vec3(viewtrans[0][1], viewtrans[1][1], viewtrans[2][1]);
            // right vector
            Vec3 s = MATH::normalize_safe(glm::cross(f, u));
            // rebuild up to avoid distortion
            u = glm::cross(s, f);

            Mat4 lookm(1);

            const Float
                    scalex = glm::length(orgtrans[0]),
                    scaley = glm::length(orgtrans[1]);

            lookm[0][0] = s.x * scalex;
            lookm[0][1] = s.y * scalex;
            lookm[0][2] = s.z * scalex;
            lookm[1][0] = u.x * scaley;
            lookm[1][1] = u.y * scaley;
            lookm[1][2] = u.z * scaley;
            lookm[2][0] =-f.x;
            lookm[2][1] =-f.y;
            lookm[2][2] =-f.z;
            lookm[3] = orgtrans[3];

            const Mat4 trans = lookm;//orgtrans * lookm;
            const Mat4 cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
            const Mat4 viewTrans = rs.cameraSpace().viewMatrix() * trans;

#else
            // get parent transformation
            Mat4 trans = parentObject()->transformation(thread, 0);
            // apply local transformation with time-offset
            calculateTransformation(trans, time, thread);

            const Mat4 cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
            const Mat4 viewTrans = rs.cameraSpace().viewMatrix() * trans;
#endif

            draw_->setAmbientColor(
                        cr_->value(time),
                        cg_->value(time),
                        cb_->value(time),
                        ca_->value(time));

            draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                                cubeViewTrans, viewTrans, trans, &rs.lightSettings());

        }
    }
}





} // namespace MO

#endif // #ifndef MO_DISABLE_EXP
