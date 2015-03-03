/** @file objectlistwidgetitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_OBJECTLISTWIDGETITEM_H
#define MOSRC_GUI_WIDGET_OBJECTLISTWIDGETITEM_H

#include <QListWidgetItem>

namespace MO {
class Object;
namespace GUI {
class ObjectListWidget;

/** Overload of data() and setData() for handling object names */
class ObjectListWidgetItem : public QListWidgetItem
{
public:
    /*explicit ObjectListWidgetItem(QListWidget * parent = 0, int type = Type)
        : QListWidgetItem(parent, type) { }
    explicit ObjectListWidgetItem(const QString & text, QListWidget * parent = 0, int type = Type)
        : QListWidgetItem(text, parent, type) { }
    explicit ObjectListWidgetItem(const QIcon & icon, const QString & text, QListWidget * parent = 0, int type = Type)
        : QListWidgetItem(icon, text, parent, type) { }
    */

    explicit ObjectListWidgetItem(Object * o, ObjectListWidget * parent, int type);

    virtual QVariant data(int role) const Q_DECL_OVERRIDE;
    virtual void setData(int role, const QVariant & value) Q_DECL_OVERRIDE;

private:

    Object * object_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_OBJECTLISTWIDGETITEM_H
