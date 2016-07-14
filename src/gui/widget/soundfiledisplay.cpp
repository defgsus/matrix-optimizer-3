/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "soundfiledisplay.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"
#include "gui/ruler.h"
#include "audio/tool/soundfile.h"

namespace MO {
namespace GUI {

struct SoundFileDisplay::Private
{
    Private(SoundFileDisplay * p)
        : p             (p)
        , viewspace     (0., -1., 1., 2.)
        , gridDraw      (new PAINTER::Grid(p))
        , valueDraw     (new PAINTER::ValueCurve(p))
        , soundFile     (nullptr)
        , rulerOptions  (Ruler::O_EnableAll)
        , doDragSpace   (false)
        , doInterpol    (false)
    {
        gridDraw->setOptions(
                    PAINTER::Grid::O_DrawAll);
        valueDraw->setCurveFunction([=](Double t)
        {
            return soundFile && t >= 0. ? soundFile->value(t, 0, doInterpol) : 0.;
        });
    }

    ~Private()
    {
        if (soundFile)
            soundFile->release();
    }

    QCursor defaultCursor() const
    {
        if (rulerOptions & Ruler::O_ChangeViewAll)
            return Qt::OpenHandCursor;
        else
            return Qt::ArrowCursor;
    }

    SoundFileDisplay* p;

    UTIL::ViewSpace viewspace;
    PAINTER::Grid * gridDraw;
    PAINTER::ValueCurve * valueDraw;
    AUDIO::SoundFile* soundFile;

    int rulerOptions;
    bool doDragSpace, doInterpol;
    QPoint dragStart, lastPos;
    UTIL::ViewSpace dragStartSpace;
};

SoundFileDisplay::SoundFileDisplay(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

SoundFileDisplay::~SoundFileDisplay()
{
    delete p_;
}

const UTIL::ViewSpace& SoundFileDisplay::viewSpace() const
{
    return p_->viewspace;
}

int SoundFileDisplay::rulerOptions() const { return p_->rulerOptions; }

AUDIO::SoundFile* SoundFileDisplay::soundFile() const { return p_->soundFile; }

void SoundFileDisplay::setViewSpace(const UTIL::ViewSpace& v)
{
    p_->viewspace = v;
    update();
}

void SoundFileDisplay::setSoundFile(AUDIO::SoundFile* s)
{
    if (s)
        s->addRef();
    if (p_->soundFile)
        p_->soundFile->release();
    p_->soundFile = s;
    update();
}

void SoundFileDisplay::fitToSoundFile()
{
    if (!p_->soundFile)
        return;

    setViewSpace(UTIL::ViewSpace(0., -1., p_->soundFile->lengthSeconds(), 2.));
    emit viewSpaceChanged(p_->viewspace);
}

void SoundFileDisplay::setRulerOptions(int options)
{
    bool changed = (p_->rulerOptions != options);
    p_->rulerOptions = options;

    p_->gridDraw->setOptions(
        (PAINTER::Grid::O_DrawX * ((p_->rulerOptions & Ruler::O_DrawX) != 0))
    |   (PAINTER::Grid::O_DrawY * ((p_->rulerOptions & Ruler::O_DrawY) != 0))
    |   (PAINTER::Grid::O_DrawTextX * ((p_->rulerOptions & Ruler::O_DrawTextX) != 0))
    |   (PAINTER::Grid::O_DrawTextY * ((p_->rulerOptions & Ruler::O_DrawTextY) != 0))
    );

    setCursor(p_->defaultCursor());

    if (changed)
        update();
}


void SoundFileDisplay::paintEvent(QPaintEvent* e)
{
    QPainter p(this);

    p.setBrush(QBrush(QColor(30,30,30)));
    p.setPen(Qt::NoPen);
    p.drawRect(e->rect());

    p_->gridDraw->setViewSpace(p_->viewspace);
    p_->gridDraw->paint(p, e->rect());

    p_->valueDraw->setViewSpace(p_->viewspace);
    p_->valueDraw->paint(p, e->rect());
}


void SoundFileDisplay::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton
        && (rulerOptions() & Ruler::O_DragAll))
    {
        p_->dragStart = p_->lastPos = e->pos();
        p_->dragStartSpace = p_->viewspace;
        p_->doDragSpace = true;
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        e->accept();
        return;
    }
}

void SoundFileDisplay::mouseMoveEvent(QMouseEvent * e)
{
    const Double zoomChange_ = 0.01;

    if (p_->doDragSpace)
    {
        bool changed = false;

        Double
            dx = p_->viewspace.mapXDistanceTo(p_->lastPos.x() - e->x()) / width(),
            dy = p_->viewspace.mapYDistanceTo(p_->lastPos.y() - e->y()) / height();

        if (rulerOptions() & Ruler::O_DragX)
        {
            p_->viewspace.addX( dx );
            changed = true;
        }
        else if (rulerOptions() & Ruler::O_ZoomY)
        {
            p_->viewspace.zoomY( 1.0 + zoomChange_ * (p_->lastPos.x() - e->x()),
                          (Double)e->y() / height() );
            changed = true;
        }

        if (rulerOptions() & Ruler::O_DragY)
        {
            p_->viewspace.addY( -dy );
            changed = true;
        }
        else if (rulerOptions() & Ruler::O_ZoomX)
        {
            p_->viewspace.zoomX( 1.0 + zoomChange_ * (p_->lastPos.y() - e->y()),
                          (Double)e->x() / width() );
            changed = true;
        }

        if (changed)
        {
            emit viewSpaceChanged(p_->viewspace);
            update();
        }

        p_->lastPos = e->pos();

        e->accept();
        return;
    }
}

void SoundFileDisplay::mouseReleaseEvent(QMouseEvent * )
{
    setCursor(p_->defaultCursor());
    p_->doDragSpace = false;
}

void SoundFileDisplay::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        const Double time = p_->viewspace.mapXTo((Double)e->x()/width());
        emit doubleClicked(time);
        e->accept();
        return;
    }
}

void SoundFileDisplay::wheelEvent(QWheelEvent * e)
{
    const Double zoomChange_ = e->delta() > 0 ? 0.1 : -0.1;
    bool changed = false;

    if (p_->rulerOptions & Ruler::O_ZoomY)
    {
        p_->viewspace.zoomY( 1.0 + zoomChange_, (Double)e->y() / height() );
        changed = true;
    }

    if (p_->rulerOptions & Ruler::O_ZoomX)
    {
        p_->viewspace.zoomX( 1.0 + zoomChange_, (Double)e->x() / width() );
        changed = true;
    }

    if (changed)
    {
        emit viewSpaceChanged(p_->viewspace);
        update();
        e->accept();
    }
}


} // namespace GUI
} // namespace MO
