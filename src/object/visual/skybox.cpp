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
        *paramDistance, *paramFadeMin, *paramFadeMax, *paramFadeExp,
        *paramOffsetX, *paramOffsetY,
        *paramScaleX, *paramScaleY,
        *paramPolySize;
    ParameterSelect
        *paramContentMode,
        *paramShapeMode,
        *paramAxis,
        *paramFade,
        *paramPoly;
    ParameterText
        *paramGlsl;

    GL::Uniform
        *u_cam_pos,
        *u_tex_color,
        *u_distance,
        *u_offset_scale,
        *u_fade_dist;
};

Skybox::Skybox()
    : ObjectGl      ()
    , p_            (new Private(this))

{
    setName("Skybox");
    initDefaultUpdateMode(UM_ALWAYS, false);
    initDefaultDepthWriteMode(DWM_OFF);
    initDefaultCullingMode(CM_NONE);
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

    params()->beginParameterGroup("geometry", tr("geometry"));

        p_->paramPoly = params()->createSelectParameter(
                "poly_type", tr("polygon"),
                tr("The polygon to use for drawing the skybox"),
                { "box", "ico" },
                { tr("box"), tr("icosahedron") },
                { tr("A classic cube"), tr("An icosahedron as fast approximation to a sphere") },
                { POLY_BOX, POLY_ICO },
                POLY_ICO, true, false);
        p_->paramPoly->setDefaultEvolvable(false);

        p_->paramPolySize = params()->createFloatParameter(
                    "poly_size", tr("size"),
                    tr("Expansion of the skybox polygon in scene units"),
                    200., 10.);
        p_->paramPolySize->setMinValue(1.);
        p_->paramPolySize->setDefaultEvolvable(false);

    params()->endParameterGroup();

    params()->beginParameterGroup("content", tr("content"));
    initParameterGroupExpanded("content");

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

        p_->paramAxis = params()->createSelectParameter(
                "axis", tr("axis"),
                tr("Selects the axis to which the shape is parallel or othogonal"),
                { "x+", "x-", "y+", "y-", "z+", "z-" },
                { tr("+X (right)"), tr("-X (left)"),
                  tr("+Y (up)"), tr("-Y (down)"),
                  tr("+Z (back)"), tr("-Z (front)") },
                { tr("Left"), tr("Right"), tr("Up"), tr("Down"), tr("Backward"), tr("Forward") },
                { A_POS_X, A_NEG_X, A_POS_Y, A_NEG_Y, A_POS_Z, A_NEG_Z },
                A_POS_Y, true, false);

        p_->paramDistance = params()->createFloatParameter(
                    "shape_distance", tr("distance"),
                    tr("Distance to shape"), 1.0, 0.1);
        p_->paramDistance->setMinValue(0.);

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
                    "// float u_time                // scene time in seconds\n"
                    "// mat4 u_projection;          // projection matrix\n"
                    "// mat4 u_cubeViewTransform;   // cube-map * view * transform\n"
                    "// mat4 u_viewTransform;       // view * transform\n"
                    "// mat4 u_transform;           // transformation only\n"
                    "// vec3 u_cam_pos       		// world position of camera\n"
                    "\n"
                    "// -- write to fragColor --\n"
                    "// dir = normalized camera ray\n"
                    "// pos = position on virtual shape surface\n"
                    "// uv = 2d texture coordinates on surface\n"
                    "void mainImage(out vec4 fragColor, in vec3 dir, in vec3 pos, in vec2 uv)\n"
                    "{\n"
                    "    // use uv for painting grid lines\n"
                    "    vec2 griduv = mod(uv * 4., 1.);\n"
                    "    vec2 grid = vec2(\n"
                    "            smoothstep(.05, .0, min(abs(griduv.x), abs(griduv.x-1.))),\n"
                    "            smoothstep(.05, .0, min(abs(griduv.y), abs(griduv.y-1.))) );\n"
                    "    vec3 col = vec3(1., .5, 0.) * grid.x\n"
                    "             + vec3(.3, .5, 1.) * grid.y;\n"
                    "    fragColor = vec4(col, 1.);\n"
                    "}\n"
                    );

        p_->paramOffsetX = params()->createFloatParameter(
                    "offset_x", tr("offset x"),
                    tr("Offsets the content on the x axis"),
                    0.0, 0.01, true, true);
        p_->paramOffsetY = params()->createFloatParameter(
                    "offset_y", tr("offset y"),
                    tr("Offsets the content on the y axis"),
                    0.0, 0.01, true, true);
        p_->paramScaleX = params()->createFloatParameter(
                    "scale_x", tr("scale x"),
                    tr("Scales the content on the x axis"),
                    1.0, 0.1, true, true);
        p_->paramScaleY = params()->createFloatParameter(
                    "scale_y", tr("scale y"),
                    tr("Scales the content on the y axis"),
                    1.0, 0.1, true, true);

        p_->textureSetting.createParameters("col", TextureSetting::TEX_NONE, true);
        p_->textureSetting.textureParam()->setWrapMode(ParameterTexture::WM_REPEAT);

    params()->endParameterGroup();



    params()->beginParameterGroup("color", tr("color"));
    initParameterGroupExpanded("color");

        p_->paramFade = params()->createBooleanParameter(
                    "fade_dist", tr("fade distance"),
                    tr("Enables fade-out of alpha channel with distance to virtual shape"),
                    tr("No fading"),
                    tr("Alpha channel is adjusted according to the distance to the virtual shape"),
                    true, true, false);

        p_->paramFadeMin = params()->createFloatParameter(
                    "fade_dist_min", tr("fade distance visible"),
                    tr("Distance to the virtual shape within the alpha channel remains unchanged"),
                    1.0, 0.1);
        p_->paramFadeMin->setMinValue(0.);

        p_->paramFadeMax = params()->createFloatParameter(
                    "fade_dist_max", tr("fade distance invisible"),
                    tr("Distance to the virtual shape after which alpha channel is cleared"),
                    30.0, .5);
        p_->paramFadeMax->setMinValue(0.);

        p_->paramFadeExp = params()->createFloatParameter(
                    "fade_dist_exp", tr("fade distance exponent"),
                    tr("Exponent of the transition between visible and invisible, "
                       "< 1 more visible, > 1 less visible"),
                    1.0, .1);
        p_->paramFadeExp->setMinValue(0.);


        p_->paramBright = params()->createFloatParameter("bright", tr("bright"), tr("Overall brightness of the color"), 1.0, 0.1);
        p_->paramR = params()->createFloatParameter("red", tr("red"), tr("Red amount of ambient color"), 1.0, 0.1);
        p_->paramG = params()->createFloatParameter("green", tr("green"), tr("Green amount of ambient color"), 1.0, 0.1);
        p_->paramB = params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of ambient color"), 1.0, 0.1);
        p_->paramA = params()->createFloatParameter("alpha", tr("alpha"), tr("Alpha amount of ambient color"), 1.0, 0.1);
        p_->paramA->setDefaultEvolvable(false);

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
            || p == p_->paramFade
            || p == p_->paramPoly
            || p == p_->paramPolySize
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

    p_->paramGlsl->setVisible(contentMode() == CM_GLSL);

    bool fade = p_->paramFade->baseValue();
    p_->paramFadeMin->setVisible(fade);
    p_->paramFadeMax->setVisible(fade);
    p_->paramFadeExp->setVisible(fade);
}

