/** @file sequenceview.cpp

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/
//#include <QDebug>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>

#include "sequenceview.h"
#include "ruler.h"
#include "object/scene.h"
#include "object/sequence.h"
#include "io/error.h"
#include "doublespinbox.h"


namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget         (parent),
    baseSequence_   (0),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this)),
    settings_       (0),
    defaultSettingsAvailable_(false)
{
    grid_->setMargin(1);
    grid_->setContentsMargins(1,1,1,1);
    grid_->setSpacing(2);
    // make sequence the biggest component
    grid_->setColumnStretch(2, 10);

    //grid_->addWidget(timelineView_, 1, 1);
    grid_->addWidget(rulerX_, 0, 2);
    grid_->addWidget(rulerY_, 1, 1);

    // set ruler properties

    rulerX_->setFixedHeight(34);
    rulerX_->setOptions(Ruler::O_DragX | Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_ZoomX);

    rulerY_->setFixedWidth(60);
    rulerY_->setOptions(Ruler::O_DragY | Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_ZoomY);

    //timelineView_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    // connect rulers to class

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect rulers to each other

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // pass viewSpaceChanged to this classes signal

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    //connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    // ---- container for settings ----

    settings_ = new QScrollArea(this);
    settings_->setMinimumWidth(200);
    grid_->addWidget(settings_, 0, 0, 2, 1);

    auto w = new QWidget(settings_);
    settings_->setWidget(w);

    settingsLayout_ = new QVBoxLayout(w);
    settingsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // connect fit-request from ruler to timeline's fit-to-view
    /*connect(rulerX_, &Ruler::fitRequest, [=]()
    {
        if (timelineView_->options() & Timeline1DView::O_ZoomViewX)
        {
            if (timelineView_->isSelected())
                timelineView_->fitSelectionToView(true, false);
            else
                timelineView_->fitToView(true, false);
        }
    });
    connect(rulerY_, &Ruler::fitRequest, [=]()
    {
        if (timelineView_->options() & Timeline1DView::O_ZoomViewY)
        {
            if (timelineView_->isSelected())
                timelineView_->fitSelectionToView(false, true);
            else
                timelineView_->fitToView(false, true);
        }
    });

    // update rulers
    timelineView_->setViewSpace(timelineView_->viewSpace(), true);
    */

}

void SequenceView::setSequence_(Sequence * s)
{
    bool different = baseSequence_ != s;

    // remove old connection !!
    if (baseSequence_)
    {
        disconnect(baseSequence_, SIGNAL(timeChanged(MO::Sequence*)),
                   this, SLOT(sequenceTimeChanged(MO::Sequence*)));
    }

    baseSequence_ = s;

    if (different)
    {
        if (!baseSequence_)
            clearDefaultSettingsWidgets_();
        else
        {
            createDefaultSettingsWidgets_();
            connect(baseSequence_, SIGNAL(timeChanged(MO::Sequence*)),
                       SLOT(sequenceTimeChanged(MO::Sequence*)));
        }
    }
}

void SequenceView::updateViewSpace_(const UTIL::ViewSpace & v)
{
    rulerX_->setViewSpace(v);
    rulerY_->setViewSpace(v);
}


void SequenceView::setSequenceWidget_(QWidget * w)
{
    grid_->addWidget(w, 1, 2);
}


QWidget * SequenceView::newContainer_(const QString& name)
{
    auto w = new QWidget(this);
    w->setObjectName("_" + name);

    auto l = new QHBoxLayout(w);
    l->setMargin(0);

    l->addWidget(new QLabel(name, w));

    return w;
}

QWidget * SequenceView::newSetting(const QString & name)
{
    auto w = newContainer_(name);
    addSettingsWidget_(w);

    return w;
}

QWidget * SequenceView::newDefaultSetting_(const QString & name)
{
    auto w = newContainer_(name);
    settingsLayout_->addWidget(w);
    defaultSettingsWidgets_.append(w);

    return w;
}


void SequenceView::clearSettingsWidgets_()
{
    for (auto w : customSettingsWidgets_)
        w->deleteLater();

    customSettingsWidgets_.clear();
}

void SequenceView::addSettingsWidget_(QWidget * w)
{
    settingsLayout_->addWidget(w);
    customSettingsWidgets_.append(w);
}

void SequenceView::clearDefaultSettingsWidgets_()
{
    defaultSettingsAvailable_ = false;

    for (auto w : defaultSettingsWidgets_)
        w->deleteLater();

    defaultSettingsWidgets_.clear();
}

void SequenceView::createDefaultSettingsWidgets_()
{
    clearDefaultSettingsWidgets_();

    if (!baseSequence_)
        return;

    QWidget * w;

    Scene * scene = baseSequence_->sceneObject();
    MO_ASSERT(scene, "no scene for Sequence in SequenceView");


#define MO__SCENE_PARAM(spin__, getter__, setter__, desc__) \
    w = newDefaultSetting_(desc__);                         \
    w->layout()->addWidget(spin__ = new DoubleSpinBox(w));  \
    spin__->setMinimum(0);                                  \
    spin__->setMaximum(60*60 * 1000);                       \
    spin__->setValue(baseSequence_->getter__());            \
    connect(spin__,                                         \
               &DoubleSpinBox::valueChanged, [=](Double v)  \
    {                                                       \
        scene->beginSequenceChange(baseSequence_);          \
        baseSequence_->setter__(v);                         \
        scene->endSequenceChange();                         \
    });

    MO__SCENE_PARAM(spinStart_, start, setStart, tr("start time"));
    MO__SCENE_PARAM(spinLength_, length, setLength, tr("length"));
    MO__SCENE_PARAM(spinEnd_, end, setEnd, tr("end time"));
    MO__SCENE_PARAM(spinLoopStart_, loopStart, setLoopStart, tr("loop start"));
    MO__SCENE_PARAM(spinLoopLength_, loopLength, setLoopLength, tr("loop length"));
    MO__SCENE_PARAM(spinLoopEnd_, loopEnd, setLoopEnd, tr("loop end"));

#undef MO__SCENE_PARAM

    // hline below
    auto f = new QFrame(this);
    f->setFrameShape(QFrame::HLine);
    settingsLayout_->addWidget(f);
    defaultSettingsWidgets_.append(f);

    defaultSettingsAvailable_ = true;
}

void SequenceView::sequenceTimeChanged(Sequence * s)
{
//    qDebug() << "seqchanged " << s << baseSequence_ << defaultSettingsAvailable_;
    if (!defaultSettingsAvailable_ || !baseSequence_
        || s != baseSequence_)
        return;
//    qDebug() << "updating";
    spinStart_->setValue( baseSequence_->start() );
    spinLength_->setValue( baseSequence_->length() );
    spinEnd_->setValue( baseSequence_->end() );
    spinLoopStart_->setValue( baseSequence_->loopStart() );
    spinLoopLength_->setValue( baseSequence_->loopLength() );
    spinLoopEnd_->setValue( baseSequence_->loopEnd() );

}

} // namespace GUI
} // namespace MO
