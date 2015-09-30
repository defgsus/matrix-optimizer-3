/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#include "imagegallery.h"
#include "object/scene.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "gl/vertexarrayobject.h"
#include "gl/compatibility.h"
#include "gl/texture.h"
#include "math/vector.h"
#include "math/constants.h"
#include "geom/geometry.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterimagelist.h"
#include "object/param/parameterint.h"
#include "object/param/parametergeometry.h"
#include "object/util/texturesetting.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

#if 0
#   define MO_DEBUG_IG(arg__) MO_PRINT("ImageGallery("<<name()<<":"<<arg__)
#else
#   define MO_DEBUG_IG(unused__) { }
#endif

namespace MO {

MO_REGISTER_OBJECT(ImageGallery)


struct ImageGallery::Entity_
{
    Entity_() : tex(0), index(0), indexT(0.f), aspect(1.f) { }
    ~Entity_()
    {
        delete tex;
    }

    GL::Texture * tex;
    int index;
    Float indexT, aspect;
    Mat4 transformFrame,
         transformImage,
         transformCurrent;
};


ImageGallery::ImageGallery(QObject * parent)
    : ObjectGl      (parent),
      shader_       (0),
      vaoImage_     (0),
      vaoFrame_     (0),
      frameTexSet_  (new TextureSetting(this)),
      u_light_amt_  (0),
      doRecompile_  (true),
      doCalcBaseTransform_(true),
      doCreateVaos_ (true)
{
    setName("ImageGallery");
    initDefaultUpdateMode(UM_ALWAYS, false);
}

ImageGallery::~ImageGallery()
{
    delete shader_;
    delete vaoImage_;
    delete vaoFrame_;
    for (auto v : entities_)
        delete v;
    //delete frameTexSet_;
}

void ImageGallery::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("imageg", 1);
}

void ImageGallery::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    /*const int ver = */io.readHeader("imageg", 1);
}

