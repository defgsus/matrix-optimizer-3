/** @file objectgraphscene.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
#define MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H

#include <QGraphicsScene>

namespace MO {
class Object;
namespace GUI {

class AbstractObjectItem;

class ObjectGraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ObjectGraphScene(QObject *parent = 0);
    ~ObjectGraphScene();

    // -------------- getter -------------------

    /** Return the item for the object, or NULL */
    AbstractObjectItem * itemForObject(Object * o) const;

    /** Return the first visible item or one of it's parents for the object, or NULL */
    AbstractObjectItem * visibleItemForObject(Object * o) const;

    /** Translates a global point into grid coordinates */
    QPoint mapToGrid(const QPointF& global_pos) const;

signals:

    void shiftView(const QPointF& delta);

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Moves the given item to the given position */
    void setGridPos(AbstractObjectItem *, const QPoint& gridPos);

    /** Calls update for all ModulatorItems connected to the given item */
    void repaintModulators(AbstractObjectItem *);

    /** Bring the item and all it's cables to front */
    void toFront(AbstractObjectItem *);

    /** Creates the edit object, for scene or for item */
    void createEditActions(AbstractObjectItem * = 0);

    // ------------------- editing --------------------

    void addObject(Object * parent, Object * newObject, int insert_index = -1);

private slots:

    void onChanged_();

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    virtual void drawForeground(QPainter * p, const QRectF &rect) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
