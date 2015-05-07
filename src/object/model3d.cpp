/** @file model3d.cpp

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"
#include "scene.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "geom/geometryfactorysettings.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "gl/compatibility.h"
#include "geom/geometry.h"
#include "geom/geometrycreator.h"
#include "geom/geometrymodifierchain.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "param/parameterint.h"
#include "util/texturesetting.h"
#include "util/colorpostprocessingsetting.h"
#include "util/texturemorphsetting.h"
#include "util/useruniformsetting.h"
#include "io/log.h"

#if 0
#   define MO_DEBUG_MODEL(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_MODEL(unused__) { }
#endif

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    : ObjectGl      (parent),
      draw_         (0),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings(this)),
      nextGeometry_ (0),
      texture_      (new TextureSetting(this)),
      textureBump_  (new TextureSetting(this)),
      texturePostProc_(new ColorPostProcessingSetting(this)),
      textureMorph_ (new TextureMorphSetting(this)),
      textureBumpMorph_(new TextureMorphSetting(this)),
      uniformSetting_(new UserUniformSetting(this)),
      u_diff_exp_   (0),
      u_bump_scale_ (0),
      u_vertex_extrude_(0),
      doRecompile_  (false)
{
    setName("Model3D");
}

Model3d::~Model3d()
{
    resetCreator_();
}

void Model3d::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("m3d", 4);

    // v2
    geomSettings_->serialize(io);

    // v3
    texture_->serialize(io);
    // v4
    textureBump_->serialize(io);
}

void Model3d::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    const int ver = io.readHeader("m3d", 4);

    if (ver >= 2)
        geomSettings_->deserialize(io);

    if (ver >= 3)
        texture_->deserialize(io);

    if (ver >= 4)
        textureBump_->deserialize(io);
}

void Model3d::createParameters()
{
    ObjectGl::createParameters();

    params()->beginParameterGroup("renderset", tr("render settings"));

        fixPosition_ = params()->createBooleanParameter("fixposition", tr("\"skybox\" (fixed position)"),
                                     tr("When fixed, the model will always be around the camera"),
                                     tr("The model behaves normally"),
                                     tr("The model will always be centered around the camera position, like a skybox"),
                                     false, true, false);

        paramLineSmooth_ = params()->createBooleanParameter(
                    "linesmooth", tr("antialiased lines"),
                    tr("Should lines be drawn with smoothed edges"),
                    tr("The lines are drawn edgy"),
                    tr("The lines are drawn smoothly (maximum line width might change)"),
                    true,
                    true, false);

        paramLineWidth_ = params()->createFloatParameter("linewidth", tr("line width"),
                                            tr("The width of the line - currently in pixels - your driver supports maximally %1 and %2 (anti-aliased)")
                                                            // XXX Not initialized before first gl context
                                                            .arg(GL::Properties::staticInstance().lineWidth[0])
                                                            .arg(GL::Properties::staticInstance().lineWidth[1]),
                                            2, 1, 10000,
                                            0.1, true, true);

        paramPointSize_ = params()->createFloatParameter("pointsize", tr("pointsize"),
                                            tr("The size of the points in pixels"),
                                            10.0,
                                            1, true, true);

        paramPointSizeMax_ = params()->createFloatParameter("pointsizemax", tr("pointsize max"),
                                            tr("The size of the closest points in pixels"),
                                            200.0,
                                            1, true, true);

        paramPointSizeDistFac_ = params()->createFloatParameter("pointsize_distfac", tr("pointsize distance factor"),
                                            tr("Approximately the distance after which the pointsize decreases by half"),
                                            10,
                                            0.1, true, true);
        paramPointSizeDistFac_->setMinValue(0.0001);

        pointSizeAuto_ = params()->createBooleanParameter("pointsize_auto", tr("pointsize distance control"),
                                         tr("Selects if the distance to camera should control the point size"),
                                         tr("The point size is uniform"),
                                         tr("The point size is between size and max size depending on distance to camera"),
                                         false, true, false);

        numDup_ = params()->createIntParameter("num_instances", tr("gl instances"),
                                               tr("Draws the model multiple times while changing the "
                                                  "gl_InstanceID variable in the shader"),
                                               1, true, true);
        numDup_->setMinValue(1);
        /*
        dupRange_ = params()->createFloatParameter("dup_range", tr("duplicate time range"),
                                                   tr("The range of time over which duplicates are drawn"),
                                                   -1., 0.01);
        */
    params()->endParameterGroup();


    params()->beginParameterGroup("shaderset", "shader settings");

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

        diffExp_ = params()->createFloatParameter("diffuseexp", tr("diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower "
                                      "is the light cone"),
                                   4.0, 0.1);
        diffExp_->setMinValue(0.001);

        // --- glsl ---

        glslDoOverride_ = params()->createBooleanParameter("glsl_write", tr("glsl access"),
                                         tr("Enables overrides for specific functions in the shader code"),
                                         tr("Overrides are enabled for the shader of the model"),
                                         tr("No overrides"),
                                         false,
                                         true, false);


        glslVertex_ = params()->createTextParameter("glslvertex", tr("glsl position"),
                        tr("A piece of glsl code to modify vertex positions"),
                        TT_GLSL,
                        "// -- uniforms --\n"
                        "// float u_time\n"
                        "// vec3 u_cam_pos\n"
                        "// vec4 u_color\n"
                        "// mat4 u_projection\n"
                        "// mat4 u_cubeViewTransform\n"
                        "// mat4 u_viewTransform\n"
                        "// mat4 u_transform\n"
                        "// -- vertex attributes --\n"
                        "// vec4 a_position\n"
                        "// vec4 a_color\n"
                        "// vec3 a_normal\n"
                        "// vec2 a_texCoord\n"
                        "// -- special stuff --\n"
                        "// vec3 v_instance\n\n"
                        "vec3 mo_modify_position(in vec3 pos)\n{\n\treturn pos;\n}\n\n"
                        , true, false);

        glslTransform_ = params()->createTextParameter("glsltransform", tr("glsl position"),
                        tr("A piece of glsl code to modify vertex positions"),
                        TT_GLSL,
                        "// -- uniforms --\n"
                        "// float u_time\n"
                        "// vec3 u_cam_pos\n"
                        "// vec4 u_color\n"
                        "// mat4 u_projection\n"
                        "// mat4 u_cubeViewTransform\n"
                        "// mat4 u_viewTransform\n"
                        "// mat4 u_transform\n"
                        "// -- vertex attributes --\n"
                        "// vec4 a_position\n"
                        "// vec4 a_color\n"
                        "// vec3 a_normal\n"
                        "// vec2 a_texCoord\n"
                        "// -- special stuff --\n"
                        "// vec3 v_instance\n\n"
                        "mat4 mo_user_transform()\n{\n\treturn mat4(1.);\n}\n"
                        , true, false);

        glslVertexOut_ = params()->createTextParameter("glsl_color", tr("glsl vertex output"),
                        tr("A piece of glsl code to modify vertex output"),
                        TT_GLSL,
                           "// " + tr("Please be aware that this interface is likely to change in the future!") +
                           "\n\n"
                           "// " + tr("You have access to these values") + ":\n"
                           "// -- uniforms --\n"
                           "// float u_time\n"
                           "// vec3 u_cam_pos\n"
                           "// vec4 u_color\n"
                           "// mat4 u_projection\n"
                           "// mat4 u_cubeViewTransform\n"
                           "// mat4 u_viewTransform\n"
                           "// mat4 u_transform\n"
                           "// -- vertex attributes --\n"
                           "// vec4 a_position\n"
                           "// vec4 a_color\n"
                           "// vec3 a_normal\n"
                           "// vec2 a_texCoord\n"
                           "// -- input to fragment stage (changeable) --\n"
                           "// vec3 v_instance\n"
                           "// vec3 v_pos\n"
                           "// vec3 v_pos_world\n"
                           "// vec3 v_pos_eye\n"
                           "// vec3 v_normal\n"
                           "// vec3 v_normal_eye\n"
                           "// vec3 v_texCoord\n"
                           "// vec3 v_cam_dir\n"
                           "// vec4 v_color\n"
                           "// vec4 v_ambient_color\n"
                           "// vec4 gl_Position\n"
                           "\n"
                           "void mo_modify_vertex_output()\n{\n\t\n}\n"
                        , true, false);

        glslNormal_ = params()->createTextParameter("glsl_normal", tr("glsl fragment normal "),
                    tr("A piece of glsl code to change the fragment normal before lighting"),
                    TT_GLSL,
                       "// " + tr("Please be aware that this interface is likely to change in the future!") +
                       "\n\n"
                       "// " + tr("You have access to these values (! means: if available)") + ":\n"
                       "// -- uniforms --\n"
                       "// float u_time\n"
                       "// vec3 u_cam_pos\n"
                       "// float u_bump_scale\n"
                       "// sampler2D tex_0 !\n"
                       "// sampler2D tex_norm_0 !\n"
                       "// -- input from vertex stage --\n"
                       "// vec3 v_instance\n"
                       "// vec3 v_pos\n"
                       "// vec3 v_pos_world\n"
                       "// vec3 v_pos_eye\n"
                       "// vec3 v_normal\n"
                       "// vec3 v_normal_eye\n"
                       "// vec3 v_texCoord\n"
                       "// vec3 v_cam_dir\n"
                       "// vec4 v_color\n"
                       "// vec4 v_ambient_color\n"
                       "\n"
                       "vec3 mo_modify_normal(in vec3 n)\n{\n\treturn n;\n}\n"
                    , true, false);

        glslFragmentOut_ = params()->createTextParameter("glsl_fragment", tr("glsl fragment output"),
                    tr("A piece of glsl code to set or modify the output fragment color"),
                    TT_GLSL,
                       "// " + tr("Please be aware that this interface is likely to change in the future!") +
                       "\n\n"
                       "// " + tr("You have access to these values (! means: if available)") + ":\n"
                       "// -- uniforms --\n"
                       "// float u_time\n"
                       "// vec3 u_cam_pos\n"
                       "// float u_bump_scale\n"
                       "// sampler2D tex_0 !\n"
                       "// sampler2D tex_norm_0 !\n"
                       "// -- input from vertex stage --\n"
                       "// vec3 v_instance\n"
                       "// vec3 v_pos\n"
                       "// vec3 v_pos_world\n"
                       "// vec3 v_pos_eye\n"
                       "// vec3 v_normal\n"
                       "// vec3 v_normal_eye\n"
                       "// vec3 v_texCoord\n"
                       "// vec3 v_cam_dir\n"
                       "// vec4 v_color\n"
                       "// vec4 v_ambient_color\n"
                       "// -- lighting --\n"
                       "// vec3 mo_normal()\n"
                       "// ... todo\n"
                       "// -- output to rasterizer --\n"
                       "// vec4 out_color\n"
                       "\n"
                       "void mo_modify_fragment_output()\n{\n\t\n}\n"
                    , true, false);

    params()->endParameterGroup();


    params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        uniformSetting_->createParameters("g");

    params()->endParameterGroup();


    params()->beginParameterGroup("color", tr("color"));
    initParameterGroupExpanded("color");

        cbright_ = params()->createFloatParameter("bright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        cr_ = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        cg_ = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        cb_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ca_ = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();


    params()->beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col", TextureSetting::TEX_NONE, true);

        usePointCoord_ = params()->createBooleanParameter("tex_use_pointcoord", tr("map on points"),
                                         tr("Currently you need to decide wether to map the texture on triangles or on point sprites"),
                                         tr("The texture coordinates are used as defined by the vertices in the geometry"),
                                         tr("Calculated texture coordinates will be used for point sprites."),
                                         false, true, false);

    params()->endParameterGroup();


    params()->beginParameterGroup("texturepp", tr("texture post-processing"));

        texturePostProc_->createParameters("tex");

        textureMorph_->createParameters("tex");

    params()->endParameterGroup();


    params()->beginParameterGroup("texturebump", tr("normal-map texture"));

        textureBump_->createParameters("bump", TextureSetting::TEX_NONE, true, true);

        bumpScale_ = params()->createFloatParameter("bumpdepth", tr("bump scale"),
                            tr("The influence of the normal-map"),
                            1.0, 0.05);

    params()->endParameterGroup();


    params()->beginParameterGroup("texturebumppp", tr("normal-map post-proc"));

        textureBumpMorph_->createParameters("bump");

    params()->endParameterGroup();


    params()->beginParameterGroup("vertexfx", tr("vertex effects"));

        vertexFx_ = params()->createBooleanParameter("vertexfx", tr("enable effects"),
                                           tr("Enables realtime vertex processing effects"),
                                           tr("Vertex effects disabled"),
                                           tr("Vertex effects enabled"),
                                           false,
                                           true, false);

        vertexExtrude_ = params()->createFloatParameter("vertexextrude", tr("extrusion"),
                                              tr("All vertex positions of the model are moved along their "
                                                 "normals by this amount"),
                                              0.0, 0.05);

    params()->endParameterGroup();



}

