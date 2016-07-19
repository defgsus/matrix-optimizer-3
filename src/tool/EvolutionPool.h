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


/** A pool working on/with EvolutionBase* instances */
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

    /** Number of slots in pool */
    size_t size() const;

    /** Constructs a union of all species */
    Properties properties() const;

    /** Is the given slot locked */
    bool isLocked(size_t idx) const;
    /** Number of locked specimen */
    size_t numLocked() const;

    /** Access to each instance */
    EvolutionBase* specimen(size_t idx) const;

    /** Returns a vector of all non-NULL specimen */
    std::vector<EvolutionBase*> getSpecies() const;

    // ------- setter -------

    /** Sets a random random seed */
    void setRandomSeed();

    /** Sets the properties. Unifies with all instances */
    void setProperties(const Properties&, bool keepSeed = true);

    /** Sets a specimen, adds reference to @p base */
    void setSpecimen(size_t idx, EvolutionBase* base);
    /** Sets a specimen by creating a new random instance of @p className */
    void setSpecimen(size_t idx, const QString& className);
    /** Sets the specimen locked or vogelfrei */
    void setLocked(size_t idx, bool locked);

    /** Resizes the pool.
        If @p doClear is true, clears the whole pool to NULL entries,
        If @p doClear is false and @p num > size(), the new free cells
        are populated with random instances, if there are any. */
    void resize(size_t num, bool doClear = false);

    // ------- mutation ---------

    /** Returns new random instance cloned from one of the pool species,
        or NULL if no instances in pool */
    EvolutionBase* createSpecimen();
    /** Returns a new random instance of one of the specimen in @p vec */
    EvolutionBase* createSpecimen(const std::vector<EvolutionBase*>& vec);
    /** Returns a new random instance of one of the specimen in @p vec */
    EvolutionBase* createSpecimen(const std::vector<const EvolutionBase*>& vec);
    /** Returns a random instance, or NULL if no instances in pool */
    EvolutionBase* getRandomSpecimen();

    /** Fills pool with new random instances of randomly cloned current content */
    void repopulate();
    /** Fills pool with new random instances cloned from @p base */
    void repopulate(const EvolutionBase* base);
    /** Fills the pool with new random instances cloned from slot @p idx */
    void repopulateFrom(size_t idx);
    /** Mates all locked specimen */
    void crossBreed();
    /** Mates all locked specimen with idx */
    void crossBreed(size_t idx);

    /** Creates an offspring from two randomly selected parents.
        Returns NULL, if impossible. */
    EvolutionBase* createOffspring(const std::vector<EvolutionBase*>& parents);

    // ------ rendering ---------

    /** Lazily renders tiles that need an update */
    void renderTiles();
    /** Sets a new resolution for rendered images */
    void setImageResolution(const QSize& res);
    /** Returns the image for the given slot.
        Call renderTiles() to update images before this function */
    const QImage& image(size_t idx) const;

private:

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // MOSRC_TOOL_EVOLUTIONPOOL_H
