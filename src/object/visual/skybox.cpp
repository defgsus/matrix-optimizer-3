/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/24/2016</p>
*/

#include "skybox.h"
#include "object/util/useruniformsetting.h"
#include "object/util/texturesetting.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parametertexture.h"
#include "gl/drawable.h"
#include "gl/shadersource.h"
#include "gl/shader.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "io/datastream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(Skybox)

struct Skybox::Private
{
    Private(Skybox*p)
        : p             (p)
        , drawable      (0)
        , textureSetting(p)
        , uniformSetting(p)
        , doRecompile   (true)
    {
        delete drawable;
    }

    GL::ShaderSource getShaderSource() const;
    bool updateGl();

    Skybox* p;
    GL::Drawable *drawable;
    TextureSetting
            textureSetting;
    UserUniformSetting
            uniformSetting;
    bool doRecompile;
    ParameterFloat
        *paramR, *paramG, *paramB, *paramA, *paramBright,
        *paramDistance, *paramFadeMin, *paramFadeMax;
    ParameterSelect
        *paramContentMode,
        *paramShapeMode,
        *paramAxis;
    ParameterText
        *paramGlsl,
        *paramGlsl2;

    GL::Uniform
        *u_cam_pos,
        *u_tex_color,
        *u_distance,
        *u_fade_dist;
};

Skybox::Skybox(QObject *parent)
    : ObjectGl  (parent)
    , p_        (new Private(this))

{
    setName("Skybox");
    initDefaultUpdateMode(UM_ALWAYS, false);
}

Skybox::~Skybox()
{
    delete p_;
}


void Skybox::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("skybox", 1);

    p_->uniformSetting.serialize(io);
    p_->textureSetting.serialize(io);
}

void Skybox::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    /*const int ver = */io.readHeader("skybox", 1);

    p_->uniformSetting.deserialize(io);
    p_->textureSetting.deserialize(io);
}

void Skybox::createParameters()
{
    ObjectGl::createParameters();


    params()->beginParameterGroup("useruniforms", tr("user uniforms"));

        p_->uniformSetting.createParameters("g");

    params()->endParameterGroup();

    params()->beginParameterGroup("content", tr("content"));

        p_->paramShapeMode = params()->createSelectParameter(
                "shape_mode", tr("shape"),
                tr("Selects the shape that is projected on the sky"),
                { "sphere", "plane", "cylinder" },
                { tr("sphere"), tr("plane"), tr("cylinder") },
                { tr("A full sphere around the scene"),
                  tr("An infinite plane"),
                  tr("An infinite cylinder") },
                { SM_SPHERE, SM_PLANE, SM_CYLINDER },
                SM_SPHERE, true, false);

        p_->paramDistance = params()->createFloatParameter(
                    "shape_distance", tr("distance"),
                    tr("Distance to shape"), 1.0, 0.1);
        p_->paramDistance->setMinValue(0.);

        p_->paramAxis = params()->createSelectParameter(
                "axis", tr("axis"),
                tr("Selects the axis to which the shape is parallel or othogonal"),
                { "x+", "x-", "y+", "y-", "z+", "z-" },
                { tr("+X"), tr("-X"), tr("+Y"), tr("-Y"), tr("+Z"), tr("-Z") },
                { tr("Left"), tr("Right"), tr("Up"), tr("Down"), tr("Backward"), tr("Forward") },
                { A_POS_X, A_NEG_X, A_POS_Y, A_NEG_Y, A_POS_Z, A_NEG_Z },
                A_POS_Y, true, false);

        p_->paramContentMode = params()->createSelectParameter(
                "content_mode", tr("content"),
                tr("Selects the type of image or function that is projected on the sky"),
                { "tex", "glsl" },
                { tr("texture"), tr("glsl") },
                { tr("Input texture"), tr("Free control via GLSL code") },
                { CM_TEXTURE, CM_GLSL },
                CM_GLSL, true, false);

        p_->paramGlsl = params()->createTextParameter(
                    "glsl_code", tr("glsl code"),
                    tr("A user-defined glsl function to generate the output color"),
                    TT_GLSL,
                    "// -- uniforms --\n"
                    "void mainImage(out vec4 fragColor, in vec3 direction)\n"
                    "{\n"
                    "\tfragColor = vec4(0.2, 0.5, 1., 1.);\n"
                    "}\n");

        p_->paramGlsl2 = params()->createTextParameter(
                    "glsl_code_2", tr("glsl code"),
                    tr("A user-defined glsl function to generate the output color"),
                    TT_GLSL,
                    "// -- uniforms --\n"
                    "void mainImage(out vec4 fragColor, in vec2 uv, in vec3 direction)\n"
                    "{\n"
                    "\tfragColor = vec4(mod(uv.x, 1.), mod(uv.y, 1.), 0., 1.);\n"
                    "}\n");


        p_->textureSetting.createParameters("col", TextureSetting::TEX_NONE, true);
        p_->textureSetting.textureParam()->setWrapMode(ParameterTexture::WM_REPEAT);

    params()->endParameterGroup();



    params()->beginParameterGroup("color", tr("color"));
    initParameterGroupExpanded("color");

        p_->paramBright = params()->createFloatParameter("bright", tr("bright"), tr("Overall brightness of the color"), 1.0, 0.1);
        p_->paramR = params()->createFloatParameter("red", tr("red"), tr("Red amount of ambient color"), 1.0, 0.1);
        p_->paramG = params()->createFloatParameter("green", tr("green"), tr("Green amount of ambient color"), 1.0, 0.1);
        p_->paramB = params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of ambient color"), 1.0, 0.1);
        p_->paramA = params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha amount of ambient color"), 1.0, 0.1);
        p_->paramA->setDefaultEvolvable(false);

        p_->paramFadeMin = params()->createFloatParameter(
                    "fade_min_dist", tr("fade distance visible"),
                    tr("Distance to the virtual shape within the alpha channel remains unchanged"),
                    1.0, 0.1);
        p_->paramFadeMin->setMinValue(0.);

        p_->paramFadeMax = params()->createFloatParameter(
                    "fade_max_dist", tr("fade distance invisible"),
                    tr("Distance to the virtual shape after which alpha channel is cleared"),
                    30.0, .5);
        p_->paramFadeMax->setMinValue(0.);

    params()->endParameterGroup();


}

