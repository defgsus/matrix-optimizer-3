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
#include <QMimeData>

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

class AbstractFrontItem;

/** Wrapper around QMimeData to store instances of
    AbstractFrontItem for drag/drop */
class FrontItemMimeData : public QMimeData
{
public:
    FrontItemMimeData() : QMimeData() { }

    /** The mimedata string for an item id */
    static const QString ItemIdMimeType;
    /** The mimedata string for the item pointer. */
    static const QString ItemPtrMimeType;
    /** The mimedata string for the application pointer. */
    static const QString AppPtrMimeType;

    /** Returns the instance of this class, if QMimeData is convertible. */
    static FrontItemMimeData * frontItemMimeData(QMimeData*);
    static const FrontItemMimeData * frontItemMimeData(const QMimeData*);

    // --------- getter -----------

    /** Returns the stored item, or NULL */
    AbstractFrontItem * getItem() const;

    /** Returns the stored AbstractFrontItem::idName() */
    QString getItemId() const;

    bool isSameApplicationInstance() const;

    // --------- setter -----------

    /** Stores the item (id/ptr) */
    void setItem(AbstractFrontItem*);

};



/** Call this in your derived .cpp to make the class known to the factory */
#define MO_REGISTER_FRONT_ITEM(Class__) \
    namespace { static const bool havereg##Class_ = AbstractFrontItem::registerFrontItem(new Class__); }

/** Base class for scene interface objects.
    This is basically a group with fancy css-like properties,
    that can be filled with other control items.
*/
class AbstractFrontItem : public QGraphicsItem
{
public:

    explicit AbstractFrontItem(QGraphicsItem * parent = 0);
    ~AbstractFrontItem();

    // ----------------- factory ----------------

    /** Returns a fresh instance of the known class, or NULL
        @see className() and MO_REGISTER_FRONT_ITEM() */
    static AbstractFrontItem * factory(const QString& className);

    /** Use MO_REGISTER_FRONT_ITEM() instead */
    static bool registerFrontItem(AbstractFrontItem*);

    // ------------------ io --------------------

    /** Writes the item to the xml stream.
        The stream is expected to be writeable.
        The section "interface-item" is created.
        The section on return is the same as on entry. */
    void serialize(IO::XmlStream&) const;
    /** Creates an item from the xml stream.
        The stream is expected to be readable
        and the current section must be "interface-item".
        The section on return is the same as on entry.
        @returns The new item, or NULL for an unknown class.
        @throws IoException on any other errors. */
    static AbstractFrontItem * deserialize(IO::XmlStream&);

    // ---------------- getter ------------------

    /* Returns a pointer to the connected Parameter, or NULL if there is none. */
    //Parameter * parameter() const { return p_param_; }

    /** Returns the parent FrontScene, or NULL */
    FrontScene * frontScene() const;

    /** Read access to properties */
    const Properties& properties() const { return *p_props_; }

    QColor borderColor() const;
    QColor backgroundColor() const;

    /** A name for the property editor */
    QString name() const;

    /** Unique id to describe the item */
    QString idName() const { return p_id_; }

    // -------------- setter --------------------

    /** Sets a new idName() for this item.
        The id will be unique for the current session/project. */
    void setNewId();

    // ----- replic. of Properties interface ----

    void setProperties(const Properties&);
    void setProperty(const QString& idName, const QVariant& v);
    template <class T> void setProperty(const QString& id, const T& v) { setProperty(id, QVariant::fromValue(v)); }
    void changeProperty(const QString& idName, const QVariant& v);
    template <class T> void changeProperty(const QString& id, const T& v) { changeProperty(id, QVariant::fromValue(v)); }
protected:
    /** Same as setProperty() but does not call updateFromProperties().
        Should be used in constructor of derived classes. */
    void initProperty(const QString& idName, const QVariant& v);
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
    bool isEditMode() const { return p_editMode_; }

    // ------------------ tree ------------------

    /** All direct children that are AbstractFrontItem castable */
    QList<AbstractFrontItem*> childFrontItems() const;

    /** Returns true when @p parent is a parent of this item or any
        of it's parents. */
    bool hasParent(const QGraphicsItem* parent) const;

    /** Returns the parent if it's an AbstractFrontItem
        (which it should be). */
    AbstractFrontItem * parentFrontItem() const;

    /** Returns true if this item is allowed to group other items.
        Default value is false.
        @see setCanHaveChildren() */
    bool canHaveChildren() const { return p_canHaveChildren_; }

    /** Enables or disables the use of this item as a group. */
    void setCanHaveChildren(bool e) { p_canHaveChildren_ = e; }

    // ----------------- editing ----------------

    /** Starts a QDrag action with the item's id */
    void startDragging();

    /** Creates a pixmap from the item as it looks right now. */
    QPixmap getPixmap(uint max_size);

    // ---------- virtual interface -------------
protected:

    /** Reimplement to supply a persistent class name for the derived item. */
    virtual const QString& className() const = 0;

    /** Reimplement to return a new instance of your derived item. */
    virtual AbstractFrontItem * cloneClass() const = 0;

    /** Return the value of the control as QVariant. (Used for presets)
        If this function returns an invalid QVariant(), the item is not
        included in presets. */
    virtual QVariant valueVariant() const = 0;
    /** Sets the value of the control. (Used for presets) */
    virtual void setValueVariant(const QVariant&) = 0;

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

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dropEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;

private:

    void p_update_from_properties_();

    static QMap<QString, AbstractFrontItem*> p_reg_items_;
    static int p_id_count_;

    QString p_id_;
    Properties * p_props_;
    QStaticText * p_statictext_name_;
    QRectF p_oldRect_;
    QRectF p_labelRect_;
    bool p_prop_changed_,
         p_editMode_,
         p_canHaveChildren_,
         p_startDrag_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTFRONTITEM_H
