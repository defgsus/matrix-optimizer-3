/** @file audiosource.cpp

    @brief Audio source belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/2/2014</p>
    <p>really started 7/23/2014</p>
*/

#include "audiosource.h"

namespace MO {
namespace AUDIO {


AudioSource::AudioSource(const QString& id, Object *parent)
    : object_ (parent),
      idName_ (id)
{
}


void AudioSource::setNumberThreads(int num)
{
    transformation_.resize(num);
    sample_.resize(num);

    for (auto t : transformation_)
        t = Mat4(1.0);

    for (auto &s : sample_)
        s = 0.f;
}



} // namespace AUDIO
} // namespace MO
