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
#include "types/properties.h"

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
        Tile(EvolutionBase*i=nullptr) : instance(i), dirty(true) { }
        ~Tile() { if (instance) instance->releaseRef(); }
        Tile(const Tile& o) : Tile(nullptr) { *this = o; }
        Tile& operator=(const Tile& o)
        {
            if (o.instance)
                o.instance->addRef();
            if (instance)
                instance->releaseRef();
            instance = o.instance;
            dirty = o.dirty;
            if (!dirty)
                image = o.image;
            return *this;
        }

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
    MATH::Twister rnd;
    QSize imgRes;
    Properties props;
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

Properties EvolutionPool::properties() const
{
    Properties p;
    for (auto& t : p_->tiles)
        p.unify(t.instance->properties());
    return p;
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

void EvolutionPool::setProperties(const Properties& m, bool keepSeed)
{
    auto pseed = p_->props.get("seed").toUInt();
    p_->props = m;
    if (!keepSeed)
        p_->rnd.setSeed(m.get("seed").toUInt());
    else
        p_->props.set("seed", pseed);

    /*
    for (auto& t : p_->tiles)
        if (t.instance)
            t.instance->properties().updateFrom(p_->props);
            */
}

void EvolutionPool::renderTiles() { p_->renderTiles(); }
void EvolutionPool::setImageResolution(const QSize &res)
{
    if (p_->imgRes == res)
        return;
    p_->imgRes = res;
}

void EvolutionPool::Private::rndSeed()
{
    props.set("seed", rnd.getUInt32());
}

EvolutionBase* EvolutionPool::createSpecimen() const
{
    if (!p_->base)
        return nullptr;
    p_->rndSeed();
    p_->base->properties().updateFrom(p_->props);
    auto c = p_->base->createClone();
    c->randomize();
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
        p_->tiles.push_back(Private::Tile(c));
    }
}

void EvolutionPool::randomize()
{
    for (Private::Tile& t : p_->tiles)
    {
        if (t.instance == 0)
        {
            t.instance = createSpecimen();
            t.dirty = true;
            continue;
        }

        p_->rndSeed();
        t.instance->properties().updateFrom(p_->props);
        t.instance->randomize();
        t.dirty = true;
    }
}

void EvolutionPool::repopulateFrom(size_t idx)
{
    auto src = specimen(idx);
    if (!src)
        return;

    src->properties().unify(p_->props);


    for (auto& e : p_->tiles)
    if (src != e.instance)
    {
        if (e.instance)
            e.instance->releaseRef();

        e.instance = src->createClone();
        p_->rndSeed();
        e.instance->properties().updateFrom(p_->props);
        e.instance->mutate();
        e.dirty = true;
    }
}

void EvolutionPool::Private::renderTile(Tile& tile)
{
    if (tile.image.size() != imgRes)
    {
        tile.image = QImage(imgRes, QImage::Format_ARGB32_Premultiplied);
    }

    if (tile.instance)
    {
        tile.instance->getImage(tile.image);
        tile.dirty = false;
    }
    else
        tile.image.fill(Qt::red);
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