void Skybox::onParametersLoaded()
{
    ObjectGl::onParametersLoaded();
}

void Skybox::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);

    if (p == p_->paramContentMode
            || p == p_->paramGlsl
            || p == p_->paramContentMode
            || p == p_->paramShapeMode
            || p == p_->paramAxis
            || p_->textureSetting.needsReinit(p)
            || p_->uniformSetting.needsReinit(p) )
    {
        p_->doRecompile = true;
        requestRender();
    }
}

void Skybox::updateParameterVisibility()
{
    ObjectGl::updateParameterVisibility();

    p_->textureSetting.updateParameterVisibility();
    p_->uniformSetting.updateParameterVisibility();
}

void Skybox::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    p_->textureSetting.getNeededFiles(files, IO::FT_TEXTURE);
}


void Skybox::initGl(uint /*thread*/)
{
    p_->updateGl();
}

void Skybox::releaseGl(uint /*thread*/)
{
    p_->textureSetting.releaseGl();

    if (p_->drawable && p_->drawable->isCreated())
        p_->drawable->releaseOpenGl();
    delete p_->drawable;
    p_->drawable = 0;
}


void Skybox::numberLightSourcesChanged(uint /*thread*/)
{
    p_->doRecompile = true;
}


GL::ShaderSource Skybox::shaderSource() const
{
    return p_->getShaderSource();
}

GL::ShaderSource Skybox::Private::getShaderSource() const
{
    try
    {
        GL::ShaderSource src;
        src.loadVertexSource(":/shader/default.vert");
        src.loadFragmentSource(":/shader/skybox.frag");

        QString decl = QString("uniform %1 u_tex_color;")
                .arg(textureSetting.isCube() ? "samplerCube" : "sampler2D");
        src.replace("//%mo_user_uniforms%", uniformSetting.getDeclarations() + "\n" + decl);
        src.replace("//%mo_user_function%", paramGlsl->baseValue(), true);

        decl = "#define SKYBOX_CONTENT SKYBOX_CONTENT_";
        switch (paramContentMode->baseValue())
        {
            case CM_TEXTURE: decl += "TEXTURE"; break;
            case CM_GLSL: decl += "GLSL"; break;
        }
        decl += "\n#define SKYBOX_SHAPE SKYBOX_SHAPE_";
        switch (paramShapeMode->baseValue())
        {
            case SM_SPHERE: decl += "SPHERE"; break;
            case SM_PLANE: decl += "PLANE"; break;
            case SM_CYLINDER: decl += "CYLINDER"; break;
        }
        decl += "\n#define SKYBOX_AXIS SKYBOX_AXIS_";
        switch (paramAxis->baseValue())
        {
            case A_POS_X: decl += "X_P"; break;
            case A_NEG_X: decl += "X_N"; break;
            case A_POS_Y: decl += "Y_P"; break;
            case A_NEG_Y: decl += "Y_N"; break;
            case A_POS_Z: decl += "Z_P"; break;
            case A_NEG_Z: decl += "Z_N"; break;
        }
        decl += "\n";
        src.replace("//%mo_config_defines%", decl);


        // resolve includes
        src.replaceIncludes([this](const QString& url, bool do_search)
        {
            return p->getGlslInclude(url, do_search);
        });

        return src;
    }
    catch (const Exception& e)
    {
        p->setErrorMessage(e.what());
        return GL::ShaderSource();
    }
}

