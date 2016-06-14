/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#include <QMouseEvent>
#include <QMenu>
#include <QAction>

#include "objecttreeview.h"
#include "model/objecttreemodel.h"
#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/param/parameter.h"
#include "gui/util/objectactions.h"
#include "io/log.h"
#include "io/streamoperators_qt.h"

namespace MO {
namespace GUI {

struct ObjectTreeView::Private
{
    Private(ObjectTreeView*p)
        : p             (p)
        , model         (new ObjectTreeModel())
        , editor        (nullptr)
        , selObjectTemp (nullptr)
    { }

    ~Private()
    {
        delete model;
    }

    /** Connect to ObjectEditor signals */
    void connectEditor();
    /** Updates the expanded/collapsed flags according
        to each Object::DT_TREE_VIEW_EXPANDED */
    void updateExpansion(bool override = false, bool exp = false);
    void updateExpansion(const QModelIndex& idx,
                         bool override = false, bool exp = false);

    void updateModel();

    void setCurIndex(const QModelIndex& idx);
    void setExpanded(const QModelIndex& idx, bool e);

    void showPopup();

    ObjectTreeView* p;
    ObjectTreeModel* model;
    ObjectEditor* editor;
    Object* selObjectTemp;
};


ObjectTreeView::ObjectTreeView(QWidget *parent)
    : QTreeView     (parent)
    , p_            (new Private(this))
{
    setObjectName("ObjectTreeView");
    setHeaderHidden(true);
    //setColumnHidden(1, true);
    setDragDropMode(DragDrop);
    setDropIndicatorShown(true);

    setModel(p_->model);

    connect(this, &ObjectTreeView::clicked, [=](const QModelIndex& idx)
    {
        p_->setCurIndex(idx);
        //MO_PRINT((selectedObject() ? selectedObject()->name() : QString("NULL")));
    });
    connect(this, &ObjectTreeView::activated, [=](const QModelIndex& idx)
    {
        p_->setCurIndex(idx);
    });
    connect(this, &ObjectTreeView::expanded, [=](const QModelIndex& idx)
    {
        p_->setExpanded(idx, true);
    });
    connect(this, &ObjectTreeView::collapsed, [=](const QModelIndex& idx)
    {
        p_->setExpanded(idx, false);
    });

    connect(p_->model, &ObjectTreeModel::modelAboutToBeReset, [=]()
    {
        p_->selObjectTemp = selectedObject();
    });
    connect(p_->model, &ObjectTreeModel::modelReset, [=]()
    {
        p_->updateExpansion();
        if (p_->selObjectTemp)
            selectObject(p_->selObjectTemp);
        p_->selObjectTemp = nullptr;
    });

}

ObjectTreeView::~ObjectTreeView()
{
    delete p_;
}

Object* ObjectTreeView::rootObject() const { return p_->model->rootObject(); }
Object* ObjectTreeView::selectedObject() const
{
    return p_->model->objectForIndex( currentIndex() );
}

void ObjectTreeView::setRootObject(Object* o)
{
    if (!o)
        selectNone();
    else
        if (o == p_->model->rootObject())
            return;

    p_->model->setRootObject(o);
    p_->connectEditor();
    p_->updateExpansion();
}

void ObjectTreeView::selectObject(Object* o)
{
    auto idx = p_->model->indexForObject(o);
    if (idx.isValid())
        setCurrentIndex(idx);
}

void ObjectTreeView::selectNone()
{
    setCurrentIndex(QModelIndex());
    p_->selObjectTemp = nullptr;
}

void ObjectTreeView::Private::setCurIndex(const QModelIndex& idx)
{
    if (auto obj = model->objectForIndex(idx))
        emit p->objectSelected(obj);
}

void ObjectTreeView::Private::setExpanded(const QModelIndex& idx, bool e)
{
    auto obj = model->objectForIndex(idx);
    if (!obj)
        return;
    QVariant v;
    if (e)
        v = true;
    obj->setAttachedData(v, Object::DT_TREE_VIEW_EXPANDED);
}

void ObjectTreeView::Private::updateExpansion(bool override, bool exp)
{
    updateExpansion(QModelIndex(), override, exp);
}

void ObjectTreeView::Private::updateExpansion(
        const QModelIndex& idx, bool override, bool exp)
{
    auto obj = model->objectForIndex(idx);
    if (obj)
    {
        bool e = exp;
        if (!override)
            e = obj->getAttachedData(Object::DT_TREE_VIEW_EXPANDED).toBool();
        p->setExpanded(idx, e);
    }
    // traverse childs
    const int rows = model->rowCount(idx);
    for (int i=0; i<rows; ++i)
    {
        updateExpansion(model->index(i, 0, idx), override, exp);
    }
}

void ObjectTreeView::Private::connectEditor()
{
    if (!model->rootObject())
    {
        editor = nullptr;
        return;
    }
    editor = model->rootObject()->editor();
    if (!editor)
        return;

    connect(editor, &ObjectEditor::objectAdded, [=](){ updateModel(); });
    connect(editor, &ObjectEditor::objectChanged, [=](){ p->update(); });
    connect(editor, &ObjectEditor::objectColorChanged, [=](){ p->update(); });
    connect(editor, &ObjectEditor::objectMoved, [=](){ updateModel(); });
    connect(editor, &ObjectEditor::objectNameChanged, [=](){ p->update(); });
    connect(editor, &ObjectEditor::objectsAdded, [=](){ updateModel(); });
    connect(editor, &ObjectEditor::objectDeleted, [=](){ updateModel(); });
    connect(editor, &ObjectEditor::objectsDeleted, [=](){ updateModel(); });
    connect(editor, &ObjectEditor::parameterChanged, [=](){ p->update(); });
    connect(editor, &ObjectEditor::parametersChanged, [=](){ p->update(); });

    // Deselect objects that are about to be deleted
    // so onModelReset does not re-select deleted objects
    connect(editor, &ObjectEditor::objectAboutToDelete, [=](const Object*o)
    {
        if (p->selectedObject() == o
            || p->selectedObject()->hasParentObject(o))
            p->selectNone();
    });
    connect(editor, &ObjectEditor::objectsAboutToDelete,
                     [=](const QList<Object*>& os)
    {
        auto sel = p->selectedObject();
        for (auto o : os)
        if (sel == o || sel->hasParentObject(o))
        {
            p->selectNone();
            break;
        }
    });
}

void ObjectTreeView::Private::updateModel()
{
    model->setRootObject(model->rootObject());
}

void ObjectTreeView::mousePressEvent(QMouseEvent* e)
{
    QTreeView::mousePressEvent(e);

    if (e->button() == Qt::RightButton)
        p_->showPopup();
}

void ObjectTreeView::Private::showPopup()
{
    auto sel = p->selectedObject();
    auto selIdx = p->currentIndex();

    if (sel == p->rootObject())
        sel = nullptr;

    ActionList actions;
    QAction* a;

    // menu title
    if (sel)
    {
        actions.addTitle(sel->name(), p);
        /*
        a = actions->addAction(sel->name());
        auto f = a->font();
        f.setBold(true);
        a->setFont(f);
        menu->addSeparator();
        */
    }

    auto sub = new QMenu(tr("tree"), p);
    actions.addMenu(sub, p);

        a = sub->addAction(tr("expand all nodes"));
        connect(a, &QAction::triggered, [=](){ updateExpansion(true, true); });

        a = sub->addAction(tr("collapse all nodes"));
        connect(a, &QAction::triggered, [=](){ updateExpansion(true, false); });

    if (sel && sel->numChildren())
    {
        sub = new QMenu(tr("branch"), p);
        actions.addMenu(sub, p);

            a = sub->addAction(tr("expand all nodes"));
            connect(a, &QAction::triggered, [=]()
                { updateExpansion(selIdx, true, true); });

            a = sub->addAction(tr("collapse all nodes"));
            connect(a, &QAction::triggered, [=]()
                { updateExpansion(selIdx, true, false); });
    }

    actions.addSeparator(p);

    if (sel)
    {
        ObjectActions::createEditActions(actions, sel, p);
        actions.addSeparator(p);
    }

    auto par = sel;
    if (!par)
        par = model->rootObject();
    if (par)
        ObjectActions::createNewObjectActions(actions, par, p, nullptr, [=](Object* o)
        {
            p->selectObject(o);
        });

    auto menu = new QMenu(p);
    menu->addActions(actions);

    menu->popup(QCursor::pos());
}


} // namespace GUI
} // namespace MO