void Skybox::getNeededFiles(IO::FileList &files)
{
    ObjectGl::getNeededFiles(files);

    p_->textureSetting.getNeededFiles(files, IO::FT_TEXTURE);
}


Skybox::ContentMode Skybox::contentMode() const { return (ContentMode)p_->paramContentMode->baseValue(); }
Skybox::ShapeMode Skybox::shapeMode() const { return (ShapeMode)p_->paramShapeMode->baseValue(); }
Skybox::Axis Skybox::axis() const { return (Axis)p_->paramAxis->baseValue(); }
Skybox::PolyType Skybox::polyType() const { return (PolyType)p_->paramPoly->baseValue(); }

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


GL::ShaderSource Skybox::valueShaderSource(uint /*index*/) const
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

        QString decl;

        decl = QString("uniform %1 u_tex_color;")
                .arg(textureSetting.isCube() ? "samplerCube" : "sampler2D");
        decl += QString("uniform %1 iChannel0;")
                .arg(textureSetting.isCube() ? "samplerCube" : "sampler2D");
        src.replace("//%mo_user_uniforms%",
                    uniformSetting.getDeclarations() + "\n" + decl);

        decl = "#define SKYBOX_CONTENT SKYBOX_CONTENT_";
        switch (p->contentMode())
        {
            case CM_TEXTURE: decl += "TEXTURE"; break;
            case CM_GLSL: decl += "GLSL"; break;
        }
        decl += "\n#define SKYBOX_SHAPE SKYBOX_SHAPE_";
        switch (p->shapeMode())
        {
            case SM_SPHERE: decl += "SPHERE"; break;
            case SM_PLANE: decl += "PLANE"; break;
            case SM_CYLINDER: decl += "CYLINDER"; break;
        }
        decl += "\n#define SKYBOX_AXIS SKYBOX_AXIS_";
        switch (p->axis())
        {
            case A_POS_X: decl += "X_P"; break;
            case A_NEG_X: decl += "X_N"; break;
            case A_POS_Y: decl += "Y_P"; break;
            case A_NEG_Y: decl += "Y_N"; break;
            case A_POS_Z: decl += "Z_P"; break;
            case A_NEG_Z: decl += "Z_N"; break;
        }
        decl += "\n";
        if (paramFade->baseValue())
            decl += "#define SKYBOX_ENABLE_FADE\n";
        decl += QString("#define MO_NUM_LIGHTS %1\n").arg(p->numberLightSources());
        src.replace("//%mo_config_defines%", decl);

        // user code
        src.replace("//%mo_user_function%", paramGlsl->baseValue(), true);

        // resolve all includes
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
        Float size = paramPolySize->baseValue();
        auto geom = new GEOM::Geometry();
        switch (p->polyType())
        {
            case POLY_BOX: GEOM::GeometryFactory::createCube(geom, size); break;
            case POLY_ICO: GEOM::GeometryFactory::createIcosahedron(geom, size); break;
        }
        drawable->setShaderSource(getShaderSource());
        drawable->setGeometry(geom);

        drawable->createOpenGl();

        // get uniforms

        u_cam_pos = drawable->shader()->getUniform("u_cam_pos");
        u_tex_color = drawable->shader()->getUniform("u_tex_color");
        u_distance = drawable->shader()->getUniform("u_distance");
        u_fade_dist = drawable->shader()->getUniform("u_fade_dist");
        u_offset_scale = drawable->shader()->getUniform("u_offset_scale");

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
                    p_->paramFadeMax->value(time),
                    p_->paramFadeExp->value(time));

    if (p_->u_offset_scale)
        p_->u_offset_scale->setFloats(
                    p_->paramOffsetX->value(time),
                    p_->paramOffsetY->value(time),
                    p_->paramScaleX->value(time),
                    p_->paramScaleY->value(time));

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