bool Skybox::Private::updateGl()
{
    bool valid = false;
    try
    {
        // load/create/querry textures
        textureSetting.initGl();
        valid = true;
    }
    catch (const Exception&)
    {
        p->setErrorMessage(textureSetting.errorString());
    }

    if (drawable && drawable->isCreated())
        drawable->releaseOpenGl();
    delete drawable;

    drawable = new GL::Drawable("skybox_geometry");

    try
    {
        auto geom = new GEOM::Geometry();
        GEOM::GeometryFactory::createIcosahedron(geom, 100.f);

        drawable->setShaderSource(getShaderSource());
        drawable->setGeometry(geom);

        drawable->createOpenGl();

        // get uniforms

        u_cam_pos = drawable->shader()->getUniform("u_cam_pos");
        u_tex_color = drawable->shader()->getUniform("u_tex_color");
        u_distance = drawable->shader()->getUniform("u_distance");
        u_fade_dist = drawable->shader()->getUniform("u_fade_dist");

        uniformSetting.tieToShader(drawable->shader());

        doRecompile = false;
        if (valid)
            p->clearError();
        return true;
    }
    catch (const Exception& e)
    {
        for (const GL::Shader::CompileMessage& msg : drawable->shader()->compileMessages())
        {
            if (msg.program == GL::Shader::P_FRAGMENT
                || msg.program == GL::Shader::P_LINKER)
            {
                paramGlsl->addErrorMessage(msg.line, msg.text);
            }
        }

        p->setErrorMessage(e.what());

        if (drawable && drawable->isCreated())
            drawable->releaseOpenGl();
        delete drawable;
        drawable = 0;
        return false;
    }
}

void Skybox::renderGl(const GL::RenderSettings &rs, const RenderTime &time)
{
    if ((p_->doRecompile && !p_->updateGl())
        || !(p_->drawable && p_->drawable->isReady()))
        return;

    Mat4 trans = transformation();
    Mat4 cubeViewTrans, viewTrans;

    // --- skybox (clear position from camera matrices) ---
    {
        trans[3] = Vec4(0., 0., 0., 1.);

        Mat4 vm = rs.cameraSpace().cubeViewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        cubeViewTrans = vm * trans;

        vm = rs.cameraSpace().viewMatrix();
        vm[3] = Vec4(0., 0., 0, 1.);
        viewTrans = vm * trans;
    }

    // --- update uniforms ---

    const Float bright = p_->paramBright->value(time);
    p_->drawable->setAmbientColor(
                p_->paramR->value(time) * bright,
                p_->paramG->value(time) * bright,
                p_->paramB->value(time) * bright,
                p_->paramA->value(time));

    if (p_->u_cam_pos)
    {
        const Vec3& pos = rs.cameraSpace().position();
        p_->u_cam_pos->setFloats(pos.x, pos.y, pos.z, 0.);
    }

    if (p_->u_distance)
        p_->u_distance->setFloats(
                    p_->paramDistance->value(time));

    if (p_->u_fade_dist)
        p_->u_fade_dist->setFloats(
                    p_->paramFadeMin->value(time),
                    p_->paramFadeMax->value(time));

    // --- bind textures ---

    uint texSlot = 0;
    p_->uniformSetting.updateUniforms(time, texSlot);

    // bind the object specific textures
    if (p_->u_tex_color) { p_->textureSetting.bind(time, texSlot); p_->u_tex_color->ints[0] = texSlot++; }

    p_->drawable->renderShader(
                rs.cameraSpace().projectionMatrix(),
                cubeViewTrans,
                viewTrans,
                trans,
                &rs.lightSettings(),
                time.second());

}

} // namespace MO
