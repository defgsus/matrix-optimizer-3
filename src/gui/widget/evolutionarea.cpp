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
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

#include "evolutionarea.h"
#include "gui/evolutionbaseviewer.h"
#include "gui/texteditdialog.h"
#include "tool/evolutionbase.h"
#include "tool/evolutionpool.h"
#include "model/evolutionmimedata.h"
#include "tool/commonresolutions.h"
#include "io/files.h"
#include "io/error.h"
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

        auto e = new KaliSetEvolution();
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
EvolutionBase* EvolutionArea::selectedSpecimen() const
    { return p_->selTile > 0 && (size_t)p_->selTile < p_->pool.size() ? p_->pool.specimen(p_->selTile) : nullptr; }
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
    if ((e->buttons() & (Qt::LeftButton | Qt::RightButton))
        && sel < p_->pool.size())
    {
        if (p_->selTile)
            updateTile(p_->selTile);
        p_->selTile = sel;
        updateTile(p_->selTile);
        emit selected(p_->selTile);

        // right-click popup
        if (e->buttons() & Qt::RightButton)
        {
            auto menu = createMenu(sel);
            if (menu)
                menu->popup(QCursor::pos());
        }

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
        QPen pen;
        if (selTile == int64_t(i))
            pen.setColor(QColor(140, 240, 140));
        else
            pen.setColor(QColor(40, 40, 40));
        if (pool.isLocked(i))
        {
            pen.setColor(pen.color().lighter(200));
            pen.setWidth(3);
        }

        p.setPen(pen);
        p.drawRect(trect.adjusted(pen.width()/2,pen.width()/2,-pen.width()/2,-pen.width()/2));
    }
}

QMenu* EvolutionArea::createMenu(unsigned idx)
{
    if (idx >= p_->pool.size())
        return nullptr;

    auto menu = new QMenu(this);
    QAction* a;
    if (p_->pool.isLocked(idx))
    {
        a = menu->addAction(tr("Unlock"));
        connect(a, &QAction::triggered, [=](){ p_->pool.setLocked(idx, false); updateTile(idx); });
        a->setCheckable(true);
        a->setChecked(true);
    }
    else
    {
        a = menu->addAction(tr("Lock"));
        connect(a, &QAction::triggered, [=](){ p_->pool.setLocked(idx, true); updateTile(idx); });
    }

    menu->addSeparator();

    a = menu->addAction(tr("Show big"));
    connect(a, &QAction::triggered, [=](){ showBig(idx); });

    a = menu->addAction(tr("Show text"));
    connect(a, &QAction::triggered, [=](){ showText(idx); });

    menu->addSeparator();

    // copy
    a = menu->addAction(tr("Copy"));
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    connect(a, &QAction::triggered, [=]()
    {
        if (auto evo = selectedSpecimen())
        {
            auto md = new EvolutionMimeData();
            md->setSpecimen(evo);
            QApplication::clipboard()->setMimeData(md);
        }
    });

    // paste
    if (EvolutionMimeData::isInClipboard())
    {
        a = menu->addAction(tr("Paste"));
        a->setShortcut(Qt::CTRL + Qt::Key_V);
        connect(a, &QAction::triggered, [=]()
        {
            auto md = EvolutionMimeData::evolutionMimeData(
                        QApplication::clipboard()->mimeData());
            try
            {
                if (auto evo = md->createSpecimen())
                {
                    p_->pool.setSpecimen(idx, evo);
                    evo->releaseRef();
                    updateTile(idx);
                }
            }
            catch (const Exception& e)
            {
                QMessageBox::critical(this, tr("paste evolution error"),
                                      e.what());
            }
        });
    }

    menu->addSeparator();

    a = menu->addAction(tr("Save json"));
    connect(a, &QAction::triggered, [=](){ saveJson(idx); });

    auto sub = menu->addMenu(tr("Save image"));
    CommonResolutions::addResolutionActions(sub);
    connect(sub, &QMenu::triggered, [=](QAction* a)
    {
        saveImage(idx, CommonResolutions::resolutions[a->data().toInt()].size());
    });



    return menu;
}

void EvolutionArea::showBig(unsigned idx)
{
    if (idx >= p_->pool.size()
        || p_->pool.specimen(idx) == nullptr)
        return;

    auto win = new EvolutionBaseViewer(this);
    win->setAttribute(Qt::WA_DeleteOnClose);

    win->setSpecimen(p_->pool.specimen(idx));

    win->show();
}

void EvolutionArea::showText(unsigned idx)
{
    if (idx >= p_->pool.size()
        || p_->pool.specimen(idx) == nullptr)
        return;

    auto win = new TextEditDialog(TT_PLAIN_TEXT, this);
    win->setAttribute(Qt::WA_DeleteOnClose);

    win->setReadOnly(true);
    win->setText(p_->pool.specimen(idx)->toString());

    win->show();
}

void EvolutionArea::saveJson(unsigned idx)
{
    auto fn = IO::Files::getSaveFileName(IO::FT_EVOLUTION, this);
    if (!fn.isEmpty())
        saveJson(idx, fn);
}

void EvolutionArea::saveImage(unsigned idx, const QSize& s)
{
    auto fn = IO::Files::getSaveFileName(IO::FT_TEXTURE, this);
    if (!fn.isEmpty())
        saveImage(idx, s, fn);
}

void EvolutionArea::saveJson(unsigned idx, const QString& fn)
{
    if (idx >= p_->pool.size() || p_->pool.specimen(idx) == nullptr)
        return;

    try
    {
        p_->pool.specimen(idx)->saveJsonFile(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("saving json failed"), e.what());
    }
}

void EvolutionArea::saveImage(unsigned idx, const QSize& s, const QString& fn)
{
    if (idx >= p_->pool.size() || p_->pool.specimen(idx) == nullptr)
        return;

    try
    {
        QImage img(s, QImage::Format_ARGB32_Premultiplied);
        p_->pool.specimen(idx)->getImage(img);
        p_->pool.specimen(idx)->saveImage(fn, img);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("saving image failed"), e.what());
    }
}

} // namespace GUI
} // namespace MO
