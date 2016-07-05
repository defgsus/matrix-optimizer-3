/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H
#define MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H

#include <QTreeView>
#include "object/object_fwd.h"

namespace MO {
namespace GUI {

class ObjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ObjectTreeView(QWidget *parent = 0);
    ~ObjectTreeView();

    Object* rootObject() const;
    Object* selectedObject() const;
    QList<Object*> selectedObjects() const;

signals:

    void objectSelected(MO::Object*);

public slots:

    void setRootObject(Object*);

    void selectObject(Object*);
    void selectObjects(const QList<Object*>&);
    void selectNone();

protected:

    void mousePressEvent(QMouseEvent*) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

private:
    struct Private;
    Private* p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H