void ImageGallery::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("content", tr("content"));
    initParameterGroupExpanded("content");

        imageList_ = params()->createImageListParameter("image_list", tr("image list"),
                                         tr("The list of images to display"),
                                         QStringList() << ":/texture/mo_black.png");

        paramImageGeom_ = params()->createGeometryParameter(
                    "geometry_image", tr("image geometry"),
                    tr("The geometry that is used to display each image"));
        paramImageGeom_->setVisibleGraph(true);

        paramFrameGeom_ = params()->createGeometryParameter(
                    "geometry_frame", tr("frame geometry"),
                    tr("A geometry that is used to display a frame for each image"));
        paramFrameGeom_->setVisibleGraph(true);

        keepImageAspect_ = params()->createBooleanParameter(
                    "keep_image_aspect", tr("keep image aspect"),
                    tr("If selected, the image geometry is scaled to the image aspect ratio"),
                    tr("Geometry is unchanged"),
                    tr("Geometry is scale according to the aspect ratio of each image"),
                    true,
                    true, false);

        keepFrameAspect_ = params()->createBooleanParameter(
                    "keep_frame_aspect", tr("keep frame aspect"),
                    tr("If selected, the frame geometry is scaled to the image aspect ratio"),
                    tr("Geometry is unchanged"),
                    tr("Geometry is scale according to the aspect ratio of each image"),
                    true,
                    true, false);

    params()->endParameterGroup();


    params()->beginParameterGroup("arrangement", "arrangement");

        arrangement_ = params()->createSelectParameter(
                    "arrangement", tr("arrangement"),
                    tr("Selects how the images will be arranged in space"),
        { "row", "col", "circ", "cyl"},
        { tr("row"), tr("column"), tr("circle"), tr("cylinder") },
        { tr("All images are arranged besides one another"),
          tr("All images are arranged above one another"),
          tr("All images are arranged in a circle"),
          tr("All images are arranged like pictures on a cylindrical wall") },
        { A_ROW, A_COLUMN, A_CIRCLE, A_CYLINDER },
                    A_ROW,
                    true, false);

        spacing_ = params()->createFloatParameter(
                    "spacing", tr("spacing"), tr("The space between each image"),
                    1.f, 0.1f);

        radius_ = params()->createFloatParameter(
                    "radius", tr("radius"), tr("The radius of the whole arrangement"),
                    10.f, 0.1f);

    params()->endParameterGroup();


    params()->beginParameterGroup("light", "lighting");

        lightMode_ = params()->createSelectParameter("lightmode", tr("lighting mode"),
            tr("Selects the way how the lighting is calculated"),
            { "none", "vertex", "fragment" },
            { tr("off"), tr("per vertex"), tr("per fragment") },
            { tr("Light-calculation is completely disabled."),
              tr("The light influence is calculated per vertex. This might lead to incorrect "
                 "results and artifacts."),
              tr("The light influence is calculated per pixel. This is most accurate but a "
                 "bit more computationally expensive.") },
            { LM_NONE, LM_PER_VERTEX, LM_PER_FRAGMENT },
            LM_PER_FRAGMENT,
            true, false);

        diffAmt_ = params()->createFloatParameter("diffuseamt", tr("image diffuse"),
                                   tr("Amount of diffuse lighting for the image"),
                                   .1, 0.1);

        diffExp_ = params()->createFloatParameter("diffuseexp", tr("image diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower"),
                                   2.0, 0.1);
        diffExp_->setMinValue(0.001);

        specAmt_ = params()->createFloatParameter("specularamt", tr("image specular"),
                                   tr("Amount of specular light reflection for the image"),
                                   .05, 0.05);

        specExp_ = params()->createFloatParameter("specexp", tr("image specular exponent"),
                                   tr("Exponent for the specular lighting - the higher, the narrower"),
                                   10.0, 0.1);
        specExp_->setMinValue(0.001);

        diffAmtF_ = params()->createFloatParameter("diffuseamt_f", tr("frame diffuse"),
                                   tr("Amount of diffuse lighting for the frame"),
                                   .3, 0.1);

        diffExpF_ = params()->createFloatParameter("diffuseexp_f", tr("frame diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower"),
                                   4.0, 0.1);
        diffExpF_->setMinValue(0.001);

        specAmtF_ = params()->createFloatParameter("specularamt_f", tr("frame specular"),
                                   tr("Amount of specular light reflection for the frame"),
                                   .2, 0.1);

        specExpF_ = params()->createFloatParameter("specexp_f", tr("frame specular exponent"),
                                   tr("Exponent for the specular lighting - the higher, the narrower"),
                                   10.0, 0.1);
        specExpF_->setMinValue(0.001);

    params()->endParameterGroup();


    params()->beginParameterGroup("frametex", "frame texture");

        frameTexSet_->setNoneTextureProxy(true);
        frameTexSet_->createParameters("_frametex", TextureSetting::TEX_NONE, true);

    params()->endParameterGroup();


    params()->beginParameterGroup("fcolor", tr("frame color"));

        fbright_ = params()->createFloatParameter("fbright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        fr_ = params()->createFloatParameter("fred", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        fg_ = params()->createFloatParameter("fgreen", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        fb_ = params()->createFloatParameter("fblue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        fa_ = params()->createFloatParameter("falpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("icolor", tr("image color"));

        mbright_ = params()->createFloatParameter("ibright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        mr_ = params()->createFloatParameter("ired", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        mg_ = params()->createFloatParameter("igreen", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        mb_ = params()->createFloatParameter("iblue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ma_ = params()->createFloatParameter("ialpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

}

void ImageGallery::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();

}

void ImageGallery::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == lightMode_)
    {
        doRecompile_ = true;
        requestRender();
    }

    if (p == imageList_
      || frameTexSet_->needsReinit(p))
        requestReinitGl();

    if (p == keepFrameAspect_
        || p == keepImageAspect_)
        doCalcBaseTransform_ = true;
}

