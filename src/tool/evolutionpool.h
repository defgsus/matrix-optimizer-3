/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#ifndef MOSRC_TOOL_EVOLUTIONPOOL_H
#define MOSRC_TOOL_EVOLUTIONPOOL_H

#include <cstddef>

class QSize;
class QImage;

namespace MO {

class EvolutionBase;
struct MutationSettings;

class EvolutionPool
{
public:

    EvolutionPool(size_t num = 0);
    ~EvolutionPool();

    size_t size() const;
    EvolutionBase* baseType() const;
    /** Returns new random instance of baseType(),
        or NULL if baseType() is NULL */
    EvolutionBase* createSpecimen() const;

    void setMutationSettings(const MutationSettings&, bool keepSeed = true);
    void setBaseType(EvolutionBase* base);
    /** Adds reference */
    void setSpecimen(size_t idx, EvolutionBase* base);

    void resize(size_t num);
    /** Fills pool with random base types */
    void randomize();
    void repopulateFrom(size_t idx);

    /** Access to each instance */
    EvolutionBase* specimen(size_t idx) const;

    /** Updates changed tiles */
    void renderTiles();
    void setImageResolution(const QSize& res);
    const QImage& image(size_t idx) const;

private:

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // MOSRC_TOOL_EVOLUTIONPOOL_H
