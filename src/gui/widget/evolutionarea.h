/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_EVOLUTIONAREA_H
#define MOSRC_GUI_WIDGET_EVOLUTIONAREA_H

#include <QWidget>

class QMenu;
namespace MO {
class EvolutionBase;
class EvolutionPool;
namespace GUI {

/** Tile renderer */
class EvolutionArea : public QWidget
{
    Q_OBJECT
public:
    explicit EvolutionArea(QWidget *parent = 0);
    ~EvolutionArea();

    unsigned numTiles() const;
    unsigned numTilesX() const;
    unsigned numTilesY() const;

    /** Resolution of each tile */
    const QSize& tileResolution() const;

    unsigned tileIndexAt(unsigned widgetX, unsigned widgetY) const;

    int selectedIndex() const;
    EvolutionBase* selectedSpecimen() const;

    const EvolutionPool& pool() const;
    EvolutionPool& pool();

    /** Creates an edit menu for the given tile */
    QMenu* createMenu(unsigned tileIndex);

signals:

    /** A tile was clicked */
    void selected(unsigned index);

public slots:

    /** Sets number of tiles that must fit on y-axis */
    void setNumTilesY(unsigned tilesY);

    /** Open a window to render the given tile */
    void showBig(unsigned idx);
    void showText(unsigned idx);
    void saveJson(unsigned idx);
    void saveJson(unsigned idx, const QString& fn);
    void saveImage(unsigned idx, const QSize& res);
    void saveImage(unsigned idx, const QSize& res, const QString& fn);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void dropEvent(QDropEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;

    /** Like update() for the area of the tile */
    void updateTile(unsigned tileIdx);

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // EVOLUTIONAREA_H