void Model3d::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == lightMode_
            || p == vertexFx_
            || p == glslDoOverride_
            || p == glslVertex_
            || p == glslTransform_
            || p == glslVertexOut_
            || p == glslFragmentOut_
            || p == glslNormal_
            || p == usePointCoord_
            || p == pointSizeAuto_
            || texturePostProc_->needsRecompile(p)
            || textureMorph_->needsRecompile(p)
            || textureBumpMorph_->needsRecompile(p) )
    {
        doRecompile_ = true;
        requestRender();
    }

    if (texture_->needsReinit(p) || textureBump_->needsReinit(p)
        || uniformSetting_->needsReinit(p))
        requestReinitGl();
}

void Model3d::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    texture_->updateParameterVisibility();
    textureBump_->updateParameterVisibility();
    texturePostProc_->updateParameterVisibility();
    textureMorph_->updateParameterVisibility();
    textureBumpMorph_->updateParameterVisibility();
    uniformSetting_->updateParameterVisibility();

    diffExp_->setVisible( lightMode_->baseValue() != LM_NONE );

    bool vertfx = vertexFx_->baseValue();
    vertexExtrude_->setVisible(vertfx);

    bool glsl = glslDoOverride_->baseValue();
    glslVertex_->setVisible(glsl);
    glslTransform_->setVisible(glsl);
    glslVertexOut_->setVisible(glsl);
    glslFragmentOut_->setVisible(glsl);
    glslNormal_->setVisible(glsl);

    bool psdist = pointSizeAuto_->baseValue() != 0;
    paramPointSizeMax_->setVisible(psdist);
    paramPointSizeDistFac_->setVisible(psdist);
}

