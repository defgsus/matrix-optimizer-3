/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#include "objecttreeview.h"
#include "model/objecttreemodel.h"
#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/param/parameter.h"
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
    void connect();
    /** Updates the expanded/collapsed flags according
        to each Object::DT_TREE_VIEW_EXPANDED */
    void updateExpansion();
    void updateExpansion(const QModelIndex& idx);

    void onModelReset();
    void updateModel();

    void setCurIndex(const QModelIndex& idx);
    void setExpanded(const QModelIndex& idx, bool e);

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
    p_->model->setRootObject(o);
    p_->connect();
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

void ObjectTreeView::Private::updateExpansion()
{
    updateExpansion(QModelIndex());
}

void ObjectTreeView::Private::updateExpansion(const QModelIndex& idx)
{
    auto obj = model->objectForIndex(idx);
    if (obj)
    {
        p->setExpanded(idx, obj->getAttachedData(Object::DT_TREE_VIEW_EXPANDED)
                            .toBool());
    }
    // traverse childs
    const int rows = model->rowCount(idx);
    for (int i=0; i<rows; ++i)
    {
        updateExpansion(model->index(i, 0, idx));
    }
}

void ObjectTreeView::Private::connect()
{
    if (!model->rootObject())
    {
        editor = nullptr;
        return;
    }
    editor = model->rootObject()->editor();
    if (!editor)
        return;

    QObject::connect(editor, &ObjectEditor::objectAdded, [=](){ updateModel(); });
    QObject::connect(editor, &ObjectEditor::objectChanged, [=](){ updateModel(); });
    QObject::connect(editor, &ObjectEditor::objectColorChanged, [=](){ updateModel(); });
    QObject::connect(editor, &ObjectEditor::objectDeleted, [=](const Object*o)
    {
        // XXX Doesn't catch deletion of parent of selection
        //if (p->selectedObject() == o)
        //    p->selectNone();
        updateModel();
    });
    QObject::connect(editor, &ObjectEditor::objectMoved, [=](){ updateModel(); });
    QObject::connect(editor, &ObjectEditor::objectNameChanged, [=](){ p->update(); });
    QObject::connect(editor, &ObjectEditor::objectsAdded, [=](){ updateModel(); });
    QObject::connect(editor, &ObjectEditor::objectsDeleted, [=](const QList<Object*>& os)
    {
        updateModel();
    });
    QObject::connect(editor, &ObjectEditor::parameterChanged, [=](){ p->update(); });
    QObject::connect(editor, &ObjectEditor::parametersChanged, [=](){ p->update(); });
}

void ObjectTreeView::Private::updateModel()
{
    model->setRootObject(model->rootObject());
}


} // namespace GUI
} // namespace MO
