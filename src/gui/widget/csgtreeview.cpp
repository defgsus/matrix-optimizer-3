#include "csgtreeview.h"
#include "model/csgtreemodel.h"

namespace MO {
namespace GUI {


CsgTreeView::CsgTreeView(QWidget *parent)
    : QTreeView     (parent)
    , csgModel_     (0)
{
    setHeaderHidden(true);

    connect(this, &QTreeView::clicked, [=](const QModelIndex& idx)
    {
        if (csgModel_)
            if (auto n = csgModel_->nodeForIndex(idx))
                emit nodeSelected(n);
    });
}

void CsgTreeView::setCsgModel(CsgTreeModel* m)
{
    setModel(csgModel_ = m);
}

} // namespace GUI
} // namespace MO
