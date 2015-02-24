/** @file abstractfrontitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H
#define MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H

#include <QGraphicsItem>
#include <QVariant>
#include <QMap>

class QStaticText;

namespace MO {

class Object;
class Parameter;
class Modulator;
class Properties;

namespace GUI {

enum FrontItemType
{
    FIT_ABSTRACT = QGraphicsItem::UserType + 1024,
    FIT_GROUP,
    FIT_FLOAT,
    FIT_TEXT
};

/** Base class for scene interface objects.
    This is basically a group with fancy css-like properties,
    which can be filled with other control items.

  Property ids are:
  (int) width, height, padding, border
  (QString) parameter-id, name
  (bool) show-label
  (QColor) border-color, background-color
*/
class AbstractFrontItem : public QGraphicsItem
{
public:

    explicit AbstractFrontItem(Parameter * p = 0, QGraphicsItem * parent = 0);
    ~AbstractFrontItem();

    // ---------------- getter ------------------

    /** Returns a pointer to the connected Parameter, or NULL if there is none. */
    Parameter * parameter() const { return p_param_; }

    /** Read access to properties */
    const Properties& properties() const { return *p_props_; }

    // -------------- setter --------------------

    void setProperties(const Properties&);
    void setProperty(const QString& id, const QVariant& v);

    // --------------- layout -------------------

    /** Rectangle of the whole area (including border) */
    QRectF rect() const;

    /** Rectangle of child area */
    QRectF innerRect() const;

    // ----------- QGraphicsItem ----------------

    enum { Type = FIT_ABSTRACT };
    virtual int type() const { return FIT_ABSTRACT; }

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    void p_update_from_properties_();

    Properties * p_props_;
    Parameter * p_param_;
    QStaticText * p_statictext_name_;
    QSizeF p_oldSize_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H
