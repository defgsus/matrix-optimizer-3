/** @file iconbar.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_ICONBAR_H
#define MOSRC_GUI_WIDGET_ICONBAR_H

#include <QTabWidget>
#include <QMap>

namespace MO {
namespace GUI {

/** A toolbar with dragable icons.

    All hacky right now..
*/
class IconBar : public QTabWidget
{
    Q_OBJECT
public:
    explicit IconBar(QWidget *parent = 0);

    QWidget * getGroupWidget(const QString& group);

signals:

public slots:

    /** Adds an icon to the group.
        For each group, there will be a tab */
    void addIcon(const QIcon& icon,
                 const QString& toolTip,
                 const QString& mimeType,
                 const QByteArray& mimeData,
                 const QString& group = "");

    void addStretch(const QString& group = "", int stretch = 0);

    /** Puts a stretch at the end of all groups */
    void finish();

private:

    QMap<QString, QWidget*> p_widgets_;

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ICONBAR_H
