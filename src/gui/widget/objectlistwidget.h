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
namespace GUI {

class ObjectListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit ObjectListWidget(QWidget *parent = 0);

signals:

    /** An object has been selected via double-click.
        (The object can also be the scene itself) */
    void objectSelected(MO::Object *);

public slots:

    /** Sets the object who's childs should be displayed in the list.
        Set to NULL, to disable the view. */
    void setParentObject(MO::Object * parent);

private slots:

    void onDoubleClicked_(QListWidgetItem*);

private:

    void updateList_();

    Object * obj_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_OBJECTLISTWIDGET_H
