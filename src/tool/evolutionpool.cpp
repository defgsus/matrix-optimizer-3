/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#include <vector>

#include <QImage>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#include "evolutionpool.h"
#include "evolutionbase.h"
#include "math/random.h"
#include "types/properties.h"
#include "tool/generalimage.h"
#include "io/error.h"
#include "io/time.h"

namespace MO {

struct EvolutionPool::Private
{
    Private(EvolutionPool* p)
        : p          (p)
        , imgRes     (32, 32)
    { }

    ~Private()
    {
    }

    struct Tile
    {
        Tile(EvolutionBase*i=nullptr) : instance(i), dirty(true), isLocked(false) { }
        ~Tile() { if (instance) instance->releaseRef("EvolutionPool tile destroy"); }
        Tile(const Tile& o) : Tile(nullptr) { *this = o; }
        Tile& operator=(const Tile& o)
        {
            if (o.instance)
                o.instance->addRef("EvolutionPool tile copyfrom");
            if (instance)
                instance->releaseRef("EvolutionPool tile copyfrom relprev");
            instance = o.instance;
            dirty = o.dirty;
            if (!dirty)
                image = o.image;
            isLocked = o.isLocked;
            return *this;
        }
        void setInstance(EvolutionBase*evo)
        {
            if (instance)
                instance->releaseRef("EvolutionPool tile set");
            instance = evo;
            dirty = true;
        }

        EvolutionBase* instance;
        QImage image;
        bool dirty, isLocked;
    };

    void rndSeed(EvolutionBase* evo);
    void renderTile(Tile&);
    void renderTiles();

