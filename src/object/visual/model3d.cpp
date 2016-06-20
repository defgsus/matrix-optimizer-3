/** @file model3d.cpp

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"
#include "object/scene.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "geom/geometryfactorysettings.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "gl/compatibility.h"
#include "gl/texture.h"
#include "geom/geometry.h"
#include "geom/geometrycreator.h"
#include "geom/geometrymodifierchain.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterint.h"
#include "object/util/texturesetting.h"
#include "object/util/colorpostprocessingsetting.h"
#include "object/util/texturemorphsetting.h"
#include "object/util/useruniformsetting.h"
#include "gui/geometrydialog.h"
#include "io/application.h"
#include "io/log_gl.h"

#if 0
#   define MO_DEBUG_MODEL(arg__) MO_PRINT("Model3d::" << arg__)
#else
#   define MO_DEBUG_MODEL(unused__) { }
#endif

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d()
    : ObjectGl      (),
      draw_         (0),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings(this)),
      nextGeometry_ (0),
      texture_      (new TextureSetting(this)),
      textureBump_  (new TextureSetting(this)),
      textureEnv_   (new TextureSetting(this)),
      texturePostProc_(new ColorPostProcessingSetting(this)),
      textureMorph_ (new TextureMorphSetting(this)),
      textureBumpMorph_(new TextureMorphSetting(this)),
      uniformSetting_(new UserUniformSetting(this)),
      u_light_amt_  (0),
      u_bump_scale_ (0),
      u_vertex_extrude_(0),
      doRecompile_  (false),
      loadedVersion_(0)
    , xxx_2d        (0)
    , xxx_cube      (0)
    , xxx_u_2d        (0)
    , xxx_u_cube      (0)
{
    setName("Model3D");
    initDefaultUpdateMode(UM_ALWAYS, false);
    setNumberOutputs(ST_GEOMETRY, 1);
}

Model3d::~Model3d()
{
    delete texture_;
    delete textureBump_;
    delete textureEnv_;
    resetCreator_();
}

void Model3d::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("m3d", 5);
    // v5 changes mo_modify_light()

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

    const int ver = loadedVersion_ = io.readHeader("m3d", 5);

    if (ver >= 2)
        geomSettings_->deserialize(io);

    if (ver >= 3)
        texture_->deserialize(io);

    if (ver >= 4)
        textureBump_->deserialize(io);
}

void Model3d::updateCodeVersion_()
{
    if (!loadedVersion_)
        return;

    if (loadedVersion_ < 5)
    {
        QString s = glslLight_->value();
        s.replace("mo_modify_light(in int index, in vec3 light_normal",
                  "mo_modify_light(in int index, in vec3 surface_normal, in vec3 light_normal");
        glslLight_->setValue(s);
    }
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

        paramPolySmooth_ = params()->createBooleanParameter(
                    "polysmooth", tr("antialiased polygons"),
                    tr("*deprecated* Should polygons be drawn with smoothed edges"),
                    tr("The polygons are drawn edgy"),
                    tr("The polygons are drawn smoothly"),
                    false,
                    true, false);

        paramLineSmooth_ = params()->createBooleanParameter(
                    "linesmooth", tr("antialiased lines"),
                    tr("Should lines be drawn with smoothed edges"),
                    tr("The lines are drawn edgy"),
                    tr("The lines are drawn smoothly (maximum line width might change)"),
                    true,
                    true, false);

        paramLineWidth_ = params()->createFloatParameter("linewidth", tr("line width"),
                                            tr("*deprecated* The width of the line - currently in pixels - your driver supports maximally %1 and %2 (anti-aliased)")
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

        paramNumInstance_ = params()->createIntParameter("num_instances", tr("gl instances"),
                                               tr("Draws the model multiple times while changing the "
                                                  "gl_InstanceID variable in the shader"),
                                               1, true, true);
        paramNumInstance_->setMinValue(1);
        /*
        dupRange_ = params()->createFloatParameter("dup_range", tr("duplicate time range"),
                                                   tr("The range of time over which duplicates are drawn"),
                                                   -1., 0.01);
        */
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
                                   tr("Exponent for the specular lighting - the higher, the narrower"),
                                   10.0, 0.1);
        specExp_->setMinValue(0.001);

    params()->endParameterGroup();


    params()->beginParameterGroup("glsl", "glsl");

        // --- glsl ---

        glslDoOverride_ = params()->createBooleanParameter("glsl_write", tr("glsl access"),
                                         tr("Enables overrides for specific functions in the shader code"),
                                         tr("Overrides are enabled for the shader of the model"),
                                         tr("No overrides"),
                                         false,
                                         true, false);
        glslDoOverride_->setDefaultEvolvable(false);

        glslVertex_ = params()->createTextParameter("glslvertex", tr("vertex position"),
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

        glslTransform_ = params()->createTextParameter("glsltransform", tr("vertex transform"),
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
                        "#include <transform>\n#include <constants>\n\n"
                        "mat4 mo_user_transform()\n{\n"
                        "\tmat4 m = mat4(1.);\n"
                        "\t//m = rotate(m, degree(45.) * a_position.y);\n"
                        "\treturn m;\n}\n"
                        , true, false);

        glslVertexOut_ = params()->createTextParameter("glsl_color", tr("vertex output"),
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

        glslNormal_ = params()->createTextParameter("glsl_normal", tr("fragment normal"),
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
                       "// mat3 v_normal_space\n"
                       "// vec3 v_texCoord\n"
                       "// vec3 v_cam_dir\n"
                       "// vec4 v_color\n"
                       "// vec4 v_ambient_color\n"
                       "\n"
                       "// a direction of (0,0,1) is the default normal\n"
                       "vec3 mo_modify_normal(in vec3 n)\n{\n\treturn n;\n}\n"
                    , true, false);

        glslLight_ = params()->createTextParameter("glsl_light_mix", tr("fragment lighting"),
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
                       "// mat3 v_normal_space\n"
                       "// vec3 v_texCoord\n"
                       "// vec3 v_cam_dir\n"
                       "// vec4 v_color\n"
                       "// vec4 v_ambient_color\n"
                       "// -- lighting --\n"
                       "// vec3 mo_normal\n"
                       "\n"
                       "// " + tr("The lighting has already been calculated at this point") +
                       "\n// " + tr("Modify color, diffuse or specular before mixing") +
                       "\nvoid mo_modify_light(in int index, in vec3 surface_normal, in vec3 light_normal,\n"
                       "                     inout vec4 color, inout float diffuse, inout float specular)\n"
                       "{\n\t\n}\n"
                    , true, false);

        glslFragmentOut_ = params()->createTextParameter("glsl_fragment", tr("fragment output"),
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
                       "// mat3 v_normal_space\n"
                       "// vec3 v_texCoord\n"
                       "// vec3 v_cam_dir\n"
                       "// vec4 v_color\n"
                       "// vec4 v_ambient_color\n"
                       "// -- lighting --\n"
                       "// vec3 mo_normal\n"
                       "// vec4 mo_light_color\n"
                       "// ... todo\n"
                       "// -- output to rasterizer --\n"
                       "// vec4 out_color\n"
                       "\n"
                       "void mo_modify_fragment_output()\n{\n\t\n}\n"
                    , true, false);

        glslDoGeometry_ = params()->createBooleanParameter(
                    "glsl_do_geometry", tr("geometry shader"),
                 tr("Enables the geometry shader code"),
                 tr("Overrides are enabled for the shader of this model"),
                 tr("No overrides"),
                 false,
                 true, false);
        glslDoGeometry_->setDefaultEvolvable(false);


        glslGeometry_ = params()->createTextParameter(
                    "glsl_geometry", tr("geometry shader"),
                    tr("A piece of glsl code to generate primitives"),
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
       "// mat3 v_normal_space\n"
       "// vec3 v_texCoord\n"
       "// vec3 v_cam_dir\n"
       "// vec4 v_color\n"
       "// vec4 v_ambient_color\n"
       "// -- lighting --\n"
       "// vec3 mo_normal\n"
       "// vec4 mo_light_color\n"
       "// ... todo\n"
       "// -- output to rasterizer --\n"
       "// vec4 out_color\n"
       "\n"
       "void main()\n{\n\t\n}\n"
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
        ca_->setDefaultEvolvable(false);

    params()->endParameterGroup();


    params()->beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("_img_texcol", tr("color texture"));

        usePointCoord_ = params()->createBooleanParameter("tex_use_pointcoord", tr("map on points"),
                     tr("Currently you need to decide wether to map the texture on triangles or on point sprites"),
                     tr("The texture coordinates are used as defined by the vertices in the geometry"),
                     tr("Calculated texture coordinates will be used for point sprites."),
                     false, true, false);

        texturePostProc_->createParameters("tex");

        textureMorph_->createParameters("tex");

    params()->endParameterGroup();


    params()->beginParameterGroup("texturebump", tr("normal-map texture"));

        textureBump_->createParameters("_img_texbump",
                    tr("normal-map texture"), ParameterTexture::IT_NONE, true);

        bumpScale_ = params()->createFloatParameter("bumpdepth", tr("bump scale"),
                            tr("The influence of the normal-map"),
                            .1, 0.01);

        textureBumpMorph_->createParameters("bump");

    params()->endParameterGroup();


    params()->beginParameterGroup("env_map", "environment map");

        textureEnv_->createParameters("_img_tex_env", tr("envmap texture"));

        /*envMapType_ = params()->createSelectParameter("env_map_type", tr("input map type"),
                                    tr("Selects the type of texture to use"),
                                                      )
        */

        envMapAmt_ = params()->createFloatParameter("env_map_amt", tr("amount"),
                                                    tr("The overall amount of the environment mapping"),
                                                    0.5f, 0.05f, true, true);

        envMapAmt2_ = params()->createFloatParameter("env_map_amt2", tr("amount direct"),
                    tr("The amount of the environment mapping when the angle of incident "
                       "is close to the emergent angle."),
                    1.f, 0.05f, true, true);

        envMapAmt3_ = params()->createFloatParameter("env_map_amt3", tr("amount far"),
                    tr("The amount of the environment mapping when the angle of incident "
                       "is opposite to the emergent angle."),
                    1.f, 0.05f, true, true);

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

void Model3d::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();
    updateCodeVersion_();
    texture_->fixCompatibility();
    textureBump_->fixCompatibility();
    textureEnv_->fixCompatibility();
}

void Model3d::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == lightMode_
            || p == vertexFx_
            || p == glslDoOverride_
            || p == glslDoGeometry_
            || p == glslVertex_
            || p == glslTransform_
            || p == glslVertexOut_
            || p == glslFragmentOut_
            || p == glslNormal_
            || p == glslLight_
            || p == glslGeometry_
            || p == usePointCoord_
            || p == pointSizeAuto_
            || texture_->onParameterChange(p)
            || textureBump_->onParameterChange(p)
            || textureEnv_->onParameterChange(p)
            || texturePostProc_->needsRecompile(p)
            || textureMorph_->needsRecompile(p)
            || textureBumpMorph_->needsRecompile(p) )
    {
        doRecompile_ = true;
        requestRender();
    }

    if (//texture_->needsReinit(p) || textureBump_->needsReinit(p)
        //|| textureEnv_->needsReinit(p)
         uniformSetting_->needsReinit(p))
        requestReinitGl();
}