void ImageGallery::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    bool isLight = lightMode_->baseValue() != LM_NONE;
    diffAmt_->setVisible( isLight );
    diffExp_->setVisible( isLight );
    specAmt_->setVisible( isLight );
    specExp_->setVisible( isLight );
    diffAmtF_->setVisible( isLight );
    diffExpF_->setVisible( isLight );
    specAmtF_->setVisible( isLight );
    specExpF_->setVisible( isLight );

    auto mode = Arrangement(arrangement_->baseValue());
    spacing_->setVisible( mode == A_COLUMN
                          || mode == A_ROW);
    radius_->setVisible( mode == A_CIRCLE
                         || mode == A_CYLINDER);

    frameTexSet_->updateParameterVisibility();
}

void ImageGallery::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    frameTexSet_->getNeededFiles(files, IO::FT_TEXTURE);

    for (const QString& fn : imageList_->baseValue())
        files << IO::FileListEntry(fn, IO::FT_TEXTURE);
}

Vec4 ImageGallery::imageColor(Double time, uint thread) const
{
    const auto b = mbright_->value(time, thread);
    return Vec4(
        mr_->value(time, thread) * b,
        mg_->value(time, thread) * b,
        mb_->value(time, thread) * b,
        ma_->value(time, thread));
}

Vec4 ImageGallery::frameColor(Double time, uint thread) const
{
    const auto b = fbright_->value(time, thread);
    return Vec4(
        fr_->value(time, thread) * b,
        fg_->value(time, thread) * b,
        fb_->value(time, thread) * b,
        fa_->value(time, thread));
}


void ImageGallery::initGl(uint /*thread*/)
{
    MO_DEBUG_IG("initGl()");

    releaseAll_();

    // get frame texture

    try
    {
        frameTexSet_->initGl();
    }
    catch (const Exception& e)
    {
        setErrorMessage(e.what());
    }

    // load image textures

    int k = 0;
    for (const QString& fn : imageList_->baseValue())
    {
        QString fnl = IO::fileManager().localFilename(fn);

        try
        {
            // get texture
            auto tex = GL::Texture::createFromImage(fnl, gl::GL_RGBA);

            // add an entry in vao entity list
            auto v = new Entity_();
            v->tex = tex;
            v->index = k++;
            v->aspect = std::max(0.0001f, float(v->tex->width()) / v->tex->height());
            entities_ << v;
        }
        catch (const Exception& e)
        {
            setErrorMessage(e.what());
        }
    }

    // set indexT field [0,1)
    for (auto v : entities_)
        v->indexT = Float(v->index) / entities_.size();

    doCalcBaseTransform_ = doRecompile_ = doCreateVaos_ = true;
}

void ImageGallery::releaseGl(uint /*thread*/)
{
    MO_DEBUG_IG("releaseGl()");

    releaseAll_();

    if (shader_)
    {
        if (shader_->isReady())
            shader_->releaseGL();
        delete shader_;
        shader_ = 0;
    }
}

void ImageGallery::numberLightSourcesChanged(uint /*thread*/)
{
    doRecompile_ = true;
}

GL::ShaderSource ImageGallery::shaderSource() const
{
    if (!shader_)
        return GL::ShaderSource();
    return *shader_->source();
}

void ImageGallery::calcBaseTransform_()
{
    for (auto v : entities_)
    {
        if (keepFrameAspect_->baseValue())
            v->transformFrame = glm::scale(Mat4(1.), Vec3(1.f, 1.f / v->aspect, 1.f));
        else
            v->transformFrame = Mat4(1.);

        if (keepImageAspect_->baseValue())
            v->transformImage = glm::scale(Mat4(1.), Vec3(1.f, 1.f / v->aspect, 1.f));
        else
            v->transformImage = Mat4(1.);
    }

    doCalcBaseTransform_ = false;
}

