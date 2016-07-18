/** @file microphonegroup.cpp

    @brief Group for uniformly setup microphones

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include "microphonegroup.h"


#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "param/parameters.h"
#include "param/parameterint.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "param/parameterfloatmatrix.h"
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




MicrophoneGroup::MicrophoneGroup()
    : Object        ()
{
    setName("MicrophoneGroup");
    setNumberMicrophones(1);
}

MicrophoneGroup::~MicrophoneGroup()
{

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

    params()->beginParameterGroup("mics", "layout");
    initParameterGroupExpanded("mics");

        paramNumMics_ = params()->createIntParameter(
                    "nummic", tr("number microphones"),
                   tr("The number of microphones in this group"),
                   1, 1, 256, 1, true, false);

        paramMicDist_ = params()->createFloatParameter(
                    "micdist", tr("distance to center"),
                    tr("The distance of the microphones to the center of the group"),
                    0.0, 0.02);

        FloatMatrix m({ 3, 2 });
        *m.data(0, 0) = -1.;
        *m.data(1, 0) = 0.;
        *m.data(2, 0) = 0.;
        *m.data(0, 1) = 1.;
        *m.data(1, 1) = 0.;
        *m.data(2, 1) = 0.;
        paramMatrix_ = params()->createFloatMatrixParameter(
                    "pos_matrix", tr("positions"),
                    tr("A 2d float matrix with the 3d positions of the microphones"),
                    m, true, true);

    params()->beginParameterGroup("params", "parameters");
    initParameterGroupExpanded("params");

        paramAmp_ = params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("Amplitude of the microphone signal"),
                    1., 0.1);

        paramDistFade_ = params()->createFloatParameter(
                    "dist_fade", tr("distance fall-off"),
                    tr("Amplitude decimation by distance factor"),
                    1., 0.1);
        paramDistFade_->setMinValue(0.);

        paramDirExp_ = params()->createFloatParameter(
                    "dir_exp", tr("directional exponent"),
                    tr("The exponent of the directional term defining the "
                       "opening angle of the microphone, the higher - the narrower"),
                    3., 0.1);
        paramDirExp_->setMinValue(0.);


        paramUseDist_ = params()->createBooleanParameter(
                    "use_dist", tr("distance sound"),
                    tr("Enable mixing with distance-sound if sound sources provide it"),
                    tr("Off"), tr("On"),
                    true, true, false);

        paramDistMin_ = params()->createFloatParameter(
                    "min_dist", tr("minimum distance"),
                tr("The minimum distance where mixing-in the distance-sound starts"),
                    3., .1, true, true);
        paramDistMin_->setMinValue(0.);

        paramDistMax_ = params()->createFloatParameter(
                    "max_dist", tr("maximum distance"),
                tr("The distance where the distance-sound is fully mixed-in"),
                    20., .1, true, true);
        paramDistMax_->setMinValue(0.);

    params()->endParameterGroup();
}

void MicrophoneGroup::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);

    if (p == paramNumMics_)
        setNumberMicrophones(paramNumMics_->baseValue());

}

void MicrophoneGroup::onParametersLoaded()
{
    Object::onParametersLoaded();

    setNumberMicrophones(paramNumMics_->baseValue());
}

void MicrophoneGroup::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    const bool useDist = paramUseDist_->baseValue();
    paramDistMin_->setVisible(useDist);
    paramDistMax_->setVisible(useDist);
}

Mat3 compute_orthogonals(const Vec3& v)
{
    Vec3 v1 = (std::abs(v.x) > std::abs(v.y))
            ? Vec3(-v.z, 0., v.x)
            : Vec3(0., v.z, -v.y);
    Vec3 v2 = glm::cross(v, v1);

    return Mat3(v, v1, v2);
}

Mat4 compute_orthogonals4(const Vec3& vi)
{
    Vec3 v = -vi;
    //Vec3 v = Vec3(vi.x,vi.z,vi.y);
    Vec3 v1 = (std::abs(v.x) > std::abs(v.y))
            ? Vec3(v.y, -v.x, 0.f)
            : Vec3(0.f, -v.z, v.y);
    Vec3 v2 = MATH::normalize_safe( glm::cross(v, v1) );

    return Mat4(Vec4(v2,0), Vec4(v1,0), Vec4(v,0), Vec4(0,0,0,1));
}

Vec3 compute_up(const Vec3& v)
{
    /*MATH::normalize_safe(glm::mix(
                        Vec3(0, micdir.z, -micdir.y),
                        Vec3(0,-1,0),
                        std::abs(micdir.x)))*/
    return (std::abs(v.x) > std::abs(v.y))
            ? Vec3(-v.z, 0., v.x)
            : Vec3(0., v.z, -v.y);
}

