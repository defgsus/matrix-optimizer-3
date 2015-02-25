/** @file frontscene.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include "frontscene.h"
#include "gui/item/abstractfrontitem.h"
#include "object/object.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertimeline1d.h"

namespace MO {
namespace GUI {


struct FrontScene::Private
{
    Private(FrontScene * s) : gscene(s) { }

    /** Recusively creates all items for the object(s) */
    void createItems(Object * root, const QPointF& pos = QPointF(0,0),
                                    AbstractFrontItem * parent = 0);

    FrontScene * gscene;


};



FrontScene::FrontScene(QObject *parent)
    : QGraphicsScene    (parent)
    , p_                (new Private(this))
{
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(onSelectionChanged_()));
    /*
    auto i = new AbstractFrontItem(0, 0);
    addItem(i);
            new AbstractFrontItem(0, i);
    addItem(new AbstractFrontItem(0, 0));
    */
}

FrontScene::~FrontScene()
{
    delete p_;
}


void FrontScene::setRootObject(Object *root)
{
    clear();

    p_->createItems(root);
}

void FrontScene::Private::createItems(Object *root, const QPointF& pos, AbstractFrontItem *parent)
{
    qreal maxh = pos.y();
    auto localPos = pos;
    for (Parameter * p : root->params()->parameters())
    {
        if (!p->isVisibleInterface())
            continue;

        // create/add item
        auto item = new AbstractFrontItem(p, parent);
        item->setPos(localPos);
        if (!parent || parent->scene() != gscene)
            gscene->addItem(item);

        // keep track of insert pos
        localPos.rx() += item->rect().width() + 4.;
        maxh = std::max(maxh, item->rect().bottom() + 4.);
    }

    localPos.ry() = maxh;

    // recurse
    for (Object * c : root->childObjects())
        createItems(c, pos, 0);
}


void FrontScene::onSelectionChanged_()
{
    /** @todo multi-select signal */

    auto list = selectedItems();
    for (QGraphicsItem * item : list)
        if (auto a = qgraphicsitem_cast<AbstractFrontItem*>(item))
        {
            emit itemSelected(a);
            break;
        }
}


} // namespace GUI
} // namespace MO
