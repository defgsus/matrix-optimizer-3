/** @file MicrophoneGroupgroup.cpp

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#if 0

#include "microphonegroup.h"


#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "param/parameters.h"
#include "param/parameterint.h"
#include "param/parameterfloat.h"
#include "audio/audiomicrophone.h"
#include "math/vector.h"


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
    Object::createParameters();

    params()->beginParameterGroup("mics", "microphones");

        pNumMics_ = params()->createIntParameter("nummic", tr("number microphones"),
                                       tr("The number of microphones to create in the group"),
                                       1, 1, 256, 1, true, false);

        pDistance_ = params()->createFloatParameter("micdist", tr("distance from center"),
                            tr("The distance of the microphones from the center of the group"),
                                          0.0, 0.02);
    params()->endParameterGroup();
}

void MicrophoneGroup::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == pNumMics_)
        requestCreateMicrophones();
}

void MicrophoneGroup::createMicrophones()
{
    Object::createMicrophones();

    micros_ = createOrDeleteMicrophones("groupmicro", pNumMics_->baseValue());
}

void MicrophoneGroup::updateAudioTransformations(Double time, uint thread)
{
    const Mat4 & trans = transformation(thread, 0);

    const Float
            micdist = pDistance_->value(time, thread);

    int index = 0;
    for (AUDIO::AudioMicrophone* m : micros_)
    {
        m->setTransformation(
                    trans * getMicroTransformation_(index, micdist), thread, 0);
        ++index;
    }
}

void MicrophoneGroup::updateAudioTransformations(Double stime, uint blocksize, uint thread)
{
#if (0)
    int index = 0;
    for (AUDIO::AudioMicrophone* m : micros_)
    {
        m->setTransformation(
                transformation(thread, 0)
     //            * getMicroTransformation_(index, stime, thread)
                 , thread, 0);
        m->setTransformation(
                transformation(thread, blocksize-1)
     //               * getMicroTransformation_(index, stime + sampleRateInv() * (blocksize-1), thread)
                 , thread, blocksize-1);
        ++index;
    }
#else

    for (uint i=0; i<blocksize; ++i)
    {
        const Double time = stime + sampleRateInv() * i;

        const Float
                micdist = pDistance_->value(time, thread);

        int index = 0;
        for (AUDIO::AudioMicrophone* m : micros_)
        {
            m->setTransformation(
                    transformation(thread, i)
                        * getMicroTransformation_(index, micdist)
                        , thread, i);
            ++index;
        }
    }
#endif
}

Mat4 MicrophoneGroup::getMicroTransformation_(uint index, Float dist ) const
{
    return glm::translate(
                MATH::rotate(Mat4(1), 20.f * index, Vec3(0,1,0))
                            , Vec3(0,0,-dist));
}

Mat4 MicrophoneGroup::getMicroTransformation_(uint index, Double time, uint thread) const
{
    const Float
            micdist = pDistance_->value(time, thread);

    return getMicroTransformation_(index, micdist);
}

} // namespace MO

#endif
