/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/7/2015</p>
*/

#include <QLayout>

#include "csgeditwidget.h"
#include "gui/propertiesview.h"
#include "gui/widget/csgtreeview.h"
#include "types/properties.h"
#include "model/csgtreemodel.h"
#include "math/csgbase.h"
#include "math/csgcombine.h"
#include "math/csgprimitives.h"
#include "io/log.h"

namespace MO {
namespace GUI {

struct CsgEditWidget::Private
{
    Private(CsgEditWidget * w)
        : widget    (w)
        , model     (0)
        , curNode   (0)
        , ignoreTreeSignal(false)
    {

    }

    void createWidgets();
    void setCurrentNode(CsgBase*);
    void updateNodeProperties();
    void updateProperties();

    CsgEditWidget * widget;
    CsgTreeView * treeView;
    PropertiesView * propView;
    CsgTreeModel * model;
    CsgBase * curNode;
    bool ignoreTreeSignal;
};

CsgEditWidget::CsgEditWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    p_->createWidgets();


    auto root = new CsgRoot;

        auto c = new CsgUnion;
        root->addChildren(c);
        c->setName("big union");

            auto s = new CsgSphere;
            auto props = s->properties();
            props.set("x", 1);
            s->setProperties(props);
            c->addChildren(s);
            c->addChildren(new CsgPlane);
            c->addChildren(new CsgSphere);

            auto c1 = new CsgIntersection;
            c->addChildren(c1);

                c1->addChildren(new CsgSphere);
                c1->addChildren(new CsgBox);

    setRootObject(root);
}

CsgEditWidget::~CsgEditWidget()
{
    delete p_;
}

void CsgEditWidget::Private::createWidgets()
{
    auto lh = new QHBoxLayout(widget);
    lh->setMargin(1);

        treeView = new CsgTreeView(widget);
        lh->addWidget(treeView);
        connect(treeView, &CsgTreeView::nodeSelected, [this](CsgBase*n)
        {
            setCurrentNode(n);
        });
        connect(treeView, SIGNAL(treeChanged()), widget, SIGNAL(changed()));

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            propView = new PropertiesView(widget);
            lv->addWidget(propView);
            connect(propView, &PropertiesView::propertyChanged, [this]()
            {
                updateNodeProperties();
                emit widget->changed();
            });

            lv->addStretch(2);
}

void CsgEditWidget::setRootObject(CsgRoot* root)
{
    p_->curNode = 0;
    if (!p_->model)
    {
        p_->model = new CsgTreeModel(p_->treeView);
        connect(p_->model, &CsgTreeModel::dataChanged, [this]()
        {
            if (!p_->ignoreTreeSignal)
                p_->updateProperties();
        });
    }
    p_->model->setRootObject(root);
    p_->treeView->setCsgModel(p_->model);

    //MO_PRINT(root->toGlsl());
}

CsgRoot* CsgEditWidget::rootObject() const
{
    return p_->model ? p_->model->rootObject() : 0;
}

void CsgEditWidget::Private::setCurrentNode(CsgBase* n)
{
    curNode = n;
    updateProperties();
}

void CsgEditWidget::Private::updateProperties()
{
    if (curNode)
        propView->setProperties(curNode->properties());
    else
        propView->setProperties(Properties());
}

void CsgEditWidget::Private::updateNodeProperties()
{
    if (!curNode)
        return;

    // set properties for node
    curNode->setProperties(propView->properties());

    // update model (if name has changed)
    QMap<int, QVariant> pmap;
    pmap.insert(Qt::EditRole, curNode->name());

    ignoreTreeSignal = true;
    model->setItemData(treeView->currentIndex(), pmap);
    ignoreTreeSignal = false;
}


} // namespace GUI
} // namespace MO
