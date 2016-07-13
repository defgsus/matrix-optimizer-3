/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#include <QLayout>

#include "soundfilewidget.h"
#include "gui/widget/soundfiledisplay.h"
#include "gui/ruler.h"

namespace MO {
namespace GUI {

struct SoundFileWidget::Private
{
    Private(SoundFileWidget* p)
        : p             (p)
    {

    }

    void createWidgets();

    SoundFileWidget* p;

    Ruler * vruler;
    SoundFileDisplay* display;
};

SoundFileWidget::SoundFileWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    p_->createWidgets();
}

SoundFileWidget::~SoundFileWidget()
{
    delete p_;
}

void SoundFileWidget::Private::createWidgets()
{
    auto lh = new QHBoxLayout(p);
    lh->setMargin(0);

        vruler = new Ruler(p);
        vruler->setOptions(Ruler::O_EnableAllY);
        vruler->setMaximumWidth(40);
        lh->addWidget(vruler);

        display = new SoundFileDisplay(p);
        display->setRulerOptions(Ruler::O_EnableAllX | Ruler::O_DrawY);
        lh->addWidget(display);

        vruler->setViewSpace(display->viewSpace());

    connect(vruler, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            display, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(display, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            vruler, SLOT(setViewSpace(UTIL::ViewSpace)));
}

void SoundFileWidget::setSoundFile(AUDIO::SoundFile* sf)
{
    p_->display->setSoundFile(sf);
    p_->display->fitToSoundFile();
}

} // namespace GUI
} // namespace MO
