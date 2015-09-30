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
#include "geom/geometry.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterimagelist.h"
#include "object/param/parameterint.h"
#include "object/param/parametergeometry.h"
#include "object/util/useruniformsetting.h"
#include "io/filemanager.h"
#include "io/datastream.h"
#include "io/log.h"

#if 1
#   define MO_DEBUG_IG(arg__) MO_PRINT("ImageGallery("<<name()<<":"<<arg__)
#else
#   define MO_DEBUG_IG(unused__) { }
#endif

namespace MO {

MO_REGISTER_OBJECT(ImageGallery)


struct ImageGallery::VAO_
{
    VAO_() : vao(0), tex(0), index(0), indexT(0.f), aspect(1.f) { }
    ~VAO_()
    {
        delete vao;
        delete tex;
    }

    GL::VertexArrayObject * vao;
    GL::Texture * tex;
    int index;
    Float indexT, aspect;
};


ImageGallery::ImageGallery(QObject * parent)
    : ObjectGl      (parent),
      shader_       (0),
      u_light_amt_  (0),
      doRecompile_  (true),
      doCreateVaos_  (true)
{
    setName("ImageGallery");
    initDefaultUpdateMode(UM_ALWAYS, false);
}

ImageGallery::~ImageGallery()
{
    delete shader_;
    for (auto v : vaos_)
        delete v;
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

        keepImageAspect_ = params()->createBooleanParameter(
                    "keep_image_aspect", tr("keep image aspect"),
                    tr("If selected, the image geometry is scaled to the image aspect ratio"),
                    tr("Geometry is unchanged"),
                    tr("Geometry is scale according to the aspect ratio of each image"),
                    true,
                    true, false);

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

        diffAmt_ = params()->createFloatParameter("diffuseamt", tr("diffuse"),
                                   tr("Amount of diffuse lighting"),
                                   .3, 0.1);

        diffExp_ = params()->createFloatParameter("diffuseexp", tr("diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower"),
                                   4.0, 0.1);
        diffExp_->setMinValue(0.001);

        specAmt_ = params()->createFloatParameter("specularamt", tr("specular"),
                                   tr("Amount of specular light reflection"),
                                   .2, 0.1);

        specExp_ = params()->createFloatParameter("specexp", tr("specular exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower"),
                                   10.0, 0.1);
        specExp_->setMinValue(0.001);

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

    if (p == imageList_)
        requestReinitGl();
}

void ImageGallery::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    diffAmt_->setVisible( lightMode_->baseValue() != LM_NONE );
    diffExp_->setVisible( lightMode_->baseValue() != LM_NONE );
    specAmt_->setVisible( lightMode_->baseValue() != LM_NONE );
    specExp_->setVisible( lightMode_->baseValue() != LM_NONE );

}

void ImageGallery::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

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

    // load textures

    int k = 0;
    for (const QString& fn : imageList_->baseValue())
    {
        QString fnl = IO::fileManager().localFilename(fn);

        try
        {
            // get texture
            auto tex = GL::Texture::createFromImage(fnl, gl::GL_RGBA);

            // add an entry in vao entity list
            auto v = new VAO_();
            v->tex = tex;
            v->index = k++;
            v->aspect = Float(v->tex->width()) / v->tex->height();
            vaos_ << v;
        }
        catch (const Exception& e)
        {
            setErrorMessage(e.what());
        }
    }

    // set indexT field
    for (auto v : vaos_)
        v->indexT = Float(v->index) / vaos_.size();

    // need to create new vaos
    doRecompile_ = doCreateVaos_ = true;
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

    // get the input geometries
    auto geoms = paramImageGeom_->getGeometries(0, MO_GFX_THREAD);

    // construct a single Geometry from it
    auto srcGeom = GEOM::Geometry::createFrom(geoms);

    if (!srcGeom->numVertices())
        return;

    // create VAO for each texture
    for (auto v : vaos_)
    {
        GEOM::Geometry * geom;
        if (!keepImageAspect_->baseValue())
        {
            geom = srcGeom;
            geom->addRef();
        }
        else
        {
            // make a copy from original
            geom = new GEOM::Geometry(*srcGeom);

            // apply transformation (aspect)
            geom->scale(1.f, 1.f / std::max(0.001f, v->aspect), 1.f);
        }

        try
        {
            // get the VAO resource
            auto vao = new GL::VertexArrayObject(QString("%1.%2")
                                                 .arg(name()).arg(v->index));
            geom->getVertexArrayObject(vao, shader_);
            // install
            v->vao = vao;
        }
        catch (const Exception& e)
        {
            setErrorMessage(tr("Failed to create VertexArrayObject from geometry\n%1")
                            .arg(e.what()));
        }

        geom->releaseRef();
    }

    srcGeom->releaseRef();

    doCreateVaos_ = false;
}

void ImageGallery::releaseVaos_()
{
    MO_DEBUG_IG("releaseVaos_()");

    // only remove the VAO_::vao field
    for (auto v : vaos_)
    {
        if (v->vao && v->vao->isCreated())
            v->vao->release();

        delete v->vao;
        v->vao = 0;
    }
}

void ImageGallery::releaseAll_()
{
    MO_DEBUG_IG("releaseAll_()");

    for (auto v : vaos_)
    {
        if (v->vao && v->vao->isCreated())
            v->vao->release();
        delete v->vao;
        if (v->tex && v->tex->isAllocated())
            v->tex->release();
        delete v->tex;
    }
    vaos_.clear();
}

void ImageGallery::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    if (paramImageGeom_->hasChanged(time, thread))
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

    if (!shader_)
        return;

    // ---- update shader uniforms -----

    if (uniformColor_)
    {
        const auto bright = mbright_->value(time, thread);
        uniformColor_->setFloats(
                mr_->value(time, thread) * bright,
                mg_->value(time, thread) * bright,
                mb_->value(time, thread) * bright,
                ma_->value(time, thread));
    }
    if (u_light_amt_)
        u_light_amt_->setFloats(
                    diffAmt_->value(time, thread),
                    diffExp_->value(time, thread),
                    specAmt_->value(time, thread),
                    specExp_->value(time, thread));

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

    for (auto v : vaos_)
    if (v->vao && v->vao->isCreated())
    {
        // get individual transformation
        Mat4 trans = transformation();

        trans = MATH::rotate(trans, v->indexT * 360.f, Vec3(0,1,0));
        trans = glm::translate(trans, Vec3(0,0,-5));

        const Mat4
                cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans,
                viewTrans = rs.cameraSpace().viewMatrix() * trans;

        if (uniformCVT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformCVT_->location(), 1, GL_FALSE, &cubeViewTrans[0][0]) );
        if (uniformVT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformVT_->location(), 1, GL_FALSE, &viewTrans[0][0]) );
        if (uniformT_)
            MO_CHECK_GL( glUniformMatrix4fv(uniformT_->location(), 1, GL_FALSE, &trans[0][0]) );

        // bind the texture
        v->tex->bind();

        // render
        v->vao->drawElements();
    }

    shader_->deactivate();
}





} // namespace MO
