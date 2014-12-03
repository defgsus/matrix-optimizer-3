/** @file modulatoritem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#ifndef MODULATORITEM_H
#define MODULATORITEM_H

#include <QGraphicsItem>

namespace MO {
class Modulator;
namespace GUI {

class AbstractObjectItem;
class ObjectGraphScene;

class ModulatorItem : public QGraphicsItem
{
public:

    enum { Type = UserType + 10 };

    ModulatorItem(Modulator *);

    // ------------------- getter ----------------------

    int type() const { return Type; }

    /** Returns the ObjectGraphScene this item is in, or NULL */
    ObjectGraphScene * objectScene() const;

    // ------------------ setter -----------------------

    void updateShape();

    // ---------- QGraphicsItem interface --------------

    QPainterPath shape() const Q_DECL_OVERRIDE;
    QRectF boundingRect() const Q_DECL_OVERRIDE;

    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

    //void mousePressEvent(QGraphicsSceneMouseEvent*);

private:

    void updateFromTo_();
    void updatePos_();
    void calcShape_();

    Modulator * mod_;
    bool isHovered_;
    AbstractObjectItem * from_, * to_;
    QPointF fromPos_, toPos_;
    QRectF rect_;
    QPainterPath shape_, boundingShape_;
};

} // namespace GUI
} // namespace MO

#endif // MODULATORITEM_H
