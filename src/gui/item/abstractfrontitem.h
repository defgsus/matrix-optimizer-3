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
namespace IO { class XmlStream; }
namespace GUI {

class FrontScene;

/** The different types of Interface Items.
    XXX Not ready yet */
enum FrontItemType
{
    FIT_ABSTRACT = QGraphicsItem::UserType + 1024,
    FIT_GROUP,
    FIT_FLOAT,
    FIT_TEXT
};

/** Base class for scene interface objects.
    This is basically a group with fancy css-like properties,
    that can be filled with other control items.
*/
class AbstractFrontItem : public QGraphicsItem
{
public:

    explicit AbstractFrontItem(Parameter * p = 0, QGraphicsItem * parent = 0);
    ~AbstractFrontItem();

    // ------------------ io --------------------

    /** Writes the item to the xml stream.
        The stream is expected to be writeable.
        The section "interface-item" is created.
        The section on return is the same as on entry. */
    void serialize(IO::XmlStream&) const;
    /** Creates an item from the xml stream.
        The stream is expected to be readable
        and the current section must be "interface-item".
        The section on return is the same as on entry. */
    static AbstractFrontItem * deserialize(IO::XmlStream&);

    // ---------------- getter ------------------

    /** Returns a pointer to the connected Parameter, or NULL if there is none. */
    Parameter * parameter() const { return p_param_; }

    /** Returns the parent FrontScene, or NULL */
    FrontScene * frontScene() const;

    /** Read access to properties */
    const Properties& properties() const { return *p_props_; }

    QColor borderColor() const;
    QColor backgroundColor() const;

    /** A name for the property editor */
    QString name() const;

    // -------------- setter --------------------

    // ----- replic. of Properties interface ----

    void setProperties(const Properties&);
    void setProperty(const QString& id, const QVariant& v);
    template <class T> void setProperty(const QString& id, const T& v) { setProperty(id, QVariant::fromValue(v)); }
    void changeProperty(const QString& id, const QVariant& v);
    template <class T> void changeProperty(const QString& id, const T& v) { changeProperty(id, QVariant::fromValue(v)); }
protected:
    /** Same as setProperty() but does not call updateFromProperties().
        Should be used in constructor of derived classes. */
    void initProperty(const QString& id, const QVariant& v);
    template <class T> void initProperty(const QString& id, const T& v) { initProperty(id, QVariant::fromValue(v)); }
public:

    // --------------- layout -------------------

    /** Rectangle of the whole area (including border) */
    QRectF rect() const;

    /** Rectangle of child area */
    QRectF innerRect() const;

    /** Sets the inside rectangle's size. */
    void setSize(const QSizeF&);

    /** Like setPos() but with anchor at topLeft of innerRect() */
    void setInsidePos(const QPointF&);

    /** Enables editing and disables functionality */
    void setEditMode(bool);
    /** Returns true when layout editing is enabled for this item. */
    bool editMode() const { return p_editMode_; }

    // ------------------ tree ------------------

    /** All direct children that are AbstractFrontItem castable */
    QList<AbstractFrontItem*> childFrontItems() const;

    // ---------- virtual interface -------------
protected:
    /** Called whenever the Properties have changed.
        Do all necessary changes here. */
    virtual void onPropertiesChanged() { }

    /** Called whenever the edit mode turned on or off */
    virtual void onEditModeChanged() { }

public:
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
    QRectF p_oldRect_;
    QRectF p_labelRect_;
    bool p_prop_changed_,
         p_editMode_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H
