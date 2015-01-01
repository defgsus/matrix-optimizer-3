/** @file model3d.cpp

    @brief Generic Drawable Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "model3d.h"
#include "io/datastream.h"
#include "gl/drawable.h"
#include "geom/geometryfactorysettings.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "geom/geometry.h"
#include "geom/geometrycreator.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "util/texturesetting.h"
#include "util/colorpostprocessingsetting.h"
#include "util/texturemorphsetting.h"

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    : ObjectGl      (parent),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings(this)),
      nextGeometry_ (0),
      texture_      (new TextureSetting(this)),
      textureBump_  (new TextureSetting(this)),
      texturePostProc_(new ColorPostProcessingSetting(this)),
      textureMorph_ (new TextureMorphSetting(this)),
      textureBumpMorph_(new TextureMorphSetting(this)),
      u_diff_exp_   (0),
      u_bump_scale_ (0),
      u_vertex_extrude_(0),
      doRecompile_  (false)
{
    setName("Model3D");
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

    params()->endParameterGroup();

    params()->beginParameterGroup("color", tr("color"));

        cbright_ = params()->createFloatParameter("bright", "bright", tr("Overall brightness of the color"), 1.0, 0.1);
        cr_ = params()->createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        cg_ = params()->createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        cb_ = params()->createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ca_ = params()->createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("surface", tr("surface"));

        diffExp_ = params()->createFloatParameter("diffuseexp", tr("diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower "
                                      "is the light cone"),
                                   4.0, 0.1);
        diffExp_->setMinValue(0.001);

    params()->endParameterGroup();

    params()->beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col", TextureSetting::TT_NONE, true);

    params()->endParameterGroup();

    params()->beginParameterGroup("texturepp", tr("texture post-processing"));

        texturePostProc_->createParameters("tex");

        textureMorph_->createParameters("tex");

    params()->endParameterGroup();

    params()->beginParameterGroup("texturebump", tr("normal-map texture"));

        textureBump_->createParameters("bump", TextureSetting::TT_NONE, true, true);

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

        glslVertex_ = params()->createTextParameter("glslvertex", tr("glsl function"),
                                                    tr("A piece of glsl code to modify vertex positions"),
                                                    TT_GLSL,
                                                    "vec3 mo_modify_position(in vec3 pos) {\n\treturn pos;\n}\n"
                                                    , true, false);
    params()->endParameterGroup();
}

void Model3d::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == lightMode_
            || p == vertexFx_
            || p == glslVertex_
            || texturePostProc_->needsRecompile(p)
            || textureMorph_->needsRecompile(p)
            || textureBumpMorph_->needsRecompile(p))
    {
        doRecompile_ = true;
        requestRender();
    }

    if (texture_->needsReinit(p) || textureBump_->needsReinit(p))
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

    diffExp_->setVisible( lightMode_->baseValue() != LM_NONE );

    bool vertfx = vertexFx_->baseValue();
    vertexExtrude_->setVisible(vertfx);
    glslVertex_->setVisible(vertfx);
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
    texture_->initGl();
    textureBump_->initGl();

    draw_ = new GL::Drawable(idName());

    if (!nextGeometry_)
    {
        creator_ = new GEOM::GeometryCreator(this);
        connect(creator_, SIGNAL(succeeded()), this, SLOT(geometryCreated_()));
        connect(creator_, SIGNAL(failed(QString)), this, SLOT(geometryFailed_()));

        geomSettings_->setObject(this);
        creator_->setSettings(*geomSettings_);
        creator_->start();
    }
}

void Model3d::releaseGl(uint /*thread*/)
{
    texture_->releaseGl();
    textureBump_->releaseGl();

    if (draw_->isReady())
        draw_->releaseOpenGl();

    delete draw_;
    draw_ = 0;

    delete creator_;
    creator_ = 0;
}

void Model3d::numberLightSourcesChanged(uint /*thread*/)
{
    doRecompile_ = true;
}

void Model3d::geometryCreated_()
{
    nextGeometry_ = creator_->takeGeometry();
    creator_->deleteLater();
    creator_ = 0;

    requestRender();
}

void Model3d::geometryFailed_()
{
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
    if (vertexFx_->baseValue())
    {
        src->addDefine("#define MO_ENABLE_VERTEX_EFFECTS");
        src->replace("%mo_modify_position%", glslVertex_->value());
    }
    draw_->setShaderSource(src);

    draw_->createOpenGl();

    // get uniforms
    u_diff_exp_ = draw_->shader()->getUniform(src->uniformNameDiffuseExponent());

    const bool isvertfx = vertexFx_->baseValue();
    u_vertex_extrude_ = draw_->shader()->getUniform("u_vertex_extrude", isvertfx);

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

}

void Model3d::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation();
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    if (nextGeometry_)
    {
        draw_->setGeometry(nextGeometry_);
        setupDrawable_();
        nextGeometry_ = 0;
    }

    if (doRecompile_)
    {
        doRecompile_ = false;
        setupDrawable_();
    }

    if (draw_->isReady())
    {
        // bind textures
        texture_->bind();
        textureBump_->bind(1);

        // update uniforms
        const auto bright = cbright_->value(time, thread);
        draw_->setAmbientColor(
                    cr_->value(time, thread) * bright,
                    cg_->value(time, thread) * bright,
                    cb_->value(time, thread) * bright,
                    ca_->value(time, thread));

        if (u_diff_exp_)
            u_diff_exp_->floats[0] = diffExp_->value(time, thread);
        if (u_bump_scale_)
            u_bump_scale_->floats[0] = bumpScale_->value(time, thread);
        if (u_vertex_extrude_)
            u_vertex_extrude_->floats[0] = vertexExtrude_->value(time, thread);

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

        // render the thing

        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans, viewTrans, trans, &rs.lightSettings(),
                            time);
    }
}





} // namespace MO
