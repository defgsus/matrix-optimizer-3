/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#include <vector>

#include <QImage>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QDropEvent>

#include "evolutionarea.h"
#include "tool/evolutionbase.h"
#include "tool/evolutionpool.h"
#include "io/log.h"

namespace MO {
namespace GUI {

struct EvolutionArea::Private
{
    Private(EvolutionArea * w)
        : widget    (w)
        , selTile   (-1)
    {
        resize(5);
        auto e = new EvolutionVectorBase(20);
        pool.setBaseType(e);
        e->releaseRef();
        pool.randomize();
    }

    void resize(unsigned numY);
    QPoint tilePos(unsigned idx) const
        { return numTiles.height() > 0
                ? QPoint((idx / numTiles.height()) * tileRes.width(),
                         (idx % numTiles.height()) * tileRes.height()) + paintOffs
                : QPoint(0, 0);  }
    QRect tileRect(unsigned idx) const
        { return QRect(tilePos(idx), tileRes); }
    void paint(QPainter&, const QRect&);

    EvolutionArea * widget;
    QSize tileRes, numTiles;
    QPoint paintOffs;

    EvolutionPool pool;
    int64_t selTile;
};

EvolutionArea::EvolutionArea(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

EvolutionArea::~EvolutionArea()
{
    delete p_;
}

unsigned EvolutionArea::numTiles() const { return p_->pool.size(); }
unsigned EvolutionArea::numTilesX() const { return p_->numTiles.width(); }
unsigned EvolutionArea::numTilesY() const { return p_->numTiles.height(); }
int EvolutionArea::selectedIndex() const { return p_->selTile; }
const EvolutionPool& EvolutionArea::pool() const { return p_->pool; }
EvolutionPool& EvolutionArea::pool() { return p_->pool; }

void EvolutionArea::setNumTilesY(unsigned n)
    { if (n != numTilesY()) { p_->resize(n); update(); } }
void EvolutionArea::updateTile(unsigned tileIdx) { update(p_->tileRect(tileIdx)); }
unsigned EvolutionArea::tileIndexAt(unsigned x, unsigned y) const
{
    x -= p_->paintOffs.x();
    y -= p_->paintOffs.y();
    if (p_->tileRes.isNull())
        return x * numTilesY() + y;
    return x / p_->tileRes.width() * numTilesY() + y / p_->tileRes.height();
}

void EvolutionArea::mousePressEvent(QMouseEvent* e)
{
    size_t sel = tileIndexAt(e->x(), e->y());

    // select tile
    if ((e->buttons() & Qt::LeftButton)
        && sel < p_->pool.size())
    {
        if (p_->selTile)
            updateTile(p_->selTile);
        p_->selTile = sel;
        updateTile(p_->selTile);
        emit selected(p_->selTile);
        e->accept();
        return;
    }

    QWidget::mousePressEvent(e);
}


void EvolutionArea::mouseMoveEvent(QMouseEvent* e)
{
    QWidget::mouseMoveEvent(e);
}


void EvolutionArea::mouseReleaseEvent(QMouseEvent* e)
{
    QWidget::mouseReleaseEvent(e);
}

void EvolutionArea::mouseDoubleClickEvent(QMouseEvent* e)
{
    size_t sel = tileIndexAt(e->x(), e->y());

    // mutate
    if ((e->buttons() & Qt::LeftButton)
        && sel < p_->pool.size())
    {
        p_->pool.repopulateFrom(sel);
        update();
    }
}

void EvolutionArea::dropEvent(QDropEvent* e)
{
    QWidget::dropEvent(e);
}

void EvolutionArea::resizeEvent(QResizeEvent*)
{
    if (numTilesY())
        p_->resize(numTilesY());
}

void EvolutionArea::paintEvent(QPaintEvent* e)
{
    QPainter paint(this);
    p_->paint(paint, e->rect());
}


void EvolutionArea::Private::resize(unsigned newNumY)
{    
    QSize s = widget->size();
    int newRes = s.height() / newNumY,
        newNumX = s.width() / newRes;
    QSize fitRes = QSize(newNumX * newRes, newNumY * newRes);

    // assign
    pool.resize(newNumX * newNumY);
    numTiles = QSize(newNumX, newNumY);
    tileRes = QSize(newRes, newRes);
    paintOffs = QPoint((s.width() - fitRes.width()) / 2,
                       (s.height() - fitRes.height()) / 2);
    pool.setImageResolution(tileRes);

    // keep selection in range
    if (selTile > 0 && size_t(selTile) >= pool.size())
        selTile = int64_t(pool.size()) - 1;

}

void EvolutionArea::Private::paint(QPainter& p, const QRect& rect)
{
    pool.renderTiles();

    p.fillRect(rect, Qt::black);

    for (size_t i=0; i<pool.size(); ++i)
    {
        auto trect = tileRect(i);
        if (!trect.intersects(rect))
            continue;

        p.drawImage(trect.topLeft(), pool.image(i));


        // --- border ---

        trect.adjust(0,0,-1,-1);

        p.setBrush(Qt::NoBrush);
        if (selTile == int64_t(i))
            p.setPen(QPen(QColor(140, 240, 140)));
        else
            p.setPen(QPen(QColor(40, 40, 40)));

        p.drawRect(trect);
    }
}



} // namespace GUI
} // namespace MO
