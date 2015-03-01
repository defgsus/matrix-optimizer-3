/** @file frontscene.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_UTIL_FRONTSCENE_H
#define MOSRC_GUI_UTIL_FRONTSCENE_H

#include <QGraphicsScene>

#include "gui/item/abstractfrontitem.h"

class QAction;

namespace MO {
class Object;
class Parameter;
namespace IO { class XmlStream; }
namespace GUI {

class FrontScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit FrontScene(QObject *parent = 0);
    ~FrontScene();

    // -------------- io -------------------

    /** Writes all items to xml.
        The stream is expected to be writeable.
        Section "user-interface" will be created. */
    void serialize(IO::XmlStream&) const;
    /** Writes all items from the list to xml.
        The stream is expected to be writeable.
        Section "user-interface" will be created.
        @note Make sure that the list of items only contains top-level-items
        via reduceToCommonParent() or topLevelFrontItems()! */
    void serialize(IO::XmlStream&, const QList<AbstractFrontItem*>&) const;
    /** Reconstructs all items from xml.
        The stream is expected to be readable
        and current section must be "user-interface" */
    void deserialize(IO::XmlStream&);
    /** Reconstructs all items from xml and appends them to the given list.
        Ownership is with caller.
        The stream is expected to be readable
        and current section must be "user-interface" */
    void deserialize(IO::XmlStream&, QList<AbstractFrontItem*>&) const;

    void saveXml(const QString& filename) const;
    void loadXml(const QString& filename);

    // ------------ getter -----------------

    /** List of actions for popups and views */
    QList<QAction*> createDefaultActions();

    /** Returns true when layout editing is enabled */
    bool editMode() const;

    /** Returns the position of the mouse over one of the connected views, or 0,0 */
    QPointF cursorPos() const;

    /** Returns the union of all view rects in scene coords. */
    QRectF viewsRect() const;

    /** Returns the same-sized rect but snapped to a grid */
    QRectF snapGrid(const QRectF&) const;
    QPointF snapGrid(const QPointF&) const;

    /** Given a scene coordinate @p inout, this functions checks, if
        there is an item at that position and transforms the coords
        to item-coords. */
    void getLocalPos(QPointF& inout, AbstractFrontItem ** item) const;

    // ------------ items ------------------

    /** Like selectedItems() but only AbstractFrontItem castables. */
    QList<AbstractFrontItem*> selectedFrontItems() const;

    /** All front items in the scene */
    QList<AbstractFrontItem*> frontItems() const;

    /** All direct childs for scene */
    QList<AbstractFrontItem*> topLevelFrontItems() const;

    /** Reduces the list of items to the ones that have no parent
        or to the ones that have the most common parent.
        For example, if the list contains items that all have the
        same parent, the list is returned unchanged. Additional items,
        that have another parent would be disregarded. In general,
        the returned list contains the children items of the parent
        that has the most children in the supplied list. */
    static QList<AbstractFrontItem*> reduceToCommonParent(const QList<AbstractFrontItem*>&);

    /** Returns the most top-left point of all item positions */
    static QPointF getTopLeftPosition(const QList<AbstractFrontItem*>&);

    // ------------ clipboard --------------

    /** The mime-type string for FrontScene/FrontItems */
    static const QString FrontItemMimeType;

    /** Returns true when the clipboard contains frontItems */
    bool isItemsInClipboard() const;

    /** Puts all given items into the clipboard.
        @note Make sure that the list of items only contains top-level-items
        via reduceToCommonParent()!
        Returns success, which should always be true, unless there's no
        memory or a programming error. */
    bool copyToClipboard(const QList<AbstractFrontItem*>&) const;

    /** Returns new instances of FrontItems from the clipboard.
        Ownership is with caller.
        On failure, an empty list is returned. */
    QList<AbstractFrontItem*> pasteFromClipboard() const;

signals:

    /** Whenever the edit-actions have changed */
    void actionsChanged(const QList<QAction*>&);

    /** When an item was selected */
    void itemSelected(AbstractFrontItem*);
    /** When multiple items where selected */
    void itemsSelected(const QList<AbstractFrontItem*>&);
    /** When the selection has changed to nothing */
    void itemUnselected();

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Change to/from edit/interaction mode */
    void setEditMode(bool e);

    // ---------------- editing ------------------

    // xxx getting there..
    AbstractFrontItem* createNew(FrontItemType type, QGraphicsItem* parent,
                                 const QPointF& pos = QPointF());

    /** Creates a new item of @p type. If the given position falls within the
        rect of another AbstractFrontItem, the new item will be a child of this. */
    AbstractFrontItem* createNew(FrontItemType type, const QPointF& pos);

    /** Create a group and put all @p items inside */
    void groupItems(const QList<AbstractFrontItem*>& items);

private slots:

    void onSelectionChanged_();

protected:

    void mouseMoveEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    void drawForeground(QPainter * p, const QRectF &rect) Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_UTIL_FRONTSCENE_H
