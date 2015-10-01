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
#include "math/random.h"
#include "math/functions.h"
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
    Float indexT, aspect, heightM;
    Mat4 transformFrame,
         transformImage,
         transformCurrent,
         transformRnd;
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


    params()->beginParameterGroup("arrangement", tr("arrangement"));

        arrangement_ = params()->createSelectParameter(
                    "arrangement", tr("arrangement"),
                    tr("Selects how the images will be arranged in space"),
        { "row", "col", "circ", "clock", "cyl", "cylv", "rndp", "rnd3" },
        { tr("row"), tr("column"), tr("circle"), tr("clock"),
          tr("cylinder horizontal"), tr("cylinder vertical"),
          tr("random 2d"), tr("random 3d") },
        { tr("The images are arranged besides one another"),
          tr("The images are arranged above one another"),
          tr("The images are arranged in a circle"),
          tr("The images are arranged in a circle, each image's up-axis is pointing to the periphery"),
          tr("The images are arranged on a cylindrical wall"),
          tr("The images are arranged on a cylindrical wheel"),
          tr("The images are arranged randomly on the xy-plane"),
          tr("The images are arranged randomly in space"),},
        { A_ROW, A_COLUMN, A_CIRCLE, A_CLOCK, A_CYLINDER_H, A_CYLINDER_V,
          A_RANDOM_PLANE, A_RANDOM_3D },
                    A_ROW,
                    true, false);

        alignH_ = params()->createSelectParameter("alignment_h", tr("alignment"),
                tr("Selects the alignment for the images"),
                { "l", "r", "c" },
                { tr("left"), tr("right"), tr("center") },
                { tr("left"), tr("right"), tr("center") },
                { Qt::AlignLeft, Qt::AlignRight, Qt::AlignCenter },
                Qt::AlignLeft, true, false);

        alignV_ = params()->createSelectParameter("alignment_v", tr("alignment"),
                tr("Selects the alignment for the images"),
                { "t", "b", "c" },
                { tr("top"), tr("bottom"), tr("center") },
                { tr("top"), tr("bottom"), tr("center") },
                { Qt::AlignTop, Qt::AlignBottom, Qt::AlignCenter },
                Qt::AlignBottom, true, false);

        randomSeed_ = params()->createIntParameter(
                    "rseed", tr("random seed"), tr("The start seed defining the random pattern"),
                    0, true, false);

        spacing_ = params()->createFloatParameter(
                    "spacing", tr("spacing"), tr("The space between each image"),
                    1.f, 0.1f);

        radius_ = params()->createFloatParameter(
                    "radius", tr("radius"), tr("The radius of the whole arrangement"),
                    10.f, 0.1f);

        radiusX_ = params()->createFloatParameter(
                    "radius_x", tr("radius x"), tr("The extent on the x axis"),
                    10.f, 0.1f);

        radiusY_ = params()->createFloatParameter(
                    "radius_y", tr("radius y"), tr("The extent on the y axis"),
                    10.f, 0.1f);

        radiusZ_ = params()->createFloatParameter(
                    "radius_z", tr("radius z"), tr("The extent on the z axis"),
                    10.f, 0.1f);

    params()->endParameterGroup();


    params()->beginParameterGroup("orient", tr("orientation"));

        scale_ = params()->createFloatParameter(
                    "scale", tr("scale"), tr("Scale multiplier for each image and frame"),
                    1.f, 0.1f);

        rotation_ = params()->createFloatParameter(
                    "rot_angle", tr("rotation angle"),
                    tr("Rotation angle in degree"),
                    0.f, 1.f);

        const QString axisTip = tr("Unit vector describing the axis to rotate around (internally normalized)");
        rotX_ = params()->createFloatParameter("rot_x", tr("rotation axis x"), axisTip, 0);
        rotY_ = params()->createFloatParameter("rot_y", tr("rotation axis y"), axisTip, 0);
        rotZ_ = params()->createFloatParameter("rot_z", tr("rotation axis z"), axisTip, 1);

    params()->endParameterGroup();


    params()->beginParameterGroup("pick", tr("image picking"));

        doPickPos_ = params()->createBooleanParameter(
                    "do_pick_pos", tr("pick position"),
                    tr("When enabled, the position of the picked image is changed"),
                    tr("Off"), tr("On"), false, true, false);

        doPickScale_ = params()->createBooleanParameter(
                    "do_pick_scale", tr("pick scale"),
                    tr("When enabled, the scale of the picked image is changed"),
                    tr("Off"), tr("On"), false, true, false);

        doPickRot_ = params()->createBooleanParameter(
                    "do_pick_rot", tr("pick rotation"),
                    tr("When enabled, the rotation of the picked image is changed"),
                    tr("Off"), tr("On"), false, true, false);

        pickIndex_ = params()->createFloatParameter(
                    "pick_index", tr("pick index"), tr("Selects the picture which should be picked "
                                                       "(starting at 0)"), 0.f);
        pickMix_ = params()->createFloatParameter(
                    "pick_mix", tr("pick"), tr("The mix value for the transformation, "
                                               "0.0 = unpicked, 1.0 = picked"), 1.f, 0.f, 1.f, 0.01f);

        pickPosX_ = params()->createFloatParameter(
                    "pick_pos_x", tr("position x"), tr("X component of the absolute position vector"),
                    0.f, 0.1f);
        pickPosY_ = params()->createFloatParameter(
                    "pick_pos_y", tr("position y"), tr("Y component of the absolute position vector"),
                    0.f, 0.1f);
        pickPosZ_ = params()->createFloatParameter(
                    "pick_pos_z", tr("position z"), tr("Z component of the absolute position vector"),
                    0.f, 0.1f);

        pickScale_ = params()->createFloatParameter(
                    "pick_scale", tr("scale"), tr("Scale multiplier for the picked image"),
                    1.f, 0.1f);

        pickRot_ = params()->createFloatParameter(
                    "pick_rot_angle", tr("rotation angle"),
                    tr("Rotation angle in degree"),
                    0.f, 1.f);

        pickRotX_ = params()->createFloatParameter("pick_rot_x", tr("rotation axis x"), axisTip, 0);
        pickRotY_ = params()->createFloatParameter("pick_rot_y", tr("rotation axis y"), axisTip, 0);
        pickRotZ_ = params()->createFloatParameter("pick_rot_z", tr("rotation axis z"), axisTip, 1);

    params()->endParameterGroup();



    params()->beginParameterGroup("light", tr("lighting"));

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


    params()->beginParameterGroup("frametex", tr("frame texture"));

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

    if (   p == keepFrameAspect_
        || p == keepImageAspect_
        || p == arrangement_
        || p == randomSeed_)
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
                          || mode == A_ROW );
    radius_->setVisible( mode == A_CIRCLE
                         || mode == A_CLOCK
                         || mode == A_CYLINDER_V
                         || mode == A_CYLINDER_H
                         || mode == A_RANDOM_PLANE );
    radiusX_->setVisible(
        radiusY_->setVisible(
            radiusZ_->setVisible(
                mode == A_RANDOM_3D )));
    alignH_->setVisible( mode == A_ROW );
    alignV_->setVisible( mode == A_COLUMN );
    randomSeed_->setVisible( mode == A_RANDOM_PLANE );

    bool picking = doPickPos_->baseValue()
            || doPickRot_->baseValue()
            || doPickScale_->baseValue();
    pickIndex_->setVisible(picking);
    pickMix_->setVisible(picking);
    pickPosX_->setVisible( doPickPos_->baseValue() );
    pickPosY_->setVisible( doPickPos_->baseValue() );
    pickPosZ_->setVisible( doPickPos_->baseValue() );
    pickScale_->setVisible( doPickScale_->baseValue() );
    pickRot_->setVisible( doPickRot_->baseValue() );
    pickRotX_->setVisible( doPickRot_->baseValue() );
    pickRotY_->setVisible( doPickRot_->baseValue() );
    pickRotZ_->setVisible( doPickRot_->baseValue() );

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
        calcEntityBaseTransform_();
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

    calcEntityTransform_(time, thread);

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


