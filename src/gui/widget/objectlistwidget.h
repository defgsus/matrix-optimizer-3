/** @file objectlistwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_OBJECTLISTWIDGET_H
#define MOSRC_GUI_WIDGET_OBJECTLISTWIDGET_H

#include <QListWidget>


namespace MO {
class Object;
class ObjectEditor;
namespace GUI {

class ObjectListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ObjectListWidget(QWidget *parent = 0);

    Object * objectForItem(const QListWidgetItem*) const;

signals:

    /** An object has been double-clicked.
        (The object can also be the scene itself) */
    void objectSelected(MO::Object *);
    /** An object has been selected (no double-click) */
    void objectClicked(MO::Object*);

public slots:

    /** Sets the object who's childs should be displayed in the list.
        Set to NULL, to disable the view. */
    void setParentObject(MO::Object * parent);

    /** Selects the object if it's in child list.
        If @p o is NULL, selection will be cleared. */
    void setSelectedObject(MO::Object * o);

protected:

    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    QMimeData * mimeData(const QList<QListWidgetItem *> items) const Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

private slots:

    void onDoubleClicked_(QListWidgetItem*);
    void onItemSelected_(QListWidgetItem*);

    void onObjectAdded_(MO::Object *);
    void onObjectChanged_(MO::Object *);
    void onObjectDeleted_(const MO::Object *);
    void onObjectNamedChanged_(MO::Object *);

private:

    void updateList_();

    Object * obj_, * root_;
    ObjectEditor * editor_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_OBJECTLISTWIDGET_H
