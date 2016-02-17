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

class Properties;

/*
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
*/

/** Base class for evolutionary framework */
class EvolutionBase : public RefCounted
{
public:
    EvolutionBase();
    ~EvolutionBase();

    /** Create a new instance, as complete copy. */
    virtual EvolutionBase * createClone() const = 0;
    virtual void copyFrom(const EvolutionBase*);

    virtual void randomize() = 0;
    virtual void mutate() = 0;
    virtual bool mate(const EvolutionBase* other) = 0;

    virtual void getImage(QImage& img) const = 0;

    const Properties& properties() const { return *p_props_; }
    Properties& properties() { return *p_props_; }

private:
    Properties* p_props_;
};

/** Base class with std::vector<double>.
    Properties are
    seed (uint)
    mutation_amount,
    mutation_prob,
    init_mean,
    init_dev (all double)
    */
class EvolutionVectorBase : public EvolutionBase
{
public:
    EvolutionVectorBase(size_t size);
    virtual EvolutionVectorBase * createClone() const
        { auto e = new EvolutionVectorBase(0); e->copyFrom(this); return e; }
    virtual void copyFrom(const EvolutionBase* o) override
    {
        EvolutionBase::copyFrom(o);
        if (auto e = dynamic_cast<const EvolutionVectorBase*>(o))
            p_vec_ = e->p_vec_;
    }

    virtual void randomize() override;
    virtual void mutate() override;
    virtual bool mate(const EvolutionBase* other) override;

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