void Model3d::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    texture_->updateParameterVisibility();
    textureBump_->updateParameterVisibility();
    textureEnv_->updateParameterVisibility();
    texturePostProc_->setVisible(texture_->isEnabled());
    textureMorph_->setVisible(texture_->isEnabled());
    textureBumpMorph_->setVisible(textureBump_->isEnabled());
    uniformSetting_->updateParameterVisibility();
    usePointCoord_->setVisible(texture_->isEnabled());

    diffAmt_->setVisible( lightMode_->baseValue() != LM_NONE );
    diffExp_->setVisible( lightMode_->baseValue() != LM_NONE );
    specAmt_->setVisible( lightMode_->baseValue() != LM_NONE );
    specExp_->setVisible( lightMode_->baseValue() != LM_NONE );

    bool vertfx = vertexFx_->baseValue();
    vertexExtrude_->setVisible(vertfx);

    bool glsl = glslDoOverride_->baseValue();
    glslVertex_->setVisible(glsl);
    glslTransform_->setVisible(glsl);
    glslVertexOut_->setVisible(glsl);
    glslFragmentOut_->setVisible(glsl);
    glslNormal_->setVisible(glsl);
    glslLight_->setVisible(glsl);
    glslDoGeometry_->setVisible(glsl);
    glslGeometry_->setVisible(glsl && glslDoGeometry_->baseValue());

    bool psdist = pointSizeAuto_->baseValue() != 0;
    paramPointSizeMax_->setVisible(psdist);
    paramPointSizeDistFac_->setVisible(psdist);

    bool doEnv = textureEnv_->isEnabled();
    envMapAmt_->setVisible(doEnv);
    envMapAmt2_->setVisible(doEnv);
    envMapAmt3_->setVisible(doEnv);

    bool doBump = textureBump_->isEnabled();
    bumpScale_->setVisible(doBump);
    textureBumpMorph_->setVisible(doBump);
}

