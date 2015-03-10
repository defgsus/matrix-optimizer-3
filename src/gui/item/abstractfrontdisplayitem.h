/** @file abstractfrontdisplayitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_ABSTRACTFRONTDISPLAYITEM_H
#define MOSRC_GUI_ITEM_ABSTRACTFRONTDISPLAYITEM_H

#include "abstractfrontitem.h"

namespace MO {
namespace GUI {

/** Base for all items that are displays, rather than controls */
class AbstractFrontDisplayItem : public AbstractFrontItem
{
public:

    explicit AbstractFrontDisplayItem(QGraphicsItem * parent = 0);

    // ---------------- getter ------------------

    /** Returns true, when the object pointer is querried from MO::Scene.
        @see assignObject() */
    bool isObjectAssigned() const { return p_d_object_ != 0; }

    /** Returns the pointer to the assigned Object, or NULL.
        @see assignObject() */
    Object * assignedObject() const { return p_d_object_; }

    /** Returns the channel as set in property 'object-output' */
    uint assignedChannel() const;

    // -------------- setter --------------------

    /** Assigns the object with given id defined in property 'object-id'.
        The item must be tied to a FrontScene and an ObjectEditor
        must be assigned to the scene. */
    bool assignObject();

    // --------------- layout -------------------

    // ----------- virtual interface ------------
protected:

    /** Calls assignObject() */
    void onPropertiesChanged() Q_DECL_OVERRIDE;

    //void onEditModeChanged() Q_DECL_OVERRIDE;
public:

    // ----------- QGraphicsItem ----------------

private:

    Object * p_d_object_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTFRONTDISPLAYITEM_H
