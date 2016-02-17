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
#include "io/log.h"

namespace MO {
namespace GUI {

struct EvolutionArea::Private
{
    Private(EvolutionArea * w)
        : widget    (w)
    {
        resize(5);
    }

    struct Tile
    {
        Tile() : instance(0) { }
        ~Tile() { if (instance) instance->releaseRef(); }

        EvolutionBase* instance;
        QImage image;
    };

    void resize(unsigned numY);
    QPoint tilePos(unsigned idx) const
        { return numTiles.height() > 0
                ? QPoint((idx / numTiles.height()) * tileRes.width(),
                         (idx % numTiles.height()) * tileRes.height()) + paintOffs
                : QPoint(0, 0);  }
    QRect tileRect(unsigned idx) const
        { return QRect(tilePos(idx), tileRes); }
    void paint(QPainter&, const QRect&) const;
    void renderTile(Tile*) const;

    EvolutionArea * widget;
    QSize tileRes, numTiles;
    QPoint paintOffs;

    std::vector<Tile> tiles;
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

unsigned EvolutionArea::numTiles() const { return p_->tiles.size(); }
unsigned EvolutionArea::numTilesX() const { return p_->numTiles.width(); }
unsigned EvolutionArea::numTilesY() const { return p_->numTiles.height(); }

void EvolutionArea::setNumTilesY(unsigned n) { p_->resize(n); update(); }
void EvolutionArea::updateTile(unsigned tileIdx) { update(p_->tileRect(tileIdx)); }

void EvolutionArea::mousePressEvent(QMouseEvent* e)
{
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


void EvolutionArea::dropEvent(QDropEvent* e)
{
    QWidget::dropEvent(e);
}

void EvolutionArea::resizeEvent(QResizeEvent*)
{
    if (numTilesY())
        setNumTilesY(numTilesY());
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
    paintOffs = QPoint((s.width() - fitRes.width()) / 2,
                       (s.height() - fitRes.height()) / 2);

    tiles.resize(newNumY * newNumX);
    numTiles = QSize(newNumX, newNumY);
    tileRes = QSize(newRes, newRes);

    for (auto& t : tiles)
        renderTile(&t);
}

void EvolutionArea::setTile(unsigned tileIdx, EvolutionBase *evo)
{
    if (tileIdx >= p_->tiles.size())
        return;

    if (p_->tiles[tileIdx].instance)
        p_->tiles[tileIdx].instance->releaseRef();
    p_->tiles[tileIdx].instance = evo;
    evo->addRef();

    updateTile(tileIdx);
}

void EvolutionArea::Private::paint(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, Qt::black);

    for (size_t i=0; i<tiles.size(); ++i)
    {
        auto trect = tileRect(i);
        if (!trect.intersects(rect))
            continue;

        p.drawImage(trect.topLeft(), tiles[i].image);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(40, 40, 40)));

        p.drawRect(trect);
    }
}

void EvolutionArea::Private::renderTile(Tile* tile) const
{
    if (tile->image.size() != tileRes)
    {
        tile->image = QImage(tileRes, QImage::Format_ARGB32_Premultiplied);
        tile->image.fill(Qt::red);
    }

    if (tile->instance)
        tile->instance->getImage(tile->image);
}


} // namespace GUI
} // namespace MO