void Model3d::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    texture_->getNeededFiles(files);
    textureBump_->getNeededFiles(files);
    textureEnv_->getNeededFiles(files);

    geomSettings_->getNeededFiles(files);
}

const GEOM::Geometry* Model3d::geometry() const
{
    return draw_ ? draw_->geometry() : 0;
}

Vec4 Model3d::modelColor(const RenderTime& time) const
{
    const auto b = cbright_->value(time);
    return Vec4(
        cr_->value(time) * b,
        cg_->value(time) * b,
        cb_->value(time) * b,
        ca_->value(time));
}

const GEOM::GeometryFactorySettings& Model3d::getGeometrySettings() const
{
    geomSettings_->setObject(const_cast<Model3d*>(this));
    return *geomSettings_;
}

void Model3d::initGl(uint /*thread*/)
{
    MO_DEBUG_MODEL("initGl()");

    // create geometry
    draw_ = new GL::Drawable(idName());

    // lazy-creation of resources?
    const bool lazy = sceneObject() ? sceneObject()->lazyFlag() : false;

    // -- create resources --

    if (lazy) // instantly
    {
        nextGeometry_ = new GEOM::Geometry;
        geomSettings_->setObject(this);
        geomSettings_->modifierChain()->execute(nextGeometry_, this);
    }
    else
    if (!nextGeometry_) // or in background
    {
        resetCreator_();
        creator_ = new GEOM::GeometryCreator(application());
        /** @todo find out if Qt's signal/slot mechanism doesn't work when
            not connected to main thread. In this case, when started via DiskRenderer,
            no signals are received from the GEOM::GeometryCreator.
            It's currently solved via the Scene::lazyFlag() but would be good
            to find out if this is the desired behaviour. */
        QObject::connect(creator_, &GEOM::GeometryCreator::succeeded,
                         [=](){ geometryCreated_(); });
        QObject::connect(creator_, &GEOM::GeometryCreator::failed,
                         [=](const QString& e){ geometryFailed_(e); });
        geomSettings_->setObject(this);
        creator_->setSettings(*geomSettings_);
        creator_->start();
        MO_DEBUG_MODEL("initGl() started creator");
    }
}

