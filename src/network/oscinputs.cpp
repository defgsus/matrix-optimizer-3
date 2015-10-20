/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#include "oscinputs.h"
#include "oscinput.h"

namespace MO {

namespace
{
    static OscInputs * instance_ = 0;
    static QMap<uint16_t, OscInput*> portMap_;
};


OscInputs::OscInputs(QObject *parent)
    : QObject   (parent)
{
}

OscInputs::~OscInputs()
{
}

OscInputs * OscInputs::instance()
{
    if (instance_ == 0)
        instance_ = new OscInputs();
    return instance_;
}

OscInput * OscInputs::getListener(uint16_t port)
{
    auto i = portMap_.find(port);
    // return existing
    if (i != portMap_.end())
    {
        i.value()->addRef();
        return i.value();
    }

    // create
    auto osc = new OscInput();
    // and open
    if (!osc->open(port))
    {
        osc->releaseRef();
        return 0;
    }
    // install in map
    portMap_.insert(port, osc);
    emit instance()->listenersChanged();
    return osc;
}

void OscInputs::releaseListener(uint16_t port)
{
    auto i = portMap_.find(port);
    if (i == portMap_.end())
        return;

    if (i.value()->referenceCount() > 2)
        i.value()->releaseRef();
    else
    {
        i.value()->releaseRef();
        i.value()->releaseRef();
        portMap_.remove(port);
        emit instance()->listenersChanged();
    }
}

QList<OscInput*> OscInputs::listeners()
{
    QList<OscInput*> list;
    for (auto l : portMap_)
        list << l;
    return list;
}




} // namespace MO
