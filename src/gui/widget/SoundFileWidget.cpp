/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#include <QLayout>
#include <QList>

#include "SoundFileWidget.h"
#include "gui/widget/SoundFileDisplay.h"
#include "gui/Ruler.h"

namespace MO {
namespace GUI {

struct SoundFileWidget::Private
{
    Private(SoundFileWidget* p)
        : p             (p)
    {

    }

    struct Display
    {
        Ruler * vruler;
        SoundFileDisplay* display;
        ~Display()
        {
            vruler->deleteLater();
            display->deleteLater();
        }
    };

    void createWidgets();
    Display* createDisplay();

    SoundFileWidget* p;

    QGridLayout* layout;
    QList<Display*> displays;
};

SoundFileWidget::SoundFileWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    p_->layout = new QGridLayout(this);
    p_->layout->setMargin(0);
}

SoundFileWidget::~SoundFileWidget()
{
    delete p_;
}

SoundFileWidget::Private::Display* SoundFileWidget::Private::createDisplay()
{
    auto d = new Display;

    d->vruler = new Ruler(p);
    d->vruler->setOptions(Ruler::O_EnableAllY);
    d->vruler->setMaximumWidth(40);

    d->display = new SoundFileDisplay(p);
    d->display->setRulerOptions(Ruler::O_EnableAllX | Ruler::O_DrawY);

    d->vruler->setViewSpace(d->display->viewSpace());

    connect(d->vruler, &Ruler::viewSpaceChanged,
            [=](const UTIL::ViewSpace& v)
    {
        auto vs = d->display->viewSpace();
        vs.setY(v.y());
        vs.setScaleY(v.scaleY());
        d->display->setViewSpace(vs);
    });

    connect(d->display, &SoundFileDisplay::viewSpaceChanged,
            [=](const UTIL::ViewSpace& v)
    {
        for (auto dis : displays)
        if (d != dis)
        {
            auto vs = dis->display->viewSpace();
            vs.setX(v.x());
            vs.setScaleX(v.scaleX());
            dis->display->setViewSpace(vs);
        }
    });

    connect(d->display, &SoundFileDisplay::doubleClicked,
            [=](Double time)
    {
        emit p->doubleClicked(d->display->soundFile(), time);
    });

    return d;
}

AUDIO::SoundFile* SoundFileWidget::soundFile(int idx) const
{
    if (idx < 0 || idx >= p_->displays.size())
        return nullptr;
    return p_->displays[idx]->display->soundFile();
}

void SoundFileWidget::clear()
{
    for (auto d : p_->displays)
        delete d;

    p_->displays.clear();
}

void SoundFileWidget::setSoundFile(AUDIO::SoundFile* sf, int idx)
{
    if (idx < 0)
        return;
    if (idx >= p_->displays.size())
    {
        auto d = p_->createDisplay();
        p_->layout->addWidget(d->vruler, p_->displays.size(), 0);
        p_->layout->addWidget(d->display, p_->displays.size(), 1);
        p_->displays.append(d);
        d->display->setSoundFile(sf);
        d->display->fitToSoundFile();
    }
    else
    {
        p_->displays[idx]->display->setSoundFile(sf);
    }
}

} // namespace GUI
} // namespace MO