    EvolutionPool* p;
    std::vector<Tile> tiles;
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

void EvolutionPool::serialize(QJsonObject& obj) const
{
    obj.insert("pool_size", QJsonValue((int)size()));
    for (size_t i=0; i<p_->tiles.size(); ++i)
    if (auto evo = p_->tiles[i].instance)
    {
        QJsonObject evoObj;
        evoObj.insert("locked", p_->tiles[i].isLocked);
        evo->toJson(evoObj);

        obj.insert(QString::number(i), evoObj);
    }
}

void EvolutionPool::deserialize(const QJsonObject& obj)
{
    if (!obj.contains("pool_size"))
        MO_IO_ERROR(READ, "'pool_size' not found in json");
    resize(obj.value("pool_size").toInt(), true);
    for (size_t i=0; i<size(); ++i)
    {
        auto evoVal = obj.value(QString::number(i));
        if (evoVal.isObject())
        {
            auto evoObj = evoVal.toObject();
            auto evo = EvolutionBase::fromJson(evoObj);
            setSpecimen(i, evo);
            setLocked(i, evoObj.value("locked").toBool());
            if (evo)
                evo->releaseRef("EvolutionPool deserialize finish");
        }
    }
}

QString EvolutionPool::toJsonString() const
{
    QJsonObject obj;
    serialize(obj);
    return QString::fromUtf8(QJsonDocument(obj).toJson());
}

void EvolutionPool::loadJsonString(const QString &str)
{
    QJsonObject obj = QJsonDocument::fromJson(str.toUtf8()).object();
    deserialize(obj);
}

void EvolutionPool::loadJsonFile(const QString &fn)
{
    QFile file(fn);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        MO_IO_ERROR(READ, "Could not open json file\n'"
                    << fn << "'\n" << file.errorString());

    auto doc = QJsonDocument::fromJson(file.readAll());
    deserialize(doc.object());
}

void EvolutionPool::saveJsonFile(const QString& fn) const
{
    QFile file(fn);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        MO_IO_ERROR(WRITE, "Could not open json file for saving\n'"
                    << fn << "'\n" << file.errorString());

    QJsonObject obj;
    serialize(obj);

    auto data = QJsonDocument(obj).toJson();
    if (file.write(data.constData(), data.size()) != data.size())
        MO_IO_ERROR(WRITE, "Could not write to json file\n'"
                    << fn << "'\n" << file.errorString());
}

size_t EvolutionPool::size() const { return p_->tiles.size(); }
bool EvolutionPool::isLocked(size_t idx) const
    { return idx < p_->tiles.size() ? p_->tiles[idx].isLocked : false; }
size_t EvolutionPool::numLocked() const
{
    size_t n = 0;
    for (auto& t : p_->tiles)
        if (t.isLocked)
            ++n;
    return n;
}
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
        if (t.instance)
            p.unify(t.instance->properties());
    return p;
}

void EvolutionPool::setSpecimen(size_t idx, EvolutionBase* evo)
{
    p_->tiles[idx].setInstance(evo);
    if (p_->tiles[idx].instance)
        p_->tiles[idx].instance->addRef("EvolutionPool set specimen");
}

void EvolutionPool::setSpecimen(size_t idx, const QString& className)
{
    auto evo = EvolutionBase::createClass(className);
    if (evo)
    {
        p_->rndSeed(evo);
        evo->randomize();
    }
    setSpecimen(idx, evo);
}

void EvolutionPool::setLocked(size_t idx, bool locked)
{
    if (idx < p_->tiles.size())
        p_->tiles[idx].isLocked = locked;
}

void EvolutionPool::setRandomSeed()
{
    p_->rnd.setSeed(long(systemTime()) % 0xffff);
}

void EvolutionPool::setProperties(const Properties& m, bool keepSeed)
{
    auto pseed = p_->props.get("seed").toUInt();
    p_->props = m;
    if (!keepSeed)
        p_->rnd.setSeed(m.get("seed").toUInt());
    else
        p_->props.set("seed", pseed);

    // overwrite all properties
    for (auto& t : p_->tiles)
        if (t.instance)
            t.instance->properties().updateFrom(p_->props);
}

void EvolutionPool::renderTiles() { p_->renderTiles(); }
void EvolutionPool::setImageResolution(const QSize &res)
{
    if (p_->imgRes == res)
        return;
    p_->imgRes = res;
}

void EvolutionPool::Private::rndSeed(EvolutionBase* evo)
{
    props.set("seed", rnd.getUInt32());
    evo->properties().set("seed", props.get("seed"));
}

EvolutionBase* EvolutionPool::createSpecimen()
{
    auto base = getRandomSpecimen();
    if (!base)
        return nullptr;
    std::vector<EvolutionBase*> vec;
    vec.push_back(base);
    return createSpecimen(vec);
}

EvolutionBase* EvolutionPool::createSpecimen(const std::vector<EvolutionBase*>& vec)
{
    if (vec.empty())
        return nullptr;
    auto base = vec[p_->rnd.getUInt32() % vec.size()];
    if (!base)
        return nullptr;
    auto c = base->createClone();
    c->properties().updateFrom(p_->props);

    p_->rndSeed(c);
    c->randomize();
    return c;
}
EvolutionBase* EvolutionPool::createSpecimen(const std::vector<const EvolutionBase*>& vec)
{
    if (vec.empty())
        return nullptr;
    auto base = vec[p_->rnd.getUInt32() % vec.size()];
    if (!base)
        return nullptr;
    auto c = base->createClone();
    c->properties().updateFrom(p_->props);

    p_->rndSeed(c);
    c->randomize();
    return c;
}

EvolutionBase* EvolutionPool::getRandomSpecimen()
{
    std::vector<EvolutionBase*> all;
    for (auto& t : p_->tiles)
        if (t.instance)
            all.push_back(t.instance);
    if (all.empty())
        return nullptr;
    size_t idx = p_->rnd.getUInt32() % all.size();
    return all[idx];
}

std::vector<EvolutionBase*> EvolutionPool::getSpecies() const
{
    std::vector<EvolutionBase*> v;
    for (auto& t : p_->tiles)
        if (t.instance)
            v.push_back(t.instance);
    return v;
}

void EvolutionPool::resize(size_t num, bool doClear)
{
    if (doClear)
    {
        for (auto& t : p_->tiles)
        if (t.instance)
        {
            t.instance->releaseRef("EvolutionPool resize clearprev");
            t.instance = 0;
            t.dirty = true;
        }
        p_->tiles.resize(num);
        return;
    }

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

void EvolutionPool::repopulate()
{
    auto vec = getSpecies();
    if (vec.empty())
        return;

    for (auto v : vec)
        v->addRef("EvolutionPool::repopulate()");

    for (size_t i=0; i<size(); ++i)
    if (!isLocked(i))
    {
        auto evo = createSpecimen(vec);
        setSpecimen(i, evo);
    }

    for (auto v : vec)
        v->releaseRef("EvolutionPool::repopulate()");
}

void EvolutionPool::repopulate(const EvolutionBase* base)
{
    if (!base)
    {
        for (size_t i=0; i<size(); ++i)
            if (!isLocked(i))
                setSpecimen(i, 0);
        return;
    }

    std::vector<const EvolutionBase*> vec;
    vec.push_back(base);

    for (size_t i=0; i<size(); ++i)
    if (!isLocked(i))
    {
        auto evo = createSpecimen(vec);
        setSpecimen(i, evo);
    }
}

void EvolutionPool::crossBreed()
{
    std::vector<EvolutionBase*> parents;
    for (auto& t : p_->tiles)
    if (t.instance && t.isLocked)
    {
        parents.push_back(t.instance);
        t.instance->addRef("EvolutionPool::crossBreed()");
    }

    for (auto& t : p_->tiles)
    if (!t.isLocked)
    {
        auto evo = createOffspring(parents);
        if (evo)
            t.setInstance(evo);
    }

    for (auto v : parents)
        v->releaseRef("EvolutionPool::crossBreed()");
}

void EvolutionPool::crossBreed(size_t idx)
{
    if (idx >= p_->tiles.size())
        return;
    auto& jovani = p_->tiles[idx];

    std::vector<EvolutionBase*> parents;
    for (auto& t : p_->tiles)
    if (t.instance && t.instance != jovani.instance && t.isLocked)
    {
        parents.push_back(t.instance);
        t.instance->addRef("EvolutionPool::crossBreed()");
    }

    if (parents.size() > 0)
    {
        size_t k=0;
        for (auto& t : p_->tiles)
        if (k++ != idx && !t.isLocked)
        {
            std::vector<EvolutionBase*> twoParents;
            twoParents.push_back(jovani.instance);
            twoParents.push_back(parents[p_->rnd.getUInt32() % parents.size()]);
            auto evo = createOffspring(twoParents);
            t.setInstance(evo);
        }
    }
    for (auto v : parents)
        v->releaseRef("EvolutionPool::crossBreed()");
}

void EvolutionPool::repopulateFrom(size_t idx)
{
    auto src = specimen(idx);
    if (!src)
        return;

    src->properties().unify(p_->props);

    for (auto& e : p_->tiles)
    if (src != e.instance && !e.isLocked)
    {
        e.setInstance( src->createClone() );
        p_->rndSeed(e.instance);
        e.instance->mutate();
        e.dirty = true;
    }
}

EvolutionBase* EvolutionPool::createOffspring(const std::vector<EvolutionBase*>& parents)
{
    if (parents.size() < 2)
        return nullptr;

    // choose one
    auto evo = parents[p_->rnd.getUInt32() % parents.size()];
    if (!evo)
        return nullptr;

    // find possible mates
    std::vector<EvolutionBase*> vec;
    for (auto v : parents)
        if (v && v != evo && evo->isCompatible(v))
            vec.push_back(v);
    if (vec.empty())
        return nullptr;

    // choose mate
    auto mate = vec[p_->rnd.getUInt32() % vec.size()];

    if (p_->rnd() < .5)
        std::swap(evo, mate);

    auto child = evo->createClone();
    p_->rndSeed(child);
    child->mate(mate);

    return child;
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
        GeneralImage::getTextImage(tile.image, QObject::tr("empty"),
                                   QColor(255,255,255), QColor(50,50,50));
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
