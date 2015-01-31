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
namespace GUI {

enum FrontItemType
{
    FIT_ABSTRACT = QGraphicsItem::UserType + 1024,
    FIT_GROUP,
    FIT_FLOAT,
    FIT_TEXT
};

/**

  Property ids are:
  (int) width, height, padding, border
  (QString) parameter-id, name
  (bool) show-label
  (QColor) border-color, background-color
*/
class AbstractFrontItem : public QGraphicsItem
{
public:

    enum { Type = FIT_ABSTRACT };

    AbstractFrontItem(FrontItemType type, Parameter * p = 0, QGraphicsItem * parent = 0);

    // ---------------- getter ------------------

    int type() const { return FIT_ABSTRACT; }

    /** Returns a pointer to the connected Parameter, or NULL if there is none. */
    Parameter * parameter() const { return p_param_; }

    /** Returns the given property, or an invalid QVariant */
    QVariant getProperty(const QString& id) const;

    /** Returns the given property, or the default value. */
    QVariant getProperty(const QString& id, const QVariant& def) const;

    /** Returns true, when there is a property named @p id */
    bool hasProperty(const QString& id) const { return p_props_.contains(id); }

    // --------------- setter -------------------

    /** Sets the given property */
    void setProperty(const QString& id, const QVariant&);

    // --------------- layout -------------------

    QRectF rect() const;

    // ----------- QGraphicsItem ----------------

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    void p_update_from_properties_();

    FrontItemType p_type_;
    Parameter * p_param_;
    QStaticText * p_statictext_name_;
    QMap<QString, QVariant> p_props_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H
