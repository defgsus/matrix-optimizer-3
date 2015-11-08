#ifndef MOSRC_GUI_WIDGET_CSGTREEVIEW_H
#define MOSRC_GUI_WIDGET_CSGTREEVIEW_H

#include <QTreeView>


namespace MO {
class CsgBase;
class CsgTreeModel;
namespace GUI {


class CsgTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit CsgTreeView(QWidget *parent = 0);

signals:

    void nodeSelected(CsgBase *);

public slots:

    /** Call this instead of QTreeView::setModel()
        to link with the derived class */
    void setCsgModel(CsgTreeModel*);

private:

    CsgTreeModel * csgModel_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CSGTREEVIEW_H