void Model3d::releaseGl(uint /*thread*/)
{
    texture_->releaseGl();
    textureBump_->releaseGl();
    textureEnv_->releaseGl();
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
            //creator_->setParent(0);
            QObject::connect(creator_, SIGNAL(finished()),
                             creator_, SLOT(deleteLater()));
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
    MO_DEBUG_MODEL("geometryCreated()");

    nextGeometry_ = creator_->takeGeometry();
    creator_->deleteLater();
    creator_ = 0;

    requestRender();
}

void Model3d::geometryFailed_(const QString& e)
{
    MO_DEBUG_MODEL("geometryFailed()");

    setErrorMessage(tr("Failed to create geometry:\n%1").arg(e));

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

GL::ShaderSource Model3d::valueShaderSource(uint channel) const
{
    if (!draw_ || channel > 0)
        return GL::ShaderSource();
    return *draw_->shaderSource();
}

void Model3d::setupDrawable_()
{
    MO_DEBUG_MODEL("setupDrawable()");

    clearError();

    if (!draw_ || !draw_->geometry())
    {
        setErrorMessage(QString("No geometry for Model3d"));
        return;
    }

    // -------- construct the GLSL source ----------

    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadDefaultSource();
    if (glslDoGeometry_->baseValue())
        src->setGeometrySource(glslGeometry_->baseValue());

    QString defines;

    if (auto s = sceneObject())
        if (s->isRendering())
            defines += "#define MO_RENDER";

    if (numberLightSources() > 0 && lightMode_->baseValue() != LM_NONE)
        defines += ("\n#define MO_ENABLE_LIGHTING");
    // still pass light info
    defines += (QString("\n#define MO_NUM_LIGHTS %1")
                   .arg(numberLightSources()));
    if (lightMode_->baseValue() == LM_PER_FRAGMENT)
        defines += ("\n#define MO_FRAGMENT_LIGHTING");
    if (texture_->isEnabled())
        defines += ("\n#define MO_ENABLE_TEXTURE");
    if (texturePostProc_->isEnabled())
        defines += ("\n#define MO_ENABLE_TEXTURE_POST_PROCESS");
    if (texture_->isCube())
        defines += ("\n#define MO_TEXTURE_IS_FULLDOME_CUBE");
    if (textureBump_->isEnabled())
        defines += ("\n#define MO_ENABLE_NORMALMAP");
    if (textureEnv_->isEnabled())
    {
        defines += ("\n#define MO_ENABLE_ENV_MAP");
        // XXX Note, when the texture is a tex-input,
        // this check only occures after the first render pass
        // because TextureSetting::bind() in renderGl() is
        // the first to actually read the texture input
        if (textureEnv_->isCube())
            defines += ("\n#define MO_ENV_MAP_IS_CUBE");
    }
    if (textureMorph_->isTransformEnabled())
        defines += ("\n#define MO_ENABLE_TEXTURE_TRANSFORMATION");
    if (textureMorph_->isSineMorphEnabled())
        defines += ("\n#define MO_ENABLE_TEXTURE_SINE_MORPH");
    if (textureBumpMorph_->isTransformEnabled())
        defines += ("\n#define MO_ENABLE_NORMALMAP_TRANSFORMATION");
    if (textureBumpMorph_->isSineMorphEnabled())
        defines += ("\n#define MO_ENABLE_NORMALMAP_SINE_MORPH");
    if (usePointCoord_->baseValue() != 0)
        defines += ("\n#define MO_USE_POINT_COORD");
    if (pointSizeAuto_->baseValue() != 0)
        defines += ("\n#define MO_ENABLE_POINT_SIZE_DISTANCE");
    if (vertexFx_->baseValue())
        defines += ("\n#define MO_ENABLE_VERTEX_EFFECTS");
    // glsl injection
    if (glslDoOverride_->baseValue())
    {
        defines += ("\n#define MO_ENABLE_VERTEX_OVERRIDE");
        defines += ("\n#define MO_ENABLE_FRAGMENT_OVERRIDE");
        defines += ("\n#define MO_ENABLE_NORMAL_OVERRIDE");
        defines += ("\n#define MO_ENABLE_LIGHT_OVERRIDE");
    }
    src->addDefine(defines);

    if (glslDoOverride_->baseValue())
    {
        QString text =
                  "#line 1\n"
                + glslVertex_->value() + "\n#line 1\n"
                + glslVertexOut_->value() + "\n";
        src->replace("//%mo_override_vert%", text);
        src->replace("//%mo_override_vert2%", "#line 1\n" + glslTransform_->value() + "\n");
        src->replace("//%mo_override_frag%", "#line 1\n" + glslFragmentOut_->value() + "\n");
        src->replace("//%mo_override_normal%", "#line 1\n" + glslNormal_->value() + "\n");
        src->replace("//%mo_override_light%", "#line 1\n" + glslLight_->value() + "\n");
    }

    // resolve includes
    glslFragmentOut_->clearIncludeObjects();
    glslLight_->clearIncludeObjects();
    glslNormal_->clearIncludeObjects();
    glslTransform_->clearIncludeObjects();
    glslVertexOut_->clearIncludeObjects();
    glslVertex_->clearIncludeObjects();
    glslGeometry_->clearIncludeObjects();
    src->replaceIncludes([this](const QString& url, bool do_search)
    {
        Object* object;
        auto inc = getGlslInclude(url, do_search, &object);
        if (object)
        {
            glslFragmentOut_->addIncludeObject(object->idName());
            glslLight_->addIncludeObject(object->idName());
            glslNormal_->addIncludeObject(object->idName());
            glslTransform_->addIncludeObject(object->idName());
            glslVertexOut_->addIncludeObject(object->idName());
            glslVertex_->addIncludeObject(object->idName());
            glslGeometry_->addIncludeObject(object->idName());
        }
        return inc;
    });
    // declare user uniforms
    src->replace("//%user_uniforms%",
                 "// " + tr("runtime user uniforms") + "\n"
                 + uniformSetting_->getDeclarations());
    MO_DEBUG_GL("Model3d(" << name() << "): user uniforms:\n" << uniformSetting_->getDeclarations());

    draw_->setShaderSource(src);

    try
    {
        MO_DEBUG_MODEL("Creating Shader and VAO");
        draw_->createOpenGl();
    }
    catch (const Exception& e)
    {
        MO_WARNING("Model3d '" << name() << "'s createOpenGL failed with\n" << e.what());
        for (const GL::Shader::CompileMessage & msg : draw_->shader()->compileMessages())
        {
            if (msg.program == GL::Shader::P_GEOMETRY
                || msg.program == GL::Shader::P_LINKER)
            {
                glslGeometry_->addErrorMessage(msg.line, msg.text);
            }
            if (msg.program == GL::Shader::P_VERTEX
                || msg.program == GL::Shader::P_LINKER)
            {
                // XXX It's not clear which editor is responsible...
                glslVertex_->addErrorMessage(msg.line, msg.text);
                glslTransform_->addErrorMessage(msg.line, msg.text);
                glslVertexOut_->addErrorMessage(msg.line, msg.text);
            }
            if (msg.program == GL::Shader::P_FRAGMENT
                || msg.program == GL::Shader::P_LINKER)
            {
                glslNormal_->addErrorMessage(msg.line, msg.text);
                glslLight_->addErrorMessage(msg.line, msg.text);
                glslFragmentOut_->addErrorMessage(msg.line, msg.text);
                glslNormal_->addErrorMessage(msg.line, msg.text);
            }
        }
        setErrorMessage(tr("Failed to initialize drawable (%1)").arg(e.what()));
        // XXX Should deinitialize or otherwise flag the object
        return;
    }

    // get uniforms
    u_light_amt_ = draw_->shader()->getUniform(src->uniformNameLightAmt());
    u_cam_pos_ = draw_->shader()->getUniform("u_cam_pos");
    u_instance_count_ = draw_->shader()->getUniform("u_instance_count");
    u_env_map_amt_ = draw_->shader()->getUniform("u_env_map_amt");

    const bool isvertfx = vertexFx_->baseValue();
    u_vertex_extrude_ = draw_->shader()->getUniform("u_vertex_extrude", isvertfx);

    u_pointsize_ = draw_->shader()->getUniform("u_pointsize_dist");

    u_tex_0_ = draw_->shader()->getUniform("tex_0");
    u_texn_0_ = draw_->shader()->getUniform("tex_norm_0");
    u_tex_env_0_ = draw_->shader()->getUniform("tex_env_0");
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

    xxx_u_2d = draw_->shader()->getUniform("u_2d");
    xxx_u_cube = draw_->shader()->getUniform("u_cube");
}



void Model3d::renderGl(const GL::RenderSettings& rs, const RenderTime& time)
{
    MO_DEBUG_MODEL("renderGl(" << time << ")");

    /** @todo wireframe-mode for Model3d, eg. glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */


#if 0 // XXX debugging GL_INVALID_OPERATION in vao->drawElements()
      //use these to trigger (needs use in shader)
    if (!xxx_2d)
    {
        std::vector<gl::GLfloat> vec(1024*1024*4);
        xxx_2d = new GL::Texture(1024,1024,gl::GL_RGBA,gl::GL_RGBA,gl::GL_FLOAT,
                                 &vec[0]);
        xxx_2d->create();
        xxx_cube = new GL::Texture(1024,1024,gl::GL_RGBA,gl::GL_RGBA,gl::GL_FLOAT,
                                 &vec[0],&vec[0],&vec[0],&vec[0],&vec[0],&vec[0]);
        xxx_cube->create();
    }

    if (xxx_u_2d)
        { xxx_u_2d->ints[0] = texSlot; GL::Texture::setActiveTexture(texSlot);
            xxx_2d->bind(); ++texSlot; }

    if (xxx_u_cube)
        { xxx_u_cube->ints[0] = texSlot; GL::Texture::setActiveTexture(texSlot);
            xxx_cube->bind(); ++texSlot; }
#endif

    // recompile 2d/cubemap change in environment texture
    // XXX Possibly for equirect/fisheye change too, once this is implemented
    if ( (texture_->isEnabled() && texture_->checkCubeChanged())
      || (textureBump_->isEnabled() && textureBump_->checkCubeChanged())
      || (textureEnv_->isEnabled() && textureEnv_->checkCubeChanged())
       )
    {
        doRecompile_ = true;
    }

    if (nextGeometry_)
    {
        MO_DEBUG_MODEL("renderGl: assigning next geometry");

        auto g = nextGeometry_;
        nextGeometry_ = 0;
        draw_->setGeometry(g);
        setupDrawable_();
    }

    if (doRecompile_)
    {
        MO_DEBUG_MODEL("renderGl: recompiling shader");

        doRecompile_ = false;
        setupDrawable_();
    }


    Mat4 trans = transformation();
    Mat4 cubeViewTrans, viewTrans;
    if (fixPosition_->baseValue() == 0)
    {
        cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
        viewTrans = rs.cameraSpace().viewMatrix() * trans;
    }
    else
    {
        // skybox (clear position from matrices)

        trans[3] = Vec4(0., 0., 0., 1.);

        Mat4 vm = rs.cameraSpace().cubeViewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        cubeViewTrans = vm * trans;

        vm = rs.cameraSpace().viewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        viewTrans = vm * trans;
    }

    if (draw_->isReady())
    {
        MO_DEBUG_MODEL("renderGl: drawing");

        const int numDup = paramNumInstance_->value(time);

        // update uniforms
        const auto bright = cbright_->value(time);
        draw_->setAmbientColor(
                    cr_->value(time) * bright,
                    cg_->value(time) * bright,
                    cb_->value(time) * bright,
                    ca_->value(time));

        if (u_instance_count_)
            u_instance_count_->floats[0] = numDup;
        if (u_light_amt_)
            u_light_amt_->setFloats(
                        diffAmt_->value(time),
                        diffExp_->value(time),
                        specAmt_->value(time),
                        specExp_->value(time));
        if (u_bump_scale_)
            u_bump_scale_->floats[0] = bumpScale_->value(time);
        if (u_vertex_extrude_)
            u_vertex_extrude_->floats[0] = vertexExtrude_->value(time);
        if (u_env_map_amt_)
            u_env_map_amt_->setFloats(
                        envMapAmt_->value(time),
                        envMapAmt2_->value(time),
                        envMapAmt3_->value(time));        
        if (u_cam_pos_)
        {
            const Vec3& pos = rs.cameraSpace().position();
            u_cam_pos_->setFloats(pos.x, pos.y, pos.z, 0.);
        }

        // bind the model3d specific textures
        uint texSlot = 0;
        if (u_tex_0_ && texture_->isEnabled())
            { u_tex_0_->ints[0] = texSlot; texture_->bind(time, &texSlot); }
        if (u_texn_0_ && textureBump_->isEnabled())
            { u_texn_0_->ints[0] = texSlot; textureBump_->bind(time, &texSlot); }
        /** @bug cubetex here + a rect color or normal tex results
            in GL_INVALID_OPERATION in Drawable's drawElements. */
        if (u_tex_env_0_ && textureEnv_->isEnabled())
            { u_tex_env_0_->ints[0] = texSlot; textureEnv_->bind(time, &texSlot); }

        // update user uniforms and textures
        uniformSetting_->updateUniforms(time, &texSlot);

        GL::Texture::setActiveTexture(0);

        if (texture_->isEnabled())
        {
            if (texturePostProc_->isEnabled())
                texturePostProc_->updateUniforms(time);

            textureMorph_->updateUniforms(time);
        }

        if (textureBump_->isEnabled())
        {
            textureBumpMorph_->updateUniforms(time);
        }

        // draw state

        GL::Properties::staticInstance().setLineSmooth(paramLineSmooth_->value(time) != 0);
        GL::Properties::staticInstance().setPolygonSmooth(paramPolySmooth_->value(time) != 0);
        GL::Properties::staticInstance().setLineWidth(paramLineWidth_->value(time));
        GL::Properties::staticInstance().setPointSize(paramPointSize_->value(time));

        if (pointSizeAuto_->baseValue())
        {
            MO_CHECK_GL( gl::glEnable(gl::GL_PROGRAM_POINT_SIZE) );
            if (u_pointsize_)
            {
                float mi = paramPointSize_->value(time);
                u_pointsize_->setFloats(mi,
                                        paramPointSizeMax_->value(time) - mi,
                                        1.f / paramPointSizeDistFac_->value(time),
                                        0);
            }
        }
        else
            MO_CHECK_GL( gl::glDisable(gl::GL_PROGRAM_POINT_SIZE) );

        //MO_CHECK_GL( glPolygonMode(gl::GL_FRONT, gl::GL_LINE) );

        // render the thing

        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans,
                            viewTrans,
                            trans,
                            &rs.lightSettings(),
                            time.second(), numDup);

        //MO_CHECK_GL( glPolygonMode(gl::GL_FRONT, gl::GL_FILL) );

        // XXX disable here so we don't change any other painter's state
        if (paramLineSmooth_->baseValue())
            GL::Properties::staticInstance().setLineSmooth(false);
        if (paramPolySmooth_->baseValue())
            GL::Properties::staticInstance().setPolygonSmooth(false);
    }
    else
        MO_DEBUG_MODEL("renderGl: drawable not ready");
}





} // namespace MO
