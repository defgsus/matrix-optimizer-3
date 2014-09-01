/** @file MicrophoneGroupgroup.cpp

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include "microphonegroup.h"


#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "param/parameterint.h"
#include "objectfactory.h"
#include "scene.h"
#include "microphone.h"

namespace MO {

MO_REGISTER_OBJECT(MicrophoneGroup)

MicrophoneGroup::MicrophoneGroup(QObject *parent) :
    Object(parent)
{
    setName("MicrophoneGroup");
}

void MicrophoneGroup::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("micg", 1);
}

void MicrophoneGroup::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("micg", 1);
}

void MicrophoneGroup::createParameters()
{
    beginParameterGroup("mics", "microphones");

        pNumMics_ = createIntParameter("nummic", tr("number microphones"),
                                       tr("The number of microphones to create in the group"),
                                       1, true, false);

    endParameterGroup();
}

void MicrophoneGroup::onParameterChanged(Parameter *p)
{
    if (p == pNumMics_)
        updateNumMics_();
}

void MicrophoneGroup::onParametersLoaded()
{
    updateNumMics_();
}

void MicrophoneGroup::updateNumMics_()
{
    /*
    Scene * scene = sceneObject();
    if (!scene)
    {
        MO_WARNING("MicrophoneGroup::updateNumMics_() missing Scene object");
        return;
    }

    int num = pNumMics_->baseValue();

    const QList<Microphone*> exist = findChildObjects<Microphone>();

    // create some more
    if (num > exist.size())
    {
        for (int i=0; i < num - exist.size(); ++i)
        {
            Object * mic = ObjectFactory::createObject("Microphone");
            scene->addObject(this, mic);
        }
    }
    // remove some
    else if (num < exist.size())
    {
        for (int i=exist.size()-1; i>=num; --i)
            scene->deleteObject(exist[i]);
    }
    */
}



} // namespace MO
