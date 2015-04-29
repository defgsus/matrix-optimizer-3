/** @file microphonegroup.cpp

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef MO_DISABLE_EXP

#include "microphonegroup.h"


#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "param/parameters.h"
#include "param/parameterint.h"
#include "param/parameterfloat.h"
#include "audio/spatial/spatialmicrophone.h"
#include "math/vector.h"


namespace MO {

MO_REGISTER_OBJECT(MicrophoneGroup)

// pot=14
static const Vec3 mic_pos[] = {
    Vec3(-0.707106,	-0.707106,	 0.0), // 1
    Vec3(-1.0, 		 0.0,  		 0.0), // 2
    Vec3(-0.707106,	 0.707106,	 0.0),
    Vec3( 0.0,		 1.0,		 0.0), // 4
    Vec3( 0.707106,	 0.707106,	 0.0),
    Vec3( 1.0,		 0.0,		 0.0), // 6
    Vec3( 0.707106,	-0.707106,	 0.0),
    Vec3( 0.0,		-1.0,		 0.0), // 8

    Vec3(-0.331413,	-0.800103,   -0.5), // 9
    Vec3(-0.800103,	-0.331413,	 -0.5),
    Vec3(-0.800103,	 0.331413,   -0.5),
    Vec3(-0.331413,	 0.800103,	 -0.5), // 12
    Vec3( 0.331413,	 0.800103,	 -0.5),
    Vec3( 0.800103,	 0.331413,	 -0.5),
    Vec3( 0.800103,	-0.331413,	 -0.5), // 15
    Vec3( 0.331413,	-0.800103,	 -0.5),

    Vec3(-0.3535533,	-0.3535533,	 -0.866025), // 17
    Vec3(-0.3535533,	 0.3535533,	 -0.866025),
    Vec3( 0.3535533,	 0.3535533,	 -0.866025),
    Vec3( 0.3535533,	-0.3535533,	 -0.866025),

    Vec3( 0.0,		 0.0,		 -1.0)  // 21 zenith
                    //if (type == Audio::MICRO_22)
    //Vec3( 0.0,		 0.0,		  1.0) ,0,1.0) ); // 22 dome origin / downwards
//                    break;
};




MicrophoneGroup::MicrophoneGroup(QObject *parent) :
    Object(parent)
{
    setName("MicrophoneGroup");
    setNumberMicrophones(1);
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
                                       tr("The number of microphones in this group"),
                                       1, 1, 256, 1, true, false);

        pDistance_ = params()->createFloatParameter("micdist", tr("distance to center"),
                            tr("The distance of the microphones to the center of the group"),
                                          0.0, 0.02);

        pDirExp_ = params()->createFloatParameter("micdirexp", tr("directional exponent"),
                            tr("The exponent setting the opening angle - larger means smaller area"),
                                          3.0, 0.1);
        pDirExp_->setMinValue(0.001);

    params()->endParameterGroup();
}

void MicrophoneGroup::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == pNumMics_)
        setNumberMicrophones(pNumMics_->baseValue());
        //requestCreateMicrophones();
}

void MicrophoneGroup::onParametersLoaded()
{
    Object::onParametersLoaded();

    setNumberMicrophones(pNumMics_->baseValue());
}

/*
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
*/

Mat3 compute_orthogonals(const Vec3& v)
{
    Vec3 v1 = (std::abs(v.x) > std::abs(v.y))
            ? Vec3(-v.z, 0., v.x)
            : Vec3(0., v.z, -v.y);
    Vec3 v2 = glm::cross(v, v1);

    return Mat3(v, v1, v2);
}

void MicrophoneGroup::calculateMicrophoneTransformation(
                    const TransformationBuffer *objectTransformation,
                    const QList<AUDIO::SpatialMicrophone *> & mics,
                    uint bufferSize, SamplePos pos, uint thread)
{
    for (int i=0; i<mics.size(); ++i)
    {
        mics[i]->setDirectionExponent(pDirExp_->value(Double(pos) / sampleRate(), thread));

        // direction -> matrix
        Vec3 micdir = mic_pos[i];
        Vec3 up = glm::normalize(glm::mix(
                                    Vec3(0, micdir.z, -micdir.y),
                                    Vec3(0,-1,0),
                                 std::abs(micdir.x)));
        Mat4 micmat = glm::lookAt(Vec3(0.), micdir, up);

        // for each sample
        for (uint j=0; j<bufferSize; ++j)
        {
            Double time = Double(pos + j) / sampleRate();

            Mat4 micmat2 = glm::translate(Mat4(1.), micdir * Float(pDistance_->value(time, thread)))
                            * micmat;

            mics[i]->transformationBuffer()->setTransformation(
                        objectTransformation->transformation(j) * micmat2
                        , j);
        }
    }
}


} // namespace MO

#endif // MO_DISABLE_EXP