void Model3d::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    texture_->getNeededFiles(files, IO::FT_TEXTURE);
    textureBump_->getNeededFiles(files, IO::FT_NORMAL_MAP);

    geomSettings_->getNeededFiles(files);
}

const GEOM::Geometry* Model3d::geometry() const
{
    return draw_ ? draw_->geometry() : 0;
}

Vec4 Model3d::modelColor(Double time, uint thread) const
{
    const auto b = cbright_->value(time, thread);
    return Vec4(
        cr_->value(time, thread) * b,
        cg_->value(time, thread) * b,
        cb_->value(time, thread) * b,
        ca_->value(time, thread));
}

const GEOM::GeometryFactorySettings& Model3d::geometrySettings() const
{
    geomSettings_->setObject(const_cast<Model3d*>(this));
    return *geomSettings_;
}

void Model3d::initGl(uint /*thread*/)
{
    MO_DEBUG_MODEL("Model3d::initGl()");

    texture_->initGl();
    textureBump_->initGl();

    draw_ = new GL::Drawable(idName());

    // lazy-creation of resources?
    const bool lazy = sceneObject() ? sceneObject()->lazyFlag() : false;

    // create resources
    if (lazy)
    {
        nextGeometry_ = new GEOM::Geometry;
        geomSettings_->setObject(this);
        geomSettings_->modifierChain()->execute(nextGeometry_, this);
    }
    else
    if (!nextGeometry_)
    {
        resetCreator_();
        creator_ = new GEOM::GeometryCreator(this);
        /** @todo find out if Qt's signal/slot mechanism doesn't work when
            not connected to main thread. In this case, when started via DiskRenderer,
            no signals are received from the GEOM::GeometryCreator.
            It's currently solved via the Scene::lazyFlag() but would be cool
            to file a bugreport if this generally doesn't work or to, at least,
            find out if this is the desired behaviour. */
        connect(creator_, SIGNAL(succeeded()), this, SLOT(geometryCreated_()));
        connect(creator_, SIGNAL(failed(QString)), this, SLOT(geometryFailed_()));

        geomSettings_->setObject(this);
        creator_->setSettings(*geomSettings_);
        creator_->start();
        MO_DEBUG_MODEL("Model3d::initGl() started creator");
    }
}

