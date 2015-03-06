/** @file textureoverlay.cpp

    @brief Texture overlay object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#include "textureoverlay.h"
#include "io/datastream.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parameterfilename.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "gl/framebufferobject.h"
#include "img/image.h"
#include "util/texturesetting.h"
#include "util/colorpostprocessingsetting.h"
#include "math/vector.h"

using namespace gl;

namespace MO {

MO_REGISTER_OBJECT(TextureOverlay)

TextureOverlay::TextureOverlay(QObject * parent)
    : ObjectGl      (parent),
      ptype_        (PT_FLAT),
      actualPtype_  (ptype_),
      texture_      (new TextureSetting(this)),
      postProc_     (new ColorPostProcessingSetting(this))
{
    setName("TextureOverlay");

    deg90_ = MATH::rotate(Mat4(1), 90.f, Vec3(1,0,0));
}

void TextureOverlay::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("texover", 2);

    // v2
    texture_->serialize(io);
}

void TextureOverlay::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    const int ver = io.readHeader("texover", 2);

    if (ver >= 2)
        texture_->deserialize(io);
}

void TextureOverlay::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col");
        /*
        paramFilename_ = createFilenameParameter("imgfile", tr("image"), tr("Filename of the image"),
                                                 IO::FT_TEXTURE, ":/texture/mo_black.png");
        */
    params()->endParameterGroup();

    params()->beginParameterGroup("textureproj", tr("projection"));

        paramPType_ = params()->createSelectParameter("projtype", tr("projection"),
            tr("Selects the type of projection for the texture overlay"),
            { "flat", "fish", "equirect" },
            { tr("flat on screen"), tr("fish-eye"), tr("equi-rect") },
            { tr("Projects the texture as-is on screen"),
              tr("Projects a fisheye/fulldome master as seen from inside"),
              tr("Projects an equi-rectangular map as seen from inside") },
            { PT_FLAT, PT_FISHEYE, PT_EQUIRECT },
              ptype_, true, false);

        pos_influence_ = params()->createFloatParameter("posinf", tr("position influence"),
            tr("A multiplier for influencing the texture offset by the transformation position"),
                                          0.0, 0.01);
    params()->endParameterGroup();

    params()->beginParameterGroup("texturecolor", tr("color"));
    initParameterGroupExpanded("texturecolor");

        cr_ = params()->createFloatParameter("red", tr("red"), tr("Red amount of color multiplier"), 1.0, 0.1);
        cr_->setMinValue(0.0);
        cg_ = params()->createFloatParameter("green", tr("green"), tr("Green amount of color multiplier"), 1.0, 0.1);
        cg_->setMinValue(0.0);
        cb_ = params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of color multiplier"), 1.0, 0.1);
        cb_->setMinValue(0.0);
        ca_ = params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha amount of color multiplier"), 1.0, 0.1);
        ca_->setMinValue(0.0);

   params()-> endParameterGroup();

    params()->beginParameterGroup("texturepost", tr("color post-processing"));

        postProc_->createParameters("");

    params()->endParameterGroup();
}

void TextureOverlay::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == paramPType_)
    {
        ptype_ = (ProjectionType)paramPType_->baseValue();
        requestReinitGl();
    }

    if (texture_->needsReinit(p) || postProc_->needsRecompile(p))
        requestReinitGl();
}

void TextureOverlay::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

    ptype_ = (ProjectionType)paramPType_->baseValue();
}

void TextureOverlay::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    texture_->updateParameterVisibility();
    postProc_->updateParameterVisibility();

    pos_influence_->setVisible( paramPType_->baseValue() != PT_FLAT );
}

void TextureOverlay::getNeededFiles(IO::FileList &files)
{
    texture_->getNeededFiles(files, IO::FT_TEXTURE);
}


void TextureOverlay::initGl(uint /*thread*/)
{
    // --- texture ---

    texture_->initGl();


    // --- shader and quad ---

    quad_ = new GL::ScreenQuad(idName());

    QString defines;
    if (ptype_ == PT_EQUIRECT)
        defines += "#define MO_EQUIRECT\n";
    if (ptype_ == PT_FISHEYE)
        defines += "#define MO_FISHEYE\n";
    if (ptype_ == PT_FLAT)
        defines += "#define MO_FLAT\n";
    if (postProc_->isEnabled())
        defines += "#define MO_POST_PROCESS";

    quad_->create(":/shader/textureoverlay.vert",
                  ":/shader/textureoverlay.frag",
                  defines);

    // --- uniforms ---
    u_color_ = quad_->shader()->getUniform("u_color", true);

    if (ptype_ == PT_FLAT)
    {
        u_local_transform_ = quad_->shader()->getUniform("u_local_transform", true);
    }

    if (ptype_ == PT_EQUIRECT || ptype_ == PT_FISHEYE)
    {
        u_dir_matrix_ = quad_->shader()->getUniform("u_cvt", true);
        u_cam_angle_ = quad_->shader()->getUniform("u_cam_angle", true);
        u_sphere_offset_ = quad_->shader()->getUniform("u_sphere_offset", true);
    }

    if (postProc_->isEnabled())
        postProc_->getUniforms(quad_->shader());

    actualPtype_ = ptype_;
}

void TextureOverlay::releaseGl(uint /*thread*/)
{
    quad_->release();
    delete quad_;

    texture_->releaseGl();
}

void TextureOverlay::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans =
            (actualPtype_ == PT_EQUIRECT || actualPtype_ == PT_FISHEYE)
                ? transformation() * deg90_
                : transformation();
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    //const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    const Float posInf = -pos_influence_->value(time, thread);

    u_color_->setFloats(cr_->value(time, thread),
                        cg_->value(time, thread),
                        cb_->value(time, thread),
                        ca_->value(time, thread));

    if (actualPtype_ == PT_EQUIRECT || actualPtype_ == PT_FISHEYE)
    {
        u_cam_angle_->floats[0] = rs.cameraSpace().isCubemap() ?
                                    90.f : rs.cameraSpace().fieldOfView();

        u_sphere_offset_->setFloats(
                    cubeViewTrans[3][0] * posInf,
                    cubeViewTrans[3][1] * posInf,
                    cubeViewTrans[3][2] * posInf, 0);

        quad_->shader()->activate();

        //if (u_dir_matrix_)
            MO_CHECK_GL( glUniformMatrix4fv(u_dir_matrix_->location(), 1, GL_FALSE,
                                            &cubeViewTrans[0][0]) );
    }

    if (actualPtype_ == PT_FLAT)
    {
        // get local transformation
        Mat4 trans(1.0);
        calculateTransformation(trans, time, thread);
        trans = glm::inverse(trans);

        quad_->shader()->activate();

        MO_CHECK_GL( glUniformMatrix4fv(u_local_transform_->location(), 1, GL_FALSE,
                                        &trans[0][0]) );
    }

    if (postProc_->isEnabled())
        postProc_->updateUniforms(time, thread);

    texture_->bind();

    MO_CHECK_GL( glDepthMask(GL_FALSE) );
    quad_->draw(rs.cameraSpace().width(), rs.cameraSpace().height());
    MO_CHECK_GL( glDepthMask(GL_TRUE) );

}





} // namespace MO
