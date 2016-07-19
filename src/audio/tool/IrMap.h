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
#include "math/Timeline1d.h"

namespace MO {
namespace AUDIO {

/** Impulse-Response map */
class IrMap
{
public:

    struct Settings
    {
        Float directAmp,        //! Amplitude of direct signal
              patchSizeRefl,    //! Patch size in seconds per reflection
              patchSizeDist,    //! Patch size in seconds per distance unit
              patchExp,         //! Exponent of bell curve
              patchExpShrink,   //! Lower bell exponent per reflection
              sos;              //! Speed of sound (seconds per unit)
        bool doFlipPhase,       //! Flip phase on reflection
             doNormalizeLocal;
        size_t sampleRate;
    };




    IrMap();

    // ------------- io ---------------

    /** Save as waveform, throws. */
    void saveWav(const QString& filename);

    // ----------- getter -------------

    bool isEmpty() const;

    static Settings getDefaultSettings();

    const Settings& getSettings() const { return p_set_; }

    size_t numSamples() const;

    /** Returns an informative string */
    QString getInfo() const;

    /** Returns a waveform from the sampled map */
    std::vector<F32> getSamples();

    QImage getImage(const QSize& res);

    // ----------- setter -------------

    void clear();

    /** Applies new settings, call before getSamples() */
    void setSettings(const Settings& s) { p_set_ = s; }

    void addSample(Float distance, Float amplitude, short numReflect);

    // ---------- helper ---------------

    /** Returns the curve sample for writing an impulse, t [0,1] */
    F32 getPatchSample(F32 t, int numRefl) const;
    size_t getPatchSize(int numRefl, F32 dist) const;

private:

    static const Float p_convert_;

#ifdef MO_IRMAP_TL
    MATH::Timeline1D p_tl_;
#else
    std::map<Float, std::pair<Float, short int>> p_map_;
    std::vector<Float> p_impulse_;
#endif
    Float p_min_amp_, p_max_amp_,
          p_min_dist_, p_max_dist_;
    short int p_min_refl_, p_max_refl_;
    Settings p_set_;
};

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_IRMAP_H
