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

#include "objectinfodialog.h"
#include "types/vector.h"
#include "io/streamoperators_qt.h"
#include "object/object.h"
#include "object/objectgl.h"
#include "object/transform/transformation.h"
#include "object/scene.h"
#include "object/control/clip.h"
#include "object/control/clipcontroller.h"
#include "object/audioobject.h"
#include "object/model3d.h"
#include "object/control/sequencefloat.h"
#include "object/control/modulatorobjectfloat.h"
#include "object/param/parameters.h"
#include "object/util/alphablendsetting.h"
#include "object/util/audioobjectconnections.h"
#include "geom/geometry.h"

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

    // ----- children -----

    s << "<p>" << tr("children objects") << ": " << o->numChildren(true) << "</p>";

    // ---- value -----

    if (auto seq = qobject_cast<SequenceFloat*>(o))
    {
        s << "<p>value at " << curTime << " sec: " << seq->value(curTime, MO_GUI_THREAD) << "</p>\n";
    }
    if (auto mod = qobject_cast<ModulatorObjectFloat*>(o))
    {
        s << "<p>value at " << curTime << " sec: " << mod->value(curTime, MO_GUI_THREAD) << "</p>\n";
    }

    // ----- modulators -----

    auto mods = o->params()->getModulatingObjects(false);
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

    if (ObjectGl * gl = qobject_cast<ObjectGl*>(o))
    {
        s << "<p>depth test: " << gl->depthTestModeNames[gl->depthTestMode()]
          << "<br/>depth write: " << gl->depthWriteModeNames[gl->depthWriteMode()]
          << "<br/>alpha blend: " << AlphaBlendSetting::modeNames[gl->alphaBlendMode()]
          << "</p>";
    }

    // ---------- matrix ---------------

    if (Transformation * tran = qobject_cast<Transformation*>(o))
    {
        Mat4 mat(1.0);
        tran->applyTransformation(mat, curTime, MO_GFX_THREAD);
        s << "<p>" << tr("applied transformation at %1").arg(curTime)
          << ":<br/>" << matrix2Html(mat) << "</p>";
    }
    else if (o->type() & Object::TG_REAL_OBJECT)
        s << "<p>" << tr("current transformation") << ":<br/>"
          << matrix2Html(o->transformation()) << "</p>";

    // ------- geometry ------------------

    if (Model3d * model = qobject_cast<Model3d*>(o))
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

    if (AudioObject * au = qobject_cast<AudioObject*>(o))
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

    if (ClipController * clipcon = qobject_cast<ClipController*>(o))
    {
        s << "<p>ClipContainer:<br/>size: "
          << clipcon->numberColumns() << "x"
          << clipcon->numberRows()
          << "</p>";
    }

    // ------------ clip ---------------

    if (Clip * clip = qobject_cast<Clip*>(o))
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

    s << "</html>";

    label_->setText(QString::fromStdString(s.str()));

}


} // namespace MO
} // namespace GUI
