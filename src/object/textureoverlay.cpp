/** @file textureoverlay.cpp

    @brief Texture overlay object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#include "textureoverlay.h"
#include "io/datastream.h"
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

namespace MO {

MO_REGISTER_OBJECT(TextureOverlay)

TextureOverlay::TextureOverlay(QObject * parent)
    : ObjectGl      (parent),
      ptype_        (PT_FLAT),
      actualPtype_  (ptype_),
      texture_      (new TextureSetting(this))
{
    setName("TextureOverlay");
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

    beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col");
        /*
        paramFilename_ = createFilenameParameter("imgfile", tr("image"), tr("Filename of the image"),
                                                 IO::FT_TEXTURE, ":/texture/mo_black.png");
        */
    endParameterGroup();

    beginParameterGroup("textureproj", tr("projection"));

        paramPType_ = createSelectParameter("projtype", tr("projection"),
            tr("Selects the type of projection for the texture overlay"),
            { "flat", "fish", "equirect" },
            { tr("flat on screen"), tr("fish-eye"), tr("equi-rect") },
            { tr("Projects the texture as-is on screen"),
              tr("Projects a fisheye/fulldome master as seen from inside"),
              tr("Projects an equi-rectangular map as seen from inside") },
            { PT_FLAT, PT_FISHEYE, PT_EQUIRECT },
              ptype_, true, false);

        pos_influence_ = createFloatParameter("posinf", tr("position influence"),
            tr("A multiplier for influencing the texture offset by the transformation position"),
                                          0.0, 0.01);
    endParameterGroup();

    beginParameterGroup("texturecolor", tr("color"));

        cr_ = createFloatParameter("red", tr("red"), tr("Red amount of color multiplier"), 1.0, 0.1);
        cr_->setMinValue(0.0);
        cg_ = createFloatParameter("green", tr("green"), tr("Green amount of color multiplier"), 1.0, 0.1);
        cg_->setMinValue(0.0);
        cb_ = createFloatParameter("blue", tr("blue"), tr("Blue amount of color multiplier"), 1.0, 0.1);
        cb_->setMinValue(0.0);
        ca_ = createFloatParameter("alpha", tr("alpha"), tr("Alpha amount of color multiplier"), 1.0, 0.1);
        ca_->setMinValue(0.0);

    endParameterGroup();

    beginParameterGroup("texturepost", tr("color post-processing"));

        paramPost_ = createSelectParameter("postenable", tr("post-processing"),
                                           tr("Enables or disables color post-processing"),
                                            { "off", "on" }, { tr("off"), tr("on") },
                                            { tr("Color-post-processing is enabled"),
                                              tr("Color-post-processing is disabled") },
                                            { 0, 1 }, 0, true, false);

        postAlpha_ = createFloatParameter("postalpha", "color to alpha",
                                        tr("Makes the choosen color transparent"),
                                        0.0, 0.0, 1.0, 0.02);
        postAlphaR_ = createFloatParameter("postalphar", "red",
                                        tr("Red component of the color to make transparent"),
                                        0.0, 0.0, 1.0, 0.02);
        postAlphaG_ = createFloatParameter("postalphag", "green",
                                        tr("Green component of the color to make transparent"),
                                        0.0, 0.0, 1.0, 0.02);
        postAlphaB_ = createFloatParameter("postalphab", "blue",
                                        tr("Blue component of the color to make transparent"),
                                        0.0, 0.0, 1.0, 0.02);
        postAlphaEdge_ = createFloatParameter("postalphae", "edge",
                                        tr("Edge of the alpha transformation - the lower the smoother"),
                                        0.0, 0.0, 1.0, 0.02);

        postBright_ = createFloatParameter("postbright", "brightness",
                                        tr("Brightness (simply offset to all colors)"),
                                        0.0, -1.0, 1.0, 0.02);

        postContrast_ = createFloatParameter("postcontr", "contrast",
                                        tr("Contrast (a multiplier around threshold)"),
                                        1.0, 0.0, 100000.0, 0.02);

        postContrastThresh_ = createFloatParameter("postcontrt", "threshold",
                                        tr("The threshold or edge between dark and "
                                           "bright for contrast setting"),
                                        0.5, 0.0, 1.0, 0.02);

        postShift_ = createFloatParameter("postshift", "rgb shift",
                                        tr("Shifts the rgb colors right (postive) or left (negative)"),
                                        0.0, -1.0, 1.0, 0.05);

        postInv_ = createFloatParameter("postinv", "negative",
                                        tr("Mix between normal and negative colors [0,1]"),
                                        0.0, 0.0, 1.0, 0.05);

        postGray_ = createFloatParameter("postgray", "grayscale",
                                        tr("Removes the saturation [0,1]"),
                                        0.0, 0.0, 1.0, 0.05);

    endParameterGroup();
}

