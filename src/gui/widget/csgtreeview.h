#ifndef MOSRC_GUI_WIDGET_CSGTREEVIEW_H
#define MOSRC_GUI_WIDGET_CSGTREEVIEW_H

#include <QTreeView>


namespace MO {
namespace GUI {


class CsgTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit CsgTreeView(QWidget *parent = 0);

signals:

public slots:
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CSGTREEVIEW_H
