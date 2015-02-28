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
    /** Reconstructs all items from xml.
        The stream is expected to be readable
        and current section must be "user-interface" */
    void deserialize(IO::XmlStream&);

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

    // ------------ items ------------------

    /** Like selectedItems() but for specified types */
    QList<AbstractFrontItem*> selectedFrontItems() const;

    /** All front items in the scene */
    QList<AbstractFrontItem*> frontItems() const;

    /** All direct childs for scene */
    QList<AbstractFrontItem*> topLevelFrontItems() const;

signals:

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
