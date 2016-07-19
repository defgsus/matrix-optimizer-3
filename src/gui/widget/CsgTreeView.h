#ifndef MOSRC_GUI_WIDGET_CSGTREEVIEW_H
#define MOSRC_GUI_WIDGET_CSGTREEVIEW_H

#include <functional>

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

    CsgBase * currentNode() const;

    QMenu * createNodeMenu(const QModelIndex&);

signals:

    void nodeSelected(CsgBase *);
    void treeChanged();

public slots:

    /** Call this instead of QTreeView::setModel()
        to link with the derived class */
    void setCsgModel(CsgTreeModel*);

    void updateModel(CsgBase * selectThis = 0);

private:

    void popup_(const QModelIndex&);
    void addNodeMenus_(QMenu*, const QList<const CsgBase*>& list,
                       std::function<void(const CsgBase*)> func);
    QMenu * createReplacementMenu_(CsgBase*);
    QMenu * createContainMenu_(CsgBase*);
    QMenu * createAddMenu_(CsgBase*);
    void addEditActions_(CsgBase*, QMenu*);

    CsgTreeModel * csgModel_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CSGTREEVIEW_H