void TextureOverlay::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);
    if (p == paramPType_)
    {
        ptype_ = (ProjectionType)paramPType_->baseValue();
        requestReinitGl();
    }
    if (p == paramPost_ || texture_->needsReinit(p))
        requestReinitGl();
}

void TextureOverlay::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

    ptype_ = (ProjectionType)paramPType_->baseValue();
}

void TextureOverlay::updateParameterVisibility()
{
    texture_->updateParameterVisibility();
}

void TextureOverlay::initGl(uint /*thread*/)
{
    // --- texture ---

    /*
    Image img;
    img.loadImage(paramFilename_->value());
    //img.loadImage("/home/defgsus/pic/android/_second/Camera/PANO_20140717_144107.jpg");
    //img.loadImage("/home/defgsus/prog/C/matrixoptimizer/data/graphic/kepler/bg_wood_03_polar.png");
    //img.loadImage(":/texture/mo_black.png");
    tex_ = GL::Texture::createFromImage(img, GL_RGBA);
    */
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
    if (paramPost_->baseValue())
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
        u_cube_hack_ = quad_->shader()->getUniform("u_cube_hack", true);
    }

    if (paramPost_->baseValue())
    {
        u_post_trans_ = quad_->shader()->getUniform("u_post_transform", true);
        u_post_bright_ = quad_->shader()->getUniform("u_post_bright", true);
        u_post_alpha_ = quad_->shader()->getUniform("u_post_alpha", true);
        u_post_alpha_edge_ = quad_->shader()->getUniform("u_post_alpha_edge", true);
    }

    actualPtype_ = ptype_;
}

void TextureOverlay::releaseGl(uint /*thread*/)
{
    quad_->release();
    delete quad_;

    texture_->releaseGl();
    //delete tex_;
}

void TextureOverlay::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation(thread, 0);
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    //const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    const Float posInf = -pos_influence_->value(time, thread);

    u_color_->setFloats(cr_->value(time, thread),
                        cg_->value(time, thread),
                        cb_->value(time, thread),
                        ca_->value(time, thread));

    if (actualPtype_ == PT_EQUIRECT || actualPtype_ == PT_FISHEYE)
    {
        u_cube_hack_->floats[0] = rs.cameraSpace().isCubemap() ? 1.f : 0.f;

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

    if (paramPost_->baseValue())
    {
        u_post_trans_->floats[0] = postGray_->value(time, thread);
        u_post_trans_->floats[1] = postInv_->value(time, thread);
        u_post_trans_->floats[2] = postShift_->value(time, thread);
        u_post_bright_->floats[0] = postBright_->value(time, thread);
        u_post_bright_->floats[1] = postContrast_->value(time, thread);
        u_post_bright_->floats[2] = postContrastThresh_->value(time, thread);
        u_post_alpha_->setFloats(
                    postAlphaR_->value(time, thread),
                    postAlphaG_->value(time, thread),
                    postAlphaB_->value(time, thread),
                    postAlpha_->value(time, thread));
        u_post_alpha_edge_->floats[0] = postAlphaEdge_->value(time, thread);
    }

    //tex_->bind();
    //rs.finalFrameBuffer().colorTexture()->bind();
    texture_->bind();

    MO_CHECK_GL( glDepthMask(false) );
    quad_->draw(rs.cameraSpace().width(), rs.cameraSpace().height());
    MO_CHECK_GL( glDepthMask(true) );

}





} // namespace MO
