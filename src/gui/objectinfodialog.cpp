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
#include "object/clip.h"
#include "object/clipcontainer.h"
#include "object/audioobject.h"
#include "object/util/alphablendsetting.h"
#include "object/util/audioobjectconnections.h"

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

    // ----- modulators -----

    auto mods = o->getModulatingObjects();
    if (!mods.isEmpty())
    {
        s << "<p>modulators:";
        for (auto mod : mods)
        {
            // once again, don't display sequence-on-track modulators
            if (mod->isSequence()
                    && mod->parentObject() && mod->parentObject()->isTrack())
                continue;
            s << "<br/>" << mod->idNamePath();
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

    // ---------- audio object -----------

    if (AudioObject * au = qobject_cast<AudioObject*>(o))
    {
        s << "<p>AudioObject:<br/>channels: "
          << au->numAudioInputs() << "/" << au->numAudioOutputs();
        if (scene && scene->audioConnections())
        {
            auto list = scene->audioConnections()->getInputs(au);
            for (auto c : list)
                s << "\n<br/>" << c->from()->name() << " ->";
            list = scene->audioConnections()->getOutputs(au);
            for (auto c : list)
                s << "\n<br/>-> " << c->to()->name();
        }
        s  << "</p>";
    }

    // ------- clip container -------------

    if (ClipContainer * clipcon = qobject_cast<ClipContainer*>(o))
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