void Model3d::releaseGl(uint /*thread*/)
{
    texture_->releaseGl();
    textureBump_->releaseGl();
    uniformSetting_->releaseGl();

    if (draw_->isReady())
        draw_->releaseOpenGl();

    delete draw_;
    draw_ = 0;

    resetCreator_();
}

void Model3d::resetCreator_()
{
    if (creator_)
    {
        if (creator_->isRunning())
        {
            connect(creator_, SIGNAL(finished()), creator_, SLOT(deleteLater()));
            creator_->discard();
        }
        else
            creator_->deleteLater();
    }
    creator_ = 0;
}

void Model3d::numberLightSourcesChanged(uint /*thread*/)
{
    doRecompile_ = true;
}

void Model3d::geometryCreated_()
{
    MO_DEBUG_MODEL("Model3d::geometryCreated()");

    nextGeometry_ = creator_->takeGeometry();
    creator_->deleteLater();
    creator_ = 0;

    requestRender();
}

void Model3d::geometryFailed_()
{
    MO_DEBUG_MODEL("Model3d::geometryFailed()");

    creator_->deleteLater();
    creator_ = 0;
}

void Model3d::setGeometrySettings(const GEOM::GeometryFactorySettings & s)
{
    *geomSettings_ = s;
    geomSettings_->setObject(this);
    requestReinitGl();
}

