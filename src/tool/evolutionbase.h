/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_TOOL_EVOLUTIONBASE_H
#define MOSRC_TOOL_EVOLUTIONBASE_H

#include "types/refcounted.h"

class QImage;

namespace MO {

struct MutationSettings
{
    MutationSettings(double prob = 0.1, double amt = 0.1)
        : probability(prob), amount(amt)
    { }

    double
        probability,
        amount;
};

/** Base class for evolutionary framework */
class EvolutionBase : public RefCounted
{
public:
    EvolutionBase() { }

    virtual void randomize() = 0;
    virtual void mutate(const MutationSettings*) = 0;
    virtual void mate(const EvolutionBase* other) = 0;

    virtual void getImage(QImage& img) const = 0;
};

} // namespace MO

#endif // MOSRC_TOOL_EVOLUTIONBASE_H
