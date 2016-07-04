/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/4/2016</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_MASTEROUTINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_MASTEROUTINTERFACE_H

namespace MO {

class MasterOutInterface
{
public:

    virtual bool isMasterOutputEnabled() const = 0;

    /** Sets the master-out parameter.
        if @p sendGui, this will be done via ObjectEditor */
    virtual void setMasterOutputEnabled(bool enable, bool sendGui = false) = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_MASTEROUTINTERFACE_H