void ImageGallery::setupShader_()
{
    MO_DEBUG_IG("setupShader_()");

    clearError();

    // -------- construct the GLSL source ----------

    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();

    if (numberLightSources() > 0 && lightMode_->baseValue() != LM_NONE)
        src->addDefine("#define MO_ENABLE_LIGHTING");
    src->addDefine(QString("#define MO_NUM_LIGHTS %1")
                   .arg(numberLightSources()));
    if (lightMode_->baseValue() == LM_PER_FRAGMENT)
        src->addDefine("#define MO_FRAGMENT_LIGHTING");
    src->addDefine("#define MO_ENABLE_TEXTURE");

    // resolve includes
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        return getGlslInclude(url, do_search);
    });

    if (!shader_)
        shader_ = new GL::Shader(name());
    shader_->setSource(src);

    try
    {
        shader_->compile();
        doRecompile_ = false;
        doCreateVaos_ = true;
    }
    catch (const Exception& e)
    {
        MO_WARNING("Error on initializing model for '" << name() << "'\n" << e.what());
        setErrorMessage(tr("Failed to initialized model (%1)").arg(e.what()));
        // XXX Should deinitialize or otherwise flag the object
        return;
    }

    // get uniforms

    u_light_amt_ = shader_->getUniform(src->uniformNameLightAmt());
    u_cam_pos_ = shader_->getUniform("u_cam_pos");
    u_tex_ = shader_->getUniform("tex_0");
    if (u_tex_)
        u_tex_->ints[0] = 0;

    uniformProj_ = shader_->getUniform(src->uniformNameProjection());
    uniformCVT_ = shader_->getUniform(src->uniformNameCubeViewTransformation());
    uniformVT_ = shader_->getUniform(src->uniformNameViewTransformation());
    uniformT_ = shader_->getUniform(src->uniformNameTransformation());
    uniformLightPos_ = shader_->getUniform(src->uniformNameLightPos());
    uniformLightColor_ = shader_->getUniform(src->uniformNameLightColor());
    uniformLightDiffuseExp_ = shader_->getUniform(src->uniformNameLightDiffuseExponent());
    uniformLightDirection_ = shader_->getUniform(src->uniformNameLightDirection());
    uniformLightDirectionParam_ = shader_->getUniform(src->uniformNameLightDirectionParam());
    uniformSceneTime_ = shader_->getUniform(src->uniformNameSceneTime());
    uniformColor_ = shader_->getUniform(src->uniformNameColor());

}

void ImageGallery::setupVaos_()
{
    releaseVaos_();
    MO_DEBUG_IG("setupVaos_()");

    if (!shader_ || !shader_->isReady())
        return;

    // image geom
    {
        // get the input geometries
        auto geoms = paramImageGeom_->getGeometries(0, MO_GFX_THREAD);

        // construct a single Geometry from it
        auto geom = GEOM::Geometry::createFrom(geoms);

        geom->getExtent(&extentMin_, &extentMax_);

        if (geom->numVertices())
        {            
            // create VAO
            try
            {
                // get the VAO resource
                vaoImage_ = new GL::VertexArrayObject(QString("%1_image")
                                                     .arg(name()));
                geom->getVertexArrayObject(vaoImage_, shader_);
            }
            catch (const Exception& e)
            {
                setErrorMessage(tr("Failed to create VertexArrayObject from image geometry\n%1")
                                .arg(e.what()));
                releaseVaos_();
            }
        }

        geom->releaseRef();
    }

    // frame geom
    {
        // get the input geometries
        auto geoms = paramFrameGeom_->getGeometries(0, MO_GFX_THREAD);

        // construct a single Geometry from it
        auto geom = GEOM::Geometry::createFrom(geoms);

        if (geom->numVertices())
        {
            Vec3 minE, maxE;
            geom->getExtent(&minE, &maxE);
            extentMin_.x = std::min(extentMin_.x, minE.x);
            extentMin_.y = std::min(extentMin_.y, minE.y);
            extentMin_.z = std::min(extentMin_.z, minE.z);
            extentMax_.x = std::max(extentMax_.x, maxE.x);
            extentMax_.y = std::max(extentMax_.y, maxE.y);
            extentMax_.z = std::max(extentMax_.z, maxE.z);

            // create VAO
            try
            {
                // get the VAO resource
                vaoFrame_ = new GL::VertexArrayObject(QString("%1_image")
                                                     .arg(name()));
                geom->getVertexArrayObject(vaoFrame_, shader_);
            }
            catch (const Exception& e)
            {
                setErrorMessage(tr("Failed to create VertexArrayObject from frame geometry\n%1")
                                .arg(e.what()));
                if (vaoFrame_->isCreated())
                    vaoFrame_->release();
                delete vaoFrame_;
                vaoFrame_ = 0;
            }
        }

        geom->releaseRef();
    }

    extent_ = extentMax_ - extentMin_;

    doCreateVaos_ = false;
}

