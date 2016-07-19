/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/19/2016</p>
*/

#ifndef MOSRC_TOOL_BRAINFEVOLUTION_H
#define MOSRC_TOOL_BRAINFEVOLUTION_H

#include <algorithm>
#include <cmath>

#include "tool/EvolutionBase.h"
#include "tool/Brainf.h"

namespace MO {
namespace MATH { class Twister; }

/**  */
class BrainfEvolution : public EvolutionBase
{
public:

    typedef uint8_t Int;

    // --- ctor ---

    BrainfEvolution(size_t codeSize = 10, size_t inputSize = 10,
                    size_t maxCodeSize = 10000, size_t maxInputSize = 10000);
    virtual const QString& className() const override
        { static const QString s("BrainfEvolution"); return s; }
    virtual void copyFrom(const EvolutionBase* o) override;
    virtual BrainfEvolution * createClone() const
        { auto e = new BrainfEvolution(); e->copyFrom(this); return e; }
    virtual bool isCompatible(const EvolutionBase* other) const override;

    // -- mutation --

    virtual void randomize() override;
    virtual void mutate() override;
    virtual void mate(const EvolutionBase* other) override;

    // --- render ---

    virtual void getImage(QImage& img) const override;

    // --- io ---

    virtual void serialize(QJsonObject&) const override;
    virtual void deserialize(const QJsonObject&) override;

    /** */
    virtual QString toString() const override;

    // --- params ---

    const std::vector<BrainfOpcode>& code() const { return p_code_; }
    const std::vector<Int>& input() const { return p_input_; }
    BrainfOpcode code(size_t idx) const { return p_code_[idx]; }
    Int input(size_t idx) const { return p_input_[idx]; }
protected:
    std::vector<BrainfOpcode>& code() { return p_code_; }
    std::vector<Int>& input() { return p_input_; }
    BrainfOpcode& code(size_t idx) { return p_code_[idx]; }
    Int& input(size_t idx) { return p_input_[idx]; }

    BrainfOpcode randomOpcode(MATH::Twister& rnd);
    void makeSaneCode(std::vector<BrainfOpcode>& code) const;

private:
    size_t p_codeSize_, p_inputSize_,
            p_maxCodeSize_, p_maxInputSize_;
    std::vector<BrainfOpcode> p_code_;
    std::vector<Int> p_input_;
    bool p_isInputLoop_;
};


} // namespace MO

#endif // MOSRC_TOOL_BRAINFEVOLUTION_H