void Model3d::setGeometry(const GEOM::Geometry & g)
{
    if (!nextGeometry_)
        nextGeometry_ = new GEOM::Geometry;
    *nextGeometry_ = g;
    requestRender();
}

void Model3d::setupDrawable_()
{
    MO_DEBUG_MODEL("Model3d::setupDrawable()");

    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();

    if (numberLightSources() > 0 && lightMode_->baseValue() != LM_NONE)
    {
        src->addDefine("#define MO_ENABLE_LIGHTING");
        src->addDefine(QString("#define MO_NUM_LIGHTS %1")
                       .arg(numberLightSources()));
    }
    if (lightMode_->baseValue() == LM_PER_FRAGMENT)
        src->addDefine("#define MO_FRAGMENT_LIGHTING");
    if (texture_->isEnabled())
        src->addDefine("#define MO_ENABLE_TEXTURE");
    if (texturePostProc_->isEnabled())
        src->addDefine("#define MO_ENABLE_TEXTURE_POST_PROCESS");
    if (texture_->isCube())
        src->addDefine("#define MO_TEXTURE_IS_FULLDOME_CUBE");
    if (textureBump_->isEnabled())
        src->addDefine("#define MO_ENABLE_NORMALMAP");
    if (textureMorph_->isTransformEnabled())
        src->addDefine("#define MO_ENABLE_TEXTURE_TRANSFORMATION");
    if (textureMorph_->isSineMorphEnabled())
        src->addDefine("#define MO_ENABLE_TEXTURE_SINE_MORPH");
    if (textureBumpMorph_->isTransformEnabled())
        src->addDefine("#define MO_ENABLE_NORMALMAP_TRANSFORMATION");
    if (textureBumpMorph_->isSineMorphEnabled())
        src->addDefine("#define MO_ENABLE_NORMALMAP_SINE_MORPH");
    if (usePointCoord_->baseValue() != 0)
        src->addDefine("#define MO_USE_POINT_COORD");
    if (pointSizeAuto_->baseValue() != 0)
        src->addDefine("#define MO_ENABLE_POINT_SIZE_DISTANCE");
    if (vertexFx_->baseValue())
        src->addDefine("#define MO_ENABLE_VERTEX_EFFECTS");
    // glsl injection
    if (glslDoOverride_->baseValue())
    {
        src->addDefine("#define MO_ENABLE_VERTEX_OVERRIDE");
        src->addDefine("#define MO_ENABLE_FRAGMENT_OVERRIDE");
        src->addDefine("#define MO_ENABLE_NORMAL_OVERRIDE");
        QString text =
                  "#line 1\n"
                + glslVertex_->value() + "\n#line 1\n"
                + glslVertexOut_->value() + "\n";
        src->replace("//%mo_override_vert%", text);
        src->replace("//%mo_override_vert2%", "#line 1\n" + glslTransform_->value() + "\n");
        src->replace("//%mo_override_frag%", "#line 1\n" + glslFragmentOut_->value() + "\n");
        src->replace("//%mo_override_normal%", "#line 1\n" + glslNormal_->value() + "\n");
    }
    // resolve includes
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        return getGlslInclude(url, do_search);
    });
    // declare user uniforms
    src->replace("//%user_uniforms%",
                 "// " + tr("runtime user uniforms") + "\n"
                 + uniformSetting_->getDeclarations());
    MO_DEBUG_GL("Model3d(" << name() << "): user uniforms:\n" << uniformSetting_->getDeclarations());

    draw_->setShaderSource(src);

    try
    {
        draw_->createOpenGl();
    }
    catch (const Exception& e)
    {
        MO_WARNING("Error on initializing model for '" << name() << "'\n" << e.what());
        for (const GL::Shader::CompileMessage & msg : draw_->shader()->compileMessages())
        {
            if (msg.program == GL::Shader::P_VERTEX
                || msg.program == GL::Shader::P_LINKER)
            {
                glslVertex_->addErrorMessage(msg.line, msg.text);
                glslVertexOut_->addErrorMessage(msg.line, msg.text);
                glslTransform_->addErrorMessage(msg.line, msg.text);
            }
            if (msg.program == GL::Shader::P_FRAGMENT
                || msg.program == GL::Shader::P_LINKER)
            {
                glslFragmentOut_->addErrorMessage(msg.line, msg.text);
                glslNormal_->addErrorMessage(msg.line, msg.text);
            }
        }
    }

    // get uniforms
    u_diff_exp_ = draw_->shader()->getUniform(src->uniformNameDiffuseExponent());
    u_cam_pos_ = draw_->shader()->getUniform("u_cam_pos");
    u_instance_count_ = draw_->shader()->getUniform("u_instance_count");

    const bool isvertfx = vertexFx_->baseValue();
    u_vertex_extrude_ = draw_->shader()->getUniform("u_vertex_extrude", isvertfx);

    u_pointsize_ = draw_->shader()->getUniform("u_pointsize_dist", false);

    if (texture_->isEnabled())
    {
        if (texturePostProc_->isEnabled())
            texturePostProc_->getUniforms(draw_->shader());

        // (checks for itself)
        textureMorph_->getUniforms(draw_->shader());
    }

    if (textureBump_->isEnabled())
    {
        u_bump_scale_ = draw_->shader()->getUniform(src->uniformNameBumpScale());

        textureBumpMorph_->getUniforms(draw_->shader(), "_bump");
    }

    uniformSetting_->tieToShader(draw_->shader());
}

