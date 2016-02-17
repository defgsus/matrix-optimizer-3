/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_TOOL_EVOLUTIONBASE_H
#define MOSRC_TOOL_EVOLUTIONBASE_H

#include <vector>
#include "types/refcounted.h"

class QImage;

namespace MO {

struct MutationSettings
{
    MutationSettings(double prob = 0.1, double amt = 0.1, uint32_t seed = 42)
        : probability(prob), amount(amt)
        , mean(0.), deviation(1.)
        , seed(seed)
    { }

    double
        probability,
        amount,
        mean,
        deviation;

    uint32_t seed;
};

/** Base class for evolutionary framework */
class EvolutionBase : public RefCounted
{
public:
    EvolutionBase() : RefCounted() { }

    /** Create a new instance, as complete copy. */
    virtual EvolutionBase * createClone() const = 0;
    virtual void copyFrom(const EvolutionBase*) = 0;

    virtual void randomize(const MutationSettings*) = 0;
    virtual void mutate(const MutationSettings*) = 0;
    virtual bool mate(const MutationSettings*, const EvolutionBase* other) = 0;

    virtual void getImage(QImage& img) const = 0;
};

/** Base class with std::vector<double>. */
class EvolutionVectorBase : public EvolutionBase
{
public:
    EvolutionVectorBase(size_t size) : p_vec_(size) { }
    virtual EvolutionVectorBase * createClone() const
        { auto e = new EvolutionVectorBase(0); e->copyFrom(this); return e; }
    virtual void copyFrom(const EvolutionBase* o) override
    {
        if (auto e = dynamic_cast<const EvolutionVectorBase*>(o))
            p_vec_ = e->p_vec_;
    }

    virtual void randomize(const MutationSettings*) override;
    virtual void mutate(const MutationSettings*) override;
    virtual bool mate(const MutationSettings*, const EvolutionBase* other) override;

    /** Base impl. renders the values */
    virtual void getImage(QImage& img) const override;

    // --- getter ---
    const std::vector<double>& vector() const { return p_vec_; }
    double vector(size_t idx) const { return p_vec_[idx]; }
protected:
    // --- setter ---
    std::vector<double>& vector() { return p_vec_; }
    double& vector(size_t idx) { return p_vec_[idx]; }

private:
    std::vector<double> p_vec_;
};

} // namespace MO

#endif // MOSRC_TOOL_EVOLUTIONBASE_H
