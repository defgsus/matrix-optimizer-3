/** @file irmap.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 29.04.2015</p>
*/

#ifndef MOSRC_AUDIO_TOOL_IRMAP_H
#define MOSRC_AUDIO_TOOL_IRMAP_H

#include <map>

#include <QImage>

#include "types/float.h"
#include "math/timeline1d.h"

namespace MO {
namespace AUDIO {

/** Impulse-Response map */
class IrMap
{
public:
    IrMap();

    // ------------- io ---------------

    /** Save as waveform, throws. */
    void saveWav(const QString& filename, Float sample_rate, Float speed_of_sound = 333.);

    // ----------- getter -------------

    bool isEmpty() const;

    size_t numSamples() const;

    /** Returns an informative string */
    QString getInfo() const;

    QImage getImage(const QSize& res);

    /** Returns a waveform from the sampled map */
    std::vector<F32> getSamples(Float sample_rate, Float speed_of_sound = 333.);

    // ----------- setter -------------

    void clear();

    void addSample(Float distance, Float amplitude);

private:

    static const Float p_convert_;

#ifdef MO_IRMAP_TL
    MATH::Timeline1D p_tl_;
#else
    std::map<Float, Float> p_map_;
#endif
    Float p_min_amp_, p_max_amp_,
          p_min_dist_, p_max_dist_;
};

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_IRMAP_H
