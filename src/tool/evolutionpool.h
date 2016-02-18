/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#ifndef MOSRC_TOOL_EVOLUTIONPOOL_H
#define MOSRC_TOOL_EVOLUTIONPOOL_H

#include <cstddef>
#include <vector>

#include <QString>

class QSize;
class QImage;
class QJsonObject;

namespace MO {

class EvolutionBase;
class Properties;

class EvolutionPool
{
public:

    EvolutionPool(size_t num = 0);
    ~EvolutionPool();

    // --------- io ---------

    void serialize(QJsonObject&) const;
    void deserialize(const QJsonObject&);

    QString toJsonString() const;
    void loadJsonString(const QString& str);

    void saveJsonFile(const QString& fn) const;
    void loadJsonFile(const QString& fn);


    // ------ getter --------

    size_t size() const;
    /** Constructs a union of all species */
    Properties properties() const;

    bool isLocked(size_t idx) const;

    /** Returns new random instance of baseType(),
        or NULL if baseType() is NULL */
    EvolutionBase* createSpecimen();
    /** Returns a new random instance of one of the specimen in @p vec */
    EvolutionBase* createSpecimen(const std::vector<EvolutionBase*>& vec);
    /** Returns a random instance, or NULL if no instances in pool */
    EvolutionBase* getRandomSpecimen();
    /** Returns a vector of all non-NULL specimen */
    std::vector<EvolutionBase*> getSpecies() const;

    void setProperties(const Properties&, bool keepSeed = true);

    /** Adds reference */
    void setSpecimen(size_t idx, EvolutionBase* base);
    void setSpecimen(size_t idx, const QString& className);
    void setLocked(size_t idx, bool locked);

    void resize(size_t num, bool doClear = false);
    /** Fills pool with random base types */
    void repopulate();
    void repopulateFrom(size_t idx);
    /** Mates all locked tiles */
    void crossBreed();
    /** Creates an offspring from the potential parents */
    EvolutionBase* createOffspring(const std::vector<EvolutionBase*>& parents);

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