void ImageGallery::calcEntityBaseTransform_()
{
    MATH::Random<> rnd(randomSeed_->baseValue());

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

        if (arrangement_->baseValue() == A_RANDOM_PLANE)
        {
            v->transformRnd = glm::translate(Mat4(1.),
                Vec3(rnd.rand(-1.f, 1.f), rnd.rand(-1.f, 1.f), 0.f));
        }

        v->heightM = keepFrameAspect_->baseValue()
                    && keepImageAspect_->baseValue()
                ? (1.f / v->aspect) : 1.f;
    }

    doCalcBaseTransform_ = false;
}

namespace {
    Float pick_mix(Float index1, Float index2)
    {
        Float x = std::max(0.f, 1.f - std::abs(Float(index1 - index2)));
        return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
    }
    /** removes the angle with rising @p mix [0,1] */
    Float remove_angle(Float a, Float mix)
    {
        Float ang = MATH::moduloSigned(a, 360.f);
        return ang < 180.f
                ? (ang - mix * ang) : (ang + mix * (360.f - ang));
    }
    Float mix_angle(Float a1, Float a2, Float mix)
    {
        Float ang1 = MATH::moduloSigned(a1, 360.f),
              ang2 = MATH::moduloSigned(a2, 360.f);
        if ((ang2 - ang1) > 180.f)
            ang2 -= 360.f;
        else if (ang1 - ang2 > 180.f)
            ang1 -= 360.f;
        return ang1 + mix * (ang2 - ang1);
    }
}

