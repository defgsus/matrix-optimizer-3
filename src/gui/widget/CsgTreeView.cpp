#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QSet>

#include "CsgTreeView.h"
#include "model/CsgTreeModel.h"
#include "math/CsgBase.h"

namespace MO {
namespace GUI {


CsgTreeView::CsgTreeView(QWidget *parent)
    : QTreeView     (parent)
    , csgModel_     (0)
{
    setHeaderHidden(true);

    // select
    connect(this, &QTreeView::clicked, [=](const QModelIndex& idx)
    {
        if (csgModel_)
            if (auto n = csgModel_->nodeForIndex(idx))
                emit nodeSelected(n);
    });

    // right-click
    connect(this, &QTreeView::pressed, [=](const QModelIndex& idx)
    {
        if (!(QApplication::mouseButtons() & Qt::RightButton))
            return;

        if (csgModel_)
            popup_(idx);
    });
}

void CsgTreeView::setCsgModel(CsgTreeModel* m)
{
    setModel(csgModel_ = m);
}

CsgBase* CsgTreeView::currentNode() const
{
    return !csgModel_ || !currentIndex().isValid()
            ? 0
            : csgModel_->nodeForIndex(currentIndex());
}

void CsgTreeView::updateModel(CsgBase* selectThis)
{
    if (!csgModel_)
        return;

    CsgBase * selNode =
            selectThis ? selectThis : currentNode();

    // store not-expanded flags
    auto indices = csgModel_->getAllIndices();
    QSet<CsgBase*> exp;
    for (const auto& i : indices)
        if (!isExpanded(i))
            exp.insert(csgModel_->nodeForIndex(i));

    // reset model
    // XXX Sorry, but i spend and wasted months to properly
    // work with QAbstractItemModel and all this QModelIndex
    // mangling tires me.
    csgModel_->setRootObject(csgModel_->rootObject());

    QModelIndex selIndex;

    // restore expanded flags
    indices = csgModel_->getAllIndices();
    for (const auto& i : indices)
    if (auto node = csgModel_->nodeForIndex(i))
    {
        if (!exp.contains(node))
            setExpanded(i, true);
        // find desired focus index
        if (node == selNode)
            selIndex = i;
    }

    if (selIndex.isValid())
    {
        setCurrentIndex(selIndex);
        emit nodeSelected(selNode);
    }
}


QMenu * CsgTreeView::createNodeMenu(const QModelIndex& idx)
{
    if (!csgModel_)
        return 0;

    CsgBase * node = csgModel_->nodeForIndex(idx);
    if (!node)
        return 0;

    auto menu = new QMenu(this);
    //QAction * a;

    // replacement
    auto sub = createReplacementMenu_(node);
    if (sub)
        menu->addMenu(sub);

    // containment
    sub = createContainMenu_(node);
    if (sub)
        menu->addMenu(sub);

    // new...
    sub = createAddMenu_(node);
    if (sub)
        menu->addMenu(sub);

    // edit
    menu->addSeparator();
    addEditActions_(node, menu);

    return menu;
}

void CsgTreeView::addNodeMenus_(
        QMenu* menu,
        const QList<const CsgBase*>& list,
        std::function<void(const CsgBase*)> func)
{
    QMap<CsgBase::Type, QMenu*> map;

    // create sub-menus
    for (auto n : list)
    {
        auto type = n->type();
        if (!map.contains(type))
        {
            auto sub = menu->addMenu(CsgBase::typeName(type));
            map.insert(type, sub);
        }
    }

    if (map.size() == 1)
        map.first()->deleteLater();

    // create items
    for (auto n : list)
    {
        // if only one group - don't use sub
        auto sub = map.size() == 1 ? menu : map[n->type()];

        auto a = sub->addAction(n->name());
        connect(a, &QAction::triggered, [=]() { func(n); });
    }
}

QMenu* CsgTreeView::createReplacementMenu_(CsgBase* node)
{
    auto listAll = CsgBase::registeredClasses();
    QList<const CsgBase*> list;
    for (auto n : listAll)
        if (n->className() != node->className()
                && node->canBeReplacedBy(n))
            list << n;

    if (list.isEmpty())
        return 0;

    auto menu = new QMenu(tr("replace"), this);

    addNodeMenus_(menu, list, [=](const CsgBase* n)
    {
        if (auto rep = CsgBase::replace(node, n->className()))
        {
            updateModel(rep);
            emit treeChanged();
        }
    });

    return menu;
}

QMenu* CsgTreeView::createContainMenu_(CsgBase* node)
{
    auto listAll = CsgBase::registeredClasses();
    QList<const CsgBase*> list;
    for (auto n : listAll)
        if (n->canHaveChildren())
            list << n;

    if (list.isEmpty())
        return 0;

    auto menu = new QMenu(tr("contain"), this);
    addNodeMenus_(menu, list, [=](const CsgBase* n)
    {
        if (auto paren = CsgBase::contain(node, n->className()))
        {
            updateModel(paren);
            emit treeChanged();
        }
    });

    return menu;
}

QMenu* CsgTreeView::createAddMenu_(CsgBase* node)
{
    if (!node->canHaveChildren())
        return 0;

    auto list = CsgBase::registeredClasses();

    auto menu = new QMenu(tr("add children"), this);
    addNodeMenus_(menu, list, [=](const CsgBase* n)
    {
        node->addChildren(n->cloneClass());
        updateModel();
        emit treeChanged();
    });

    return menu;
}

void CsgTreeView::addEditActions_(CsgBase* node, QMenu* menu)
{
    QAction * a;

    // delete
    if (node->parent())
    {
        a = menu->addAction(tr("delete"));
        connect(a, &QAction::triggered, [=]()
        {
            CsgBase::deleteNode(node);
            updateModel();
            emit treeChanged();
        });
    }
}


void CsgTreeView::popup_(const QModelIndex& idx)
{
    auto m = createNodeMenu(idx);
    m->setAttribute(Qt::WA_DeleteOnClose); // does it work???
    m->popup(QCursor::pos());
}


} // namespace GUI
} // namespace MO
