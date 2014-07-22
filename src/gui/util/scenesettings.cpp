/** @file scenesettings.cpp

    @brief ViewSpaces and stuff for scene objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include "scenesettings.h"
#include "object/sequence.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {


SceneSettings::SceneSettings(QObject *parent)
    :   QObject(parent)
{
}


void SceneSettings::setViewSpace(const Object *obj, const UTIL::ViewSpace &viewspace)
{
    viewSpaces_.insert(obj->idName(), viewspace);
}

UTIL::ViewSpace SceneSettings::getViewSpace(const Object *obj)
{
    // return saved viewspace
    auto i = viewSpaces_.find(obj->idName());
    if (i != viewSpaces_.end())
        return i.value();

    // create new space
    UTIL::ViewSpace space;

    // adjust/initialize to certain objects

    if (const Sequence * seq = qobject_cast<const Sequence*>(obj))
    {
        space.setScaleX(seq->length());
    }

    if (const SequenceFloat * seqf = qobject_cast<const SequenceFloat*>(obj))
    {
        Double minv, maxv;
        seqf->getMinMaxValue(0, seqf->length(), minv, maxv);
        space.setY(minv);
        space.setScaleY(maxv-minv);
    }

    return space;
}


} // namespace GUI
} // namespace MO
