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
#include "object/audio/audiounit.h"
#include "object/clip.h"

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

    Double curTime = 0.0;
    if (Scene * s = o->sceneObject())
        curTime = s->sceneTime();

    std::stringstream s, s1;
    s << "<html><b>" << o->infoName() << "</b><br/>"
      << o->idNamePath() << "/" << o->idName() << "<br/>";

    s << "<p>" << tr("children objects") << ": " << o->numChildren(true) << "</p>";

    // ----- audiosources and microphones -----

    if (!o->microphones().isEmpty() || !o->audioSources().isEmpty())
    {
        s << "<p>";
        if (!o->audioSources().isEmpty())
            s << "audio sources: " << o->audioSources().size() << "<br/>";
        if (!o->microphones().isEmpty())
            s << "microphones: " << o->microphones().size();
        s << "</p>";
    }

    // ---------- render modes ---------

    if (ObjectGl * gl = qobject_cast<ObjectGl*>(o))
    {
        s << "<p>depth test: " << gl->depthTestModeNames[gl->depthTestMode()]
          << "<br/>depth write: " << gl->depthWriteModeNames[gl->depthWriteMode()]
          << "<br/>alpha blend: " << gl->alphaBlendModeNames[gl->alphaBlendMode()]
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
          << matrix2Html(o->transformation(MO_GFX_THREAD, 0)) << "</p>";

    // ---------- audio unit -----------

    if (AudioUnit * au = qobject_cast<AudioUnit*>(o))
    {
        s << "<p>AudioUnit:<br/>channels: "
          << au->numChannelsIn() << "/" << au->numChannelsOut()
          << "<br/>max. channels: " << au->maxChannelsIn() << "/" << au->maxChannelsOut()
          << "<br/>buffer size (per thread): ";
        for (uint i=0; i<au->numberThreads(); ++i)
        {
            if (i > 0)
                s << " / ";
            s << au->bufferSize(i);
        }
        s  << "</p>";
    }

    // ------------ clip ---------------

    if (Clip * clip = qobject_cast<Clip*>(o))
    {
        s << "<p>Contained sequences: "
          << clip->sequences().size()
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
