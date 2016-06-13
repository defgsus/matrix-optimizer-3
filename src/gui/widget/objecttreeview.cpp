/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#include "objecttreeview.h"
#include "model/objecttreemodel.h"
#include "object/object.h"

namespace MO {
namespace GUI {

struct ObjectTreeView::Private
{
    Private(ObjectTreeView*p)
        : p         (p)
        , model     (new ObjectTreeModel())
    { }

    ~Private()
    {
        delete model;
    }

    void setCurIndex(const QModelIndex& idx);

    ObjectTreeView* p;
    ObjectTreeModel* model;
};


ObjectTreeView::ObjectTreeView(QWidget *parent)
    : QTreeView     (parent)
    , p_            (new Private(this))
{
    setObjectName("ObjectTreeView");
    setHeaderHidden(true);
    //setColumnHidden(1, true);
    setModel(p_->model);

    connect(this, &ObjectTreeView::clicked, [=](const QModelIndex& idx)
    {
        p_->setCurIndex(idx);
    });
    connect(this, &ObjectTreeView::activated, [=](const QModelIndex& idx)
    {
        p_->setCurIndex(idx);
    });
}

ObjectTreeView::~ObjectTreeView()
{
    delete p_;
}

void ObjectTreeView::setObject(Object* o)
{
    p_->model->setRootObject(o);
}

void ObjectTreeView::Private::setCurIndex(const QModelIndex& idx)
{
    auto obj = model->itemForIndex(idx);
    if (obj)
        emit p->objectSelected(obj);
}


} // namespace GUI
} // namespace MO