void ImageGallery::releaseVaos_()
{
    MO_DEBUG_IG("releaseVaos_()");

    if (vaoImage_ && vaoImage_->isCreated())
        vaoImage_->release();

    delete vaoImage_;
    vaoImage_ = 0;

    if (vaoFrame_ && vaoFrame_->isCreated())
        vaoFrame_->release();

    delete vaoFrame_;
    vaoFrame_ = 0;
}

void ImageGallery::releaseAll_()
{
    MO_DEBUG_IG("releaseAll_()");

    for (auto v : entities_)
    {
        if (v->tex && v->tex->isAllocated())
            v->tex->release();
        delete v->tex;
    }
    entities_.clear();

    frameTexSet_->releaseGl();

    if (vaoImage_ && vaoImage_->isCreated())
        vaoImage_->release();

    delete vaoImage_;
    vaoImage_ = 0;

    if (vaoFrame_ && vaoFrame_->isCreated())
        vaoFrame_->release();

    delete vaoFrame_;
    vaoFrame_ = 0;
}

void ImageGallery::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    // -- lazy initializer --

    if (paramImageGeom_->hasChanged(time, thread)
      || paramFrameGeom_->hasChanged(time, thread))
    {
        doCreateVaos_ = true;
    }

    if (doRecompile_)
    {
        setupShader_();
    }

    if (doCreateVaos_)
    {
        setupVaos_();
    }

    if (doCalcBaseTransform_)
    {
        calcBaseTransform_();
    }

    if (!shader_ || !vaoImage_ || !vaoImage_->isCreated())
        return;

    // --- get common parameter values ---

    Vec4 frameCol, imageCol,
         frameLightAmt, imageLightAmt;

    if (uniformColor_)
    {
        frameCol = frameColor(time, thread);
        imageCol = imageColor(time, thread);
    }

    if (u_light_amt_)
    {
        imageLightAmt = Vec4(
                    diffAmt_->value(time, thread),
                    diffExp_->value(time, thread),
                    specAmt_->value(time, thread),
                    specExp_->value(time, thread));
        frameLightAmt = Vec4(
                    diffAmtF_->value(time, thread),
                    diffExpF_->value(time, thread),
                    specAmtF_->value(time, thread),
                    specExpF_->value(time, thread));
    }

    // ---- update shader uniforms -----

    if (u_cam_pos_)
    {
        const Vec3& pos = rs.cameraSpace().position();
        u_cam_pos_->setFloats(pos.x, pos.y, pos.z, 0.);
    }

    // texture slot
    if (u_tex_)
        u_tex_->ints[0] = 0;

    shader_->activate();
    shader_->sendUniforms();

    using namespace gl;

    if (uniformProj_)
        MO_CHECK_GL( glUniformMatrix4fv(uniformProj_->location(), 1, GL_FALSE,
                                        &rs.cameraSpace().projectionMatrix()[0][0]) );

    if (uniformSceneTime_ && time >= 0.)
        MO_CHECK_GL( glUniform1f(uniformSceneTime_->location(), time) );

    if (rs.lightSettings().count())
    {
        if (uniformLightPos_)
            MO_CHECK_GL( glUniform3fv(uniformLightPos_->location(),
                                      rs.lightSettings().count(), rs.lightSettings().positions()) );
        if (uniformLightColor_)
            MO_CHECK_GL( glUniform4fv(uniformLightColor_->location(),
                                      rs.lightSettings().count(), rs.lightSettings().colors()) );
        if (uniformLightDirection_)
            MO_CHECK_GL( glUniform3fv(uniformLightDirection_->location(),
                                      rs.lightSettings().count(), rs.lightSettings().directions()) );
        if (uniformLightDirectionParam_)
            MO_CHECK_GL( glUniform3fv(uniformLightDirectionParam_->location(),
                                      rs.lightSettings().count(), rs.lightSettings().directionParam()) );
        if (uniformLightDiffuseExp_)
            MO_CHECK_GL( glUniform1fv(uniformLightDiffuseExp_->location(),
                                      rs.lightSettings().count(), rs.lightSettings().diffuseExponents()) );
    }

    // --- calc individual transformation ---

    const Float
            spacing = spacing_->value(time, thread),
            radius = radius_->value(time, thread);

    switch (Arrangement(arrangement_->baseValue()))
    {
        case A_ROW:
        {
            Float x = 0.;
            for (auto v : entities_)
            {
                Mat4 trans = glm::translate(Mat4(1.), Vec3(x, 0, 0));
                x += extent_.x + spacing;

                v->transformCurrent = trans;
            }
        }
        break;

        case A_COLUMN:
        {
            Float y = 0.;
            for (auto v : entities_)
            {
                Mat4 trans = glm::translate(Mat4(1.), Vec3(0, y, 0));
                y += extent_.y + spacing;

                v->transformCurrent = trans;
            }
        }
        break;

        case A_CIRCLE:
        {
            for (auto v : entities_)
            {
                Float t = v->indexT * TWO_PI;
                Mat4 trans = glm::translate(Mat4(1.),
                            Vec3(radius * std::sin(t), radius * std::cos(t), 0));

                v->transformCurrent = trans;
            }
        }
        break;

        case A_CYLINDER:
            for (auto v : entities_)
            {
                Mat4 trans = MATH::rotate(Mat4(1.), v->indexT * 360.f, Vec3(0,1,0));
                trans = glm::translate(trans, Vec3(0,0,-radius));

                v->transformCurrent = trans;
            }
        break;
    }


    // render each entity
    for (auto v : entities_)
    {
        // get base transformation

        const Mat4
                trans = transformation() * v->transformCurrent,
                cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans,
                viewTrans = rs.cameraSpace().viewMatrix() * trans;

        // -- render frame --

        if (vaoFrame_ && vaoFrame_->isCreated())
        {
            if (uniformColor_)
            {
                uniformColor_->set(frameCol);
                uniformColor_->send();
            }

            if (u_light_amt_)
            {
                u_light_amt_->set(frameLightAmt);
                u_light_amt_->send();
            }

            if (uniformCVT_)
                MO_CHECK_GL( glUniformMatrix4fv(uniformCVT_->location(), 1, GL_FALSE,
                                                &(cubeViewTrans * v->transformFrame)[0][0]) );
            if (uniformVT_)
                MO_CHECK_GL( glUniformMatrix4fv(uniformVT_->location(), 1, GL_FALSE,
                                                &(viewTrans * v->transformFrame)[0][0]) );
            if (uniformT_)
                MO_CHECK_GL( glUniformMatrix4fv(uniformT_->location(), 1, GL_FALSE,
                                                &(trans * v->transformFrame)[0][0]) );
            // bind frame texture
            // (note: 'None' type binds a white picture)
            frameTexSet_->bind(time, thread);

            // render frame
            vaoFrame_->drawElements();
        }


        // -- render image --

        if (uniformColor_)
        {
            uniformColor_->set(imageCol);
            uniformColor_->send();
        }

        if (u_light_amt_)
        {
            u_light_amt_->set(imageLightAmt);
            u_light_amt_->send();
        }

        if (uniformCVT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformCVT_->location(), 1, GL_FALSE,
                                            &(cubeViewTrans * v->transformImage)[0][0]) );
        if (uniformVT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformVT_->location(), 1, GL_FALSE,
                                            &(viewTrans * v->transformImage)[0][0]) );
        if (uniformT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformT_->location(), 1, GL_FALSE,
                                            &(trans * v->transformImage)[0][0]) );

        // bind the image texture
        v->tex->bind();

        // render image
        vaoImage_->drawElements();

    }

    shader_->deactivate();
}





} // namespace MO