void Model3d::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    MO_DEBUG_MODEL("Model3d::renderGl(" << thread << ", " << time << ")");

    Mat4 trans = transformation();
    Mat4 cubeViewTrans, viewTrans;
    if (fixPosition_->baseValue() == 0)
    {
        cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
        viewTrans = rs.cameraSpace().viewMatrix() * trans;
    }
    else
    {
        trans[3] = Vec4(0., 0., 0., 1.);
        Mat4 vm = rs.cameraSpace().cubeViewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        cubeViewTrans = vm * trans;

        vm = rs.cameraSpace().viewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        viewTrans = vm * trans;
    }

    if (nextGeometry_)
    {
        MO_DEBUG_MODEL("Model3d::renderGl: assigning next geometry");

        auto g = nextGeometry_;
        nextGeometry_ = 0;
        draw_->setGeometry(g);
        setupDrawable_();
    }

    if (doRecompile_)
    {
        MO_DEBUG_MODEL("Model3d::renderGl: recompiling shader");

        doRecompile_ = false;
        setupDrawable_();
    }

    if (draw_->isReady())
    {
        MO_DEBUG_MODEL("Model3d::renderGl: drawing");

        // bind textures
        texture_->bind();
        textureBump_->bind(1);
        uint texSlot = 2;

        const int numDup = numDup_->value(time, thread);

        // update uniforms
        const auto bright = cbright_->value(time, thread);
        draw_->setAmbientColor(
                    cr_->value(time, thread) * bright,
                    cg_->value(time, thread) * bright,
                    cb_->value(time, thread) * bright,
                    ca_->value(time, thread));

        if (u_instance_count_)
            u_instance_count_->floats[0] = numDup;
        if (u_diff_exp_)
            u_diff_exp_->floats[0] = diffExp_->value(time, thread);
        if (u_bump_scale_)
            u_bump_scale_->floats[0] = bumpScale_->value(time, thread);
        if (u_vertex_extrude_)
            u_vertex_extrude_->floats[0] = vertexExtrude_->value(time, thread);

        uniformSetting_->updateUniforms(time, thread, texSlot);

        if (texture_->isEnabled())
        {
            if (texturePostProc_->isEnabled())
                texturePostProc_->updateUniforms(time, thread);

            textureMorph_->updateUniforms(time, thread);
        }

        if (textureBump_->isEnabled())
        {
            textureBumpMorph_->updateUniforms(time, thread);
        }

        if (u_cam_pos_)
        {
            const Vec3& pos = rs.cameraSpace().position();
            u_cam_pos_->setFloats(pos.x, pos.y, pos.z, 0.);
        }

        // draw state

        GL::Properties::staticInstance().setLineSmooth(paramLineSmooth_->value(time, thread) != 0);
        GL::Properties::staticInstance().setLineWidth(paramLineWidth_->value(time, thread));
        GL::Properties::staticInstance().setPointSize(paramPointSize_->value(time, thread));

        if (pointSizeAuto_->baseValue())
        {
            MO_CHECK_GL( gl::glEnable(gl::GL_PROGRAM_POINT_SIZE) );
            if (u_pointsize_)
            {
                float mi = paramPointSize_->value(time, thread);
                u_pointsize_->setFloats(mi,
                                        paramPointSizeMax_->value(time, thread) - mi,
                                        1.f / paramPointSizeDistFac_->value(time, thread),
                                        0);
            }
        }
        else
            MO_CHECK_GL( gl::glDisable(gl::GL_PROGRAM_POINT_SIZE) );

        // render the thing

        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans,
                            viewTrans,
                            trans,
                            &rs.lightSettings(),
                            time, numDup);
    }
    else
        MO_DEBUG_MODEL("Model3d::renderGl: drawable not ready");
}





} // namespace MO
