/** @file objectinfodialog.cpp

    @brief Object information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/23/2014</p>
*/

#include <sstream>

#include <QLayout>
#include <QLabel>
#include <QPushButton>

#include "ObjectInfoDialog.h"
#include "types/vector.h"
#include "gl/Texture.h"
#include "io/streamoperators_qt.h"
#include "object/Object.h"
#include "object/Scene.h"
#include "object/AudioObject.h"
#include "object/transform/Transformation.h"
#include "object/control/Clip.h"
#include "object/control/ClipController.h"
#include "object/control/SequenceFloat.h"
#include "object/control/ModulatorObjectFloat.h"
#include "object/visual/ObjectGl.h"
#include "object/visual/Model3d.h"
#include "object/visual/ShaderObject.h"
#include "object/texture/TextureObjectBase.h"
#include "object/param/Parameters.h"
#include "object/util/AlphaBlendSetting.h"
#include "object/util/AudioObjectConnections.h"
#include "object/interface/ValueFloatInterface.h"
#include "object/interface/ValueFloatMatrixInterface.h"
#include "object/interface/ValueGeometryInterface.h"
#include "geom/Geometry.h"

namespace MO {
namespace GUI {



ObjectInfoDialog::ObjectInfoDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags)
{
    setMinimumSize(400, 300);

    auto lv = new QVBoxLayout(this);

        label_ = new QLabel(this);
        lv->addWidget(label_);

        auto but = new QPushButton(tr("Close"), this);
        lv->addWidget(but);
        but->setMaximumWidth(100);//setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        connect(but, SIGNAL(clicked()), this, SLOT(close()));
}


QString matrix2Html(const Mat4& mat)
{
    std::stringstream s;
    s << mat;
    QString tmp = QString::fromStdString(s.str()).toHtmlEscaped();
    tmp.replace('\n', "<br/>");
    return tmp;
}

void ObjectInfoDialog::setObject(Object * o)
{
    setWindowTitle(o->name());

    Scene * scene = o->sceneObject();

    Double curTime = 0.0;
    if (scene)
        curTime = scene->sceneTime();

    std::stringstream s, s1;
    s << "<html><b>" << o->infoName() << "</b><br/>"
      << o->idNamePath() << "<br/>";

    // ----- error -----

    if (!o->errorString().isEmpty())
    {
        s << "<p>" << tr("Error: ") << o->errorString() << "</p>\n";
    }

    // ----- children -----

    s << "<p>" << tr("children objects") << ": " << o->numChildren(true) << "</p>\n";


    // ---- float value -----

    auto outputMap = o->getNumberOutputs();

    if (auto fi = dynamic_cast<ValueFloatInterface*>(o))
    {
        s << "<p>float values at " << curTime << " sec: ";
        size_t num = outputMap[ST_FLOAT];
        for (size_t i = 0; i < num; ++i)
            s << "<br/>" << o->getOutputName(ST_FLOAT, i) << ": "
              << fi->valueFloat(i, RenderTime(curTime, MO_GUI_THREAD))
              << "\n";
        s << "</p>\n";
    }

    // ---- float matrix value -----

    if (auto fmi = dynamic_cast<ValueFloatMatrixInterface*>(o))
    {
        s << "<p>float-matrix values at " << curTime << " sec: ";
        size_t num = outputMap[ST_FLOAT_MATRIX];
        for (size_t i = 0; i < num; ++i)
            s << "<br/>" << o->getOutputName(ST_FLOAT_MATRIX, i) << ": "
              << fmi->valueFloatMatrix(i,
                RenderTime(curTime, MO_GUI_THREAD)).layoutString() << "\n";
        s << "</p>\n";
    }

    // ---- geometry value -----

    if (auto gi = dynamic_cast<ValueGeometryInterface*>(o))
    {
        s << "<p>geometry values at " << curTime << " sec: ";
        size_t num = outputMap[ST_GEOMETRY];
        for (size_t i = 0; i < num; ++i)
        {
            s << "<br/>" << o->getOutputName(ST_GEOMETRY, i) << ": ";
            auto geom = gi->valueGeometry(i, RenderTime(curTime, MO_GFX_THREAD));
            if (!geom)
                s << "null";
            else
                s << geom->infoString();
        }
        s << "</p>\n";
    }

    // ----- modulators -----

    auto mods = o->params()->getModulatingObjectsList(false);
    if (!mods.isEmpty())
    {
        s << "<p>modulators:";
        for (auto mod : mods)
        {
            // once again, don't display sequence-on-track modulators
            if (mod->isSequence()
                    && mod->parentObject() && mod->parentObject()->isTrack())
                continue;
            s << "<br/>" << mod->idNamePath() << " (" << mod->name() << ")";
        }
        s << "</p>";
    }

    // ----- audiosources and microphones -----

    if (o->numberMicrophones() || o->numberSoundSources())
    {
        s << "<p>";
        if (o->numberSoundSources())
            s << "audio sources: " << o->numberSoundSources() << "<br/>";
        if (o->numberMicrophones())
            s << "microphones: " << o->numberMicrophones();
        s << "</p>";
    }

    // ---------- render modes ---------

    if (ObjectGl * gl = dynamic_cast<ObjectGl*>(o))
    if (gl->isRenderSettingsEnabled())
    {
        s << "<p>render count: " << gl->renderCount()
          << "<br/>depth test: " << gl->depthTestModeNames[gl->depthTestMode()]
          << "<br/>depth write: " << gl->depthWriteModeNames[gl->depthWriteMode()]
          << "<br/>alpha blend: " << AlphaBlendSetting::modeNames[gl->alphaBlendMode()]
          << "</p>";
    }

    // ---------- texture outputs -----------

    if (ValueTextureInterface * ti = dynamic_cast<ValueTextureInterface*>(o))
    {
        s << "<p>textures:<ul>";
        for (uint chan = 0; chan<32; ++chan)
        {
            const GL::Texture * t = ti->valueTexture(chan, RenderTime(curTime, MO_GUI_THREAD));
            if (t)
                s << "<li>" << t->infoString() << "</li>";
        }
        s << "</ul></p>";
    }

    // ---------- matrix ---------------
    if (Transformation * tran = dynamic_cast<Transformation*>(o))
    {
        Mat4 mat(1.0);
        tran->applyTransformation(mat, RenderTime(curTime, MO_GUI_THREAD));
        s << "<p>" << tr("applied transformation at %1").arg(curTime)
          << ":<br/>" << matrix2Html(mat) << "</p>";
    }
    else if (o->type() & Object::TG_REAL_OBJECT)
        s << "<p>" << tr("current transformation") << ":<br/>"
          << matrix2Html(o->transformation()) << "</p>";

    // ------- geometry ------------------

    if (Model3d * model = dynamic_cast<Model3d*>(o))
    {
        s << "<p>" << tr("geometry") << ": ";
        if (!model->geometry())
            s << "null";
        else
        {
            const GEOM::Geometry * geom = model->geometry();
            s << "<br/>" << "vertices:  " << geom->numVertices()
              << "<br/>" << "lines:     " << geom->numLines()
              << "<br/>" << "triangles: " << geom->numTriangles()
              << "<br/>" << "user attributes:";
            const QStringList list = geom->getAttributeNames();
            if (list.isEmpty())
                s << " none";
            else
                for (auto & n : list)
                    s << " " << n;
        }
        s << "</p>";
    }

    // ---------- audio object -----------
    if (AudioObject * au = dynamic_cast<AudioObject*>(o))
    {
        s << "<p>AudioObject:<br/>channels: "
          << au->numAudioInputs() << "/" << au->numAudioOutputs()
          << "<ul>";
        if (scene && scene->audioConnections())
        {
            auto list = scene->audioConnections()->getInputs(au);
            for (auto c : list)
                s << "\n<li>" << c->from()->name()
                  << " -&gt; " << c->to()->getAudioInputName(c->inputChannel())
                  << "</li>";

            list = scene->audioConnections()->getOutputs(au);
            for (auto c : list)
                s << "\n<li>"
                  << c->from()->getAudioOutputName(c->outputChannel())
                  << " -&gt; " << c->to()->name()
                  << "</li>";
        }
        s  << "</ul></p>";
    }

    // ------- clip container -------------

    if (ClipController * clipcon = dynamic_cast<ClipController*>(o))
    {
        s << "<p>ClipContainer:<br/>size: "
          << clipcon->numberColumns() << "x"
          << clipcon->numberRows()
          << "</p>";
    }

    // ------------ clip ---------------

    if (Clip * clip = dynamic_cast<Clip*>(o))
    {
        s << "<p>Clip:<br/>position: " << clip->column() << ", " << clip->row()
          << "<br/>Contained sequences: " << clip->sequences().size()
          << "<br/>playing: ";
        if (clip->isPlaying())
            s << "since " << clip->timeStarted();
        else
            s << "no";
        s << "</p>";
    }

    auto infoStr = o->infoString();
    if (!infoStr.isEmpty())
        s << "<p>" << infoStr << "</p>";


    s << "</html>";

    label_->setText(QString::fromStdString(s.str()));

}


} // namespace MO
} // namespace GUI
