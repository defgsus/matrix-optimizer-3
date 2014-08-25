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
#include "geom/geometrycreator.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "util/texturesetting.h"
#include "util/colorpostprocessingsetting.h"

namespace MO {

MO_REGISTER_OBJECT(Model3d)

Model3d::Model3d(QObject * parent)
    : ObjectGl      (parent),
      creator_      (0),
      geomSettings_ (new GEOM::GeometryFactorySettings),
      nextGeometry_ (0),
      texture_      (new TextureSetting(this)),
      textureBump_  (new TextureSetting(this)),
      texturePostProc_(new ColorPostProcessingSetting(this)),
      u_diff_exp_   (0),
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

    beginParameterGroup("shaderset", "shader settings");

        lightMode_ = createSelectParameter("lightmode", tr("lighting mode"),
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

    endParameterGroup();

    beginParameterGroup("color", tr("color"));

        cr_ = createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        cg_ = createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        cb_ = createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ca_ = createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    endParameterGroup();

    beginParameterGroup("texture", tr("texture"));

        texture_->createParameters("col", TextureSetting::TT_NONE, true);

    endParameterGroup();

    beginParameterGroup("texturepp", tr("texture post-processing"));

        texturePostProc_->createParameters("tex");

    endParameterGroup();

    beginParameterGroup("texturebump", tr("normal-map texture"));

        textureBump_->createParameters("bump", TextureSetting::TT_NONE, true);

    endParameterGroup();

    beginParameterGroup("surface", tr("surface"));

        diffExp_ = createFloatParameter("diffuseexp", tr("diffuse exponent"),
                                   tr("Exponent for the diffuse lighting - the higher, the narrower "
                                      "is the light cone"),
                                   4.0, 0.1);
        diffExp_->setMinValue(0.001);

    endParameterGroup();
}

void Model3d::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == lightMode_ || texturePostProc_->needsRecompile(p))
        doRecompile_ = true;

    if (texture_->needsReinit(p) || textureBump_->needsReinit(p))
        requestReinitGl();
}

void Model3d::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    texture_->updateParameterVisibility();
    textureBump_->updateParameterVisibility();

    diffExp_->setVisible( lightMode_->baseValue() != LM_NONE );
}

void Model3d::initGl(uint /*thread*/)
{
    texture_->initGl();
    textureBump_->initGl();

    draw_ = new GL::Drawable(idName());

    creator_ = new GEOM::GeometryCreator(this);
    connect(creator_, SIGNAL(succeeded()), this, SLOT(geometryCreated_()));
    connect(creator_, SIGNAL(failed(QString)), this, SLOT(geometryFailed_()));

    creator_->setSettings(*geomSettings_);
    creator_->start();
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

void Model3d::numberLightSourcesChanged(uint thread)
{
    doRecompile_ = isGlInitialized(thread);
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
    requestReinitGl();
}

void Model3d::setupDrawable_()
{
    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadVertexSource(":/shader/default.vert");
    src->loadFragmentSource(":/shader/default.frag");

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

    draw_->setShaderSource(src);

    draw_->createOpenGl();

    // get uniforms
    u_diff_exp_ = draw_->shader()->getUniform(src->uniformNameDiffuseExponent());
    if (texturePostProc_->isEnabled())
        texturePostProc_->getUniforms(draw_->shader());
}

void Model3d::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation(thread, 0);
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
        draw_->setAmbientColor(
                    cr_->value(time, thread),
                    cg_->value(time, thread),
                    cb_->value(time, thread),
                    ca_->value(time, thread));
        if (u_diff_exp_)
            u_diff_exp_->floats[0] = diffExp_->value(time, thread);
        if (texturePostProc_->isEnabled())
            texturePostProc_->updateUniforms(time, thread);

        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans, viewTrans, trans, &rs.lightSettings());
    }
}





} // namespace MO
