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

namespace MO {
namespace AUDIO {

/** Impulse-Response map */
class IrMap
{
public:
    IrMap();

    // ----------- getter -------------

    /** Returns an informative string */
    QString getInfo() const;

    QImage getImage(const QSize& res);

    // ----------- setter -------------

    void clear();

    void addSample(Float distance, Float amplitude);

private:

    std::map<Float, Float> p_map_;
    Float p_min_amp_, p_max_amp_,
          p_min_dist_, p_max_dist_;
};

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_IRMAP_H
