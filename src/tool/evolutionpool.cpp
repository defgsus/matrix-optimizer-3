/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#include <vector>

#include <QImage>

#include "evolutionpool.h"
#include "evolutionbase.h"
#include "math/random.h"

namespace MO {

struct EvolutionPool::Private
{
    Private(EvolutionPool* p)
        : p          (p)
        , base       (0)
        , imgRes     (32, 32)
    { }

    ~Private()
    {
        if (base)
            base->releaseRef();
    }

    struct Tile
    {
        Tile(EvolutionBase*i=0) : instance(i), dirty(true) { }
        ~Tile() { if (instance) instance->releaseRef(); }

        EvolutionBase* instance;
        QImage image;
        bool dirty;
    };

    void rndSeed();
    void renderTile(Tile&);
    void renderTiles();

    EvolutionPool* p;
    std::vector<Tile> tiles;
    EvolutionBase* base;
    MutationSettings set;
    MATH::Twister rnd;
    QSize imgRes;
};

EvolutionPool::EvolutionPool(size_t num)
    : p_        (new Private(this))
{
    resize(num);
}

EvolutionPool::~EvolutionPool()
{
    resize(0);
    delete p_;
}

size_t EvolutionPool::size() const { return p_->tiles.size(); }
EvolutionBase* EvolutionPool::baseType() const { return p_->base; }
EvolutionBase* EvolutionPool::specimen(size_t idx) const
{
    return p_->tiles[idx].instance;
}
const QImage& EvolutionPool::image(size_t idx) const
{
    return p_->tiles[idx].image;
}

void EvolutionPool::setBaseType(EvolutionBase* base)
{
    if (p_->base)
        p_->base->releaseRef();
    p_->base = base;
    if (p_->base)
        p_->base->addRef();
}

void EvolutionPool::setSpecimen(size_t idx, EvolutionBase* evo)
{
    if (p_->tiles[idx].instance)
        p_->tiles[idx].instance->releaseRef();
    p_->tiles[idx].instance = evo;
    evo->addRef();
}

void EvolutionPool::setMutationSettings(const MutationSettings& m)
{
    p_->set = m;
    p_->rnd.setSeed(p_->set.seed);
}

void EvolutionPool::setImageResolution(const QSize &res)
{
    if (p_->imgRes == res)
        return;
    p_->imgRes = res;
    p_->renderTiles();
}

void EvolutionPool::Private::rndSeed()
{
    set.seed = rnd.getUInt32();
}

EvolutionBase* EvolutionPool::createSpecimen() const
{
    if (!p_->base)
        return nullptr;
    auto c = p_->base->createClone();
    c->randomize(&p_->set);
    return c;
}

void EvolutionPool::resize(size_t num)
{
    if (num == p_->tiles.size())
        return;

    // remove back
    if (num < p_->tiles.size())
    {
        p_->tiles.resize(num);
    }
    else
    // add to back
    while (num > p_->tiles.size())
    {
        auto c = createSpecimen();
        p_->tiles.push_back(Private::Tile(0));
        p_->tiles.back().instance = c;
    }

    p_->renderTiles();
}

void EvolutionPool::randomize()
{
    for (Private::Tile& t : p_->tiles)
    {
        if (t.instance == 0)
            t.instance = createSpecimen();
        if (!t.instance)
            continue;

        p_->rndSeed();
        t.instance->randomize(&p_->set);
        t.dirty = true;
    }
}

void EvolutionPool::repopulateFrom(size_t idx)
{
    auto src = specimen(idx);
    if (!src)
        return;

    for (auto& e : p_->tiles)
    if (src != e.instance)
    {
        if (e.instance)
            e.instance->releaseRef();

        e.instance = src->createClone();
        p_->rndSeed();
        e.instance->mutate(&p_->set);
        e.dirty = true;
    }

    p_->renderTiles();
}

void EvolutionPool::Private::renderTile(Tile& tile)
{
    if (tile.image.size() != imgRes)
    {
        tile.image = QImage(imgRes, QImage::Format_ARGB32_Premultiplied);
        tile.image.fill(Qt::red);
    }

    if (tile.instance)
    {
        tile.instance->getImage(tile.image);
        tile.dirty = false;
    }
}

void EvolutionPool::Private::renderTiles()
{
    for (auto& t : tiles)
    {
        if (t.dirty || t.image.size() != imgRes)
            renderTile(t);
    }
}



} // namespace MO