void MicrophoneGroup::calculateMicrophoneTransformation(
                    const TransformationBuffer *objectTransformation,
                    const QList<AUDIO::SpatialMicrophone *> & mics,
                    const RenderTime& time)
{
    const F32
            amp = paramAmp_->value(time),
            distFade = paramDistFade_->value(time),
            dirExp = std::max(0.00001, paramDirExp_->value(time));
    const bool useDist = paramUseDist_->baseValue();
    F32 minDist=0., maxDist=0.;
    if (useDist)
        minDist = paramDistMin_->value(time),
        maxDist = paramDistMax_->value(time);

    const auto m = paramMatrix_->value(time);
    std::vector<Vec3> mic_vec(mics.size());
    getMicVectors(mic_vec, m);

    for (int i=0; i<mics.size(); ++i)
    {
        // update parameters
        mics[i]->setAmplitude(amp);
        mics[i]->setDistanceFadeout(distFade);
        mics[i]->setDirectionExponent(dirExp);
        mics[i]->setDistanceSound(useDist, minDist, maxDist);

        // direction -> matrix
        Vec3 micdir = mic_vec[i];
#if 0
        Vec3 up = compute_up(micdir);
                /*MATH::normalize_safe(glm::mix(
                                    Vec3(0, micdir.z, -micdir.y),
                                    Vec3(0,-1,0),
                                    std::abs(micdir.x)));*/
        Mat4 micmat = glm::lookAt(Vec3(0.), micdir, up);
#else
        Mat4 micmat = compute_orthogonals4(micdir);
#endif

        if (!paramMicDist_->isModulated())
        {
            Mat4 micmat2 = glm::translate(
                        Mat4(1.), micdir * Float(paramMicDist_->value(time)))
                            * micmat;

            for (uint j=0; j<mics[i]->transformationBuffer()->bufferSize(); ++j)
                mics[i]->transformationBuffer()->setTransformation(
                        objectTransformation->transformation(j) * micmat2
                        , j);
        }
        else
        // for each sample
        for (uint j=0; j<mics[i]->transformationBuffer()->bufferSize(); ++j)
        {
            RenderTime btime(time);
            btime += Double(j) * sampleRateInv();

            Mat4 micmat2 = glm::translate(
                        Mat4(1.), micdir * Float(paramMicDist_->value(time)))
                            * micmat;

            mics[i]->transformationBuffer()->setTransformation(
                        objectTransformation->transformation(j) * micmat2
                        , j);
        }
    }
}

void MicrophoneGroup::getMicVectors(std::vector<Vec3>& vec, const FloatMatrix& m)
{
    if (m.numDimensions() < 2 || m.size(0) < 3)
    {
        for (auto& v : vec)
            v = Vec3(0,0,-1);
        return;
    }
    size_t i, num = std::min(vec.size(), m.size(1));
    for (i=0; i<num; ++i)
    {
        for (size_t j=0; j<3; ++j)
            vec[i][j] = *m.data(j, i);
        if (glm::all(glm::lessThanEqual(glm::abs(vec[i]), Vec3(0.0001f))))
            vec[i] = Vec3(0,0,-1);
        vec[i] = MATH::normalize_safe(vec[i]);
    }
    for (; i<vec.size(); ++i)
        vec[i] = Vec3(0,0,-1);
}

QString MicrophoneGroup::infoString() const
{
    const auto m = paramMatrix_->baseValue();
    std::vector<Vec3> vec(paramNumMics_->baseValue());
    getMicVectors(vec, m);
    const Float dist = paramMicDist_->baseValue();

    std::stringstream s;
    s << "mic pos: ";
    for (size_t i=0; i<vec.size(); ++i)
    {
        if (i != 0)
            s << ", ";
        s << (vec[i] * dist);
    }
    return QString::fromStdString(s.str()).toHtmlEscaped();
}


} // namespace MO
