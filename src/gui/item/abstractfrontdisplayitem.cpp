/** @file abstractfrontdisplayitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#include <QGraphicsSceneDragDropEvent>

#include "abstractfrontdisplayitem.h"
#include "gui/util/frontpreset.h"
#include "gui/util/frontscene.h"
#include "types/properties.h"
#include "object/scene.h"
#include "object/util/objecteditor.h"
#include "model/objectmimedata.h"

namespace MO {
namespace GUI {

AbstractFrontDisplayItem::AbstractFrontDisplayItem(QGraphicsItem * parent)
    : AbstractFrontItem (parent)
    , p_d_object_       (0)
{
    //setCanHaveChildren(true);

    initProperty("object-id", QString("/"));
    initProperty("object-output", 0);

    initProperty("value-label-visible", false);
    initProperty("value-label-outside", true);
    initProperty("value-label-align", Properties::Alignment(Properties::A_BOTTOM | Properties::A_HCENTER));
    initProperty("value-label-margin", 0);
}


void AbstractFrontDisplayItem::onPropertiesChanged()
{
    assignObject();
}

bool AbstractFrontDisplayItem::assignObject()
{
    p_d_object_ = 0;

    // get pointer to MO::Scene
    auto s = frontScene();
    if (!s)
        return false;
    auto e = s->objectEditor();
    if (!e)
        return false;

    // find object pointer
    p_d_object_ = e->scene()->findChildObject(
                properties().get("object-id").toString(),
                true);

    return p_d_object_ != 0;
}

bool AbstractFrontDisplayItem::assignObject(Object * o)
{
    setProperty("object-id", o ? o->idName() : QString(""));
    return assignedObject();
}

uint AbstractFrontDisplayItem::assignedChannel() const
{
    int c = properties().get("object-output").toInt();
    return c < 0 ? 0 : c;
}

/*
void AbstractFrontDisplayItemItem::onEditModeChanged()
{
    // disable mouse events for controls in edit mode
    Qt::MouseButtons mb = editMode() ? Qt::NoButton : Qt::AllButtons;

    if (p_->fader)
        p_->fader->setAcceptedMouseButtons(mb);
}
*/



void AbstractFrontDisplayItem::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    // generally ignore
    e->ignore();

    // an object?
    if (auto odata = ObjectMimeData::objectMimeData(e->mimeData()))
    {
        if (!odata->isSameApplicationInstance())
            return;

        Object * o = odata->getDescription().pointer();
        if (acceptObject(o))
        {
            e->accept();
            return;
        }
    }

    AbstractFrontItem::dragEnterEvent(e);
}

void AbstractFrontDisplayItem::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    // generally ignore
    e->ignore();

    // an object?
    if (auto odata = ObjectMimeData::objectMimeData(e->mimeData()))
    {
        if (!odata->isSameApplicationInstance())
            return;

        Object * o = odata->getDescription().pointer();
        if (acceptObject(o))
        {
            assignObject(o);
            e->accept();
            return;
        }
    }

    AbstractFrontItem::dropEvent(e);
}



} // namespace GUI
} // namespace MO