void ImageGallery::calcEntityTransform_(Double time, uint thread)
{
    if (entities_.empty())
        return;

    // get some parameter values

    const Float
            spacing = spacing_->value(time, thread),
            radius = radius_->value(time, thread);
    const int
            alignH = alignH_->baseValue(),
            alignV = alignV_->baseValue();
    const bool
            doPickPos = doPickPos_->baseValue(),
            doPickRot = doPickRot_->baseValue(),
            doPickScale = doPickScale_->baseValue();

    Float pickIndex=0.f, pickMix=0.f, pickRot=0.f;
    Vec3 pickPos, pickScale, pickRotAxis;
    if (doPickPos)
        pickPos = Vec3(pickPosX_->value(time, thread),
                       pickPosY_->value(time, thread),
                       pickPosZ_->value(time, thread));
    if (doPickScale)
        pickScale = Vec3(pickScale_->value(time, thread));
    if (doPickRot)
    {
        pickRot = pickRot_->value(time, thread);
        pickRotAxis = Vec3(pickRotX_->value(time, thread),
                           pickRotY_->value(time, thread),
                           pickRotZ_->value(time, thread));
    }
    if (doPickPos || doPickScale || doPickRot)
    {
        pickIndex = pickIndex_->value(time, thread),
        pickMix = pickMix_->value(time, thread);
    }

    switch (Arrangement(arrangement_->baseValue()))
    {
        case A_ROW:
        {
            Float x = 0.;
            if (alignH == Qt::AlignCenter)
                x = -.5f * entities_.size() * (extent_.x + spacing);
            else if (alignH == Qt::AlignRight)
                x = -1.f * entities_.size() * (extent_.x + spacing);
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
            if (alignV != Qt::AlignTop)
            {
                // find actual height first
                Float height = 0.f;
                for (auto v : entities_)
                    height += extent_.y * v->heightM + spacing;
                if (alignV == Qt::AlignBottom)
                    y = height;
                else if (alignV == Qt::AlignCenter)
                    y = .5f * height;
            }
            for (auto v : entities_)
            {
                const Float ys = extent_.y * v->heightM * .5;
                if (v != entities_.first())
                    y -= ys;
                Mat4 trans = glm::translate(Mat4(1.), Vec3(0, y, 0));
                y -= ys + spacing;

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

        case A_CLOCK:
        {
            for (auto v : entities_)
            {
                Float t = v->indexT * TWO_PI;
                Mat4 trans = glm::translate(Mat4(1.),
                            Vec3(radius * std::sin(t), radius * std::cos(t), 0));

                Float   mix = pick_mix(pickIndex, v->index),
                        ang = remove_angle(v->indexT * 360.f, mix);
                trans = MATH::rotate(trans, ang, Vec3(0,0,-1));

                v->transformCurrent = trans;
            }
        }
        break;

        case A_CYLINDER_H:
            for (auto v : entities_)
            {
                Float   mix = pick_mix(pickIndex, v->index),
                        ang = remove_angle(v->indexT * 360.f, mix);
                Mat4 trans = MATH::rotate(Mat4(1.), ang, Vec3(0,1,0));
                trans = glm::translate(trans, Vec3(0,0,-radius));

                v->transformCurrent = trans;
            }
        break;

        case A_CYLINDER_V:
            for (auto v : entities_)
            {
                Float   mix = pick_mix(pickIndex, v->index),
                        ang = remove_angle(v->indexT * 360.f, mix);
                Mat4 trans = MATH::rotate(Mat4(1.), ang, Vec3(1,0,0));
                trans = glm::translate(trans, Vec3(0,0,-radius));

                v->transformCurrent = trans;
            }
        break;

        case A_RANDOM_PLANE:
        {
            const Vec4 sc = Vec4(radius, radius, 1, 1);
            for (auto v : entities_)
            {
                Mat4 trans = v->transformRnd;
                trans[3] *= sc;

                v->transformCurrent = trans;
            }
        }
        break;

        case A_RANDOM_3D:
        {
            const Vec4 sc = Vec4(radiusX_->value(time, thread),
                                 radiusY_->value(time, thread),
                                 radiusZ_->value(time, thread), 1);
            for (auto v : entities_)
            {
                Mat4 trans = v->transformRnd;
                trans[3] *= sc;

                v->transformCurrent = trans;
            }
        }
        break;
    }

    // final transform
    const Vec3
            scale = Vec3(scale_->value(time, thread)),
            rotv = Vec3(rotX_->value(time, thread),
                        rotY_->value(time, thread),
                        rotZ_->value(time, thread));
    const Float rota = rotation_->value(time, thread);

    if (!doPickRot && !doPickScale && !doPickPos)
    {
        for (auto v : entities_)
        {
            v->transformCurrent =
                    MATH::rotate(glm::scale(v->transformCurrent, scale)
                                , rota, rotv);
        }
    }
    else // with picking
    {
        for (auto v : entities_)
        {
            // mix by index selection
            Float mix = pickMix * pick_mix(pickIndex, v->index);

            Vec3 fscale = doPickScale
                    ? (scale + mix * (pickScale - scale))
                    : scale;
            Vec3 frotv = doPickRot
                    ? (rotv + mix * (pickRotAxis - rotv))
                    : rotv;
            Float frota = doPickRot
                    ? mix_angle(rota, pickRot, mix)
                    : rota;

            // blend rotation and scale
            v->transformCurrent =
                    MATH::rotate(glm::scale(v->transformCurrent, fscale)
                                , frota, frotv);

            // blend position
            if (doPickPos)
                v->transformCurrent[3] = v->transformCurrent[3]
                        + mix * (Vec4(pickPos, 1.) - v->transformCurrent[3]);

        }
    }
}


} // namespace MO
