/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_TOOL_EVOLUTIONBASE_H
#define MOSRC_TOOL_EVOLUTIONBASE_H

#include <vector>
#include <map>

#include <QStringList>

#include "types/refcounted.h"

class QImage;
class QJsonObject;
class QJsonDocument;

namespace MO {

class Properties;


#define MO_REGISTER_EVOLUTION(Class__) \
    namespace { static bool success_register_##Class__##_ = EvolutionBase::registerClass(new Class__); }

/** Base class for evolutionary framework */
class EvolutionBase : public RefCounted
{
public:
    EvolutionBase();
    ~EvolutionBase();

    // ---- ctor -----

    /** Create a new instance, as complete copy. */
    virtual EvolutionBase * createClone() const = 0;
    virtual const QString& className() const = 0;
    /** Default impl copies parameters() */
    virtual void copyFrom(const EvolutionBase*);
    /** Returns true when copyFrom() and mate() work with @p other */
    virtual bool isCompatible(const EvolutionBase* other) const = 0;

    // -- io --

    virtual void serialize(QJsonObject&) const = 0;
    virtual void deserialize(const QJsonObject&) = 0;

    /** Puts an object with key className() into @p obj */
    void toJson(QJsonObject& obj) const;
    static EvolutionBase* fromJson(const QJsonObject&obj);
    QString toJsonString() const;
    static EvolutionBase* fromJsonString(const QString&);

    void saveJsonFile(const QString& fn) const;
    static EvolutionBase* fromJsonFile(const QString& fn);

    /** Saves the image with attached json code */
    void saveImage(const QString& fn, const QImage& img);

    // --- mutation ---

    virtual void randomize() = 0;
    virtual void mutate() = 0;
    virtual void mate(const EvolutionBase* other) = 0;

    const Properties& properties() const { return *p_props_; }
    Properties& properties() { return *p_props_; }

    // -- rendering --

    /** Some string representation, default returns json string */
    virtual QString toString() const { return toJsonString(); }

    virtual void getImage(QImage& img) const = 0;

    // --- factory ---

    static bool registerClass(EvolutionBase*);
    static QStringList registeredClassNames();
    static EvolutionBase* createClass(const QString& className);

private:
    Properties* p_props_;
    static std::map<QString, EvolutionBase*> p_reg_map_;
};



/** Base class with std::vector<double>.
    Properties are
    @li seed (uint)
    @li mutation_amount
    @li mutation_prob
    @li grow_prob
    @li shrink_prob
    @li init_mean
    @li init_var
    @li init_dev (all double)
    */
class EvolutionVectorBase : public EvolutionBase
{
public:
    // -- ctor --

    /** If @p resizeable is true, properties for grow and shrink are created */
    EvolutionVectorBase(size_t size = 10, bool resizeable = false);

    virtual const QString& className() const override
        { static const QString s("EvolutionVectorBase"); return s; }
    virtual EvolutionVectorBase * createClone() const override
        { auto e = new EvolutionVectorBase(0); e->copyFrom(this); return e; }
    virtual void copyFrom(const EvolutionBase* o) override
    {
        EvolutionBase::copyFrom(o);
        if (auto e = dynamic_cast<const EvolutionVectorBase*>(o))
            p_vec_ = e->p_vec_;
    }

    virtual bool isCompatible(const EvolutionBase* other) const override;

    // -- io --

    virtual void serialize(QJsonObject&) const;
    virtual void deserialize(const QJsonObject&);

    // -- mutation --

    virtual void randomize() override;
    virtual void mutate() override;
    virtual void mate(const EvolutionBase* other) override;

    // -- render --

    /** Base impl. renders the values */
    virtual void getImage(QImage& img) const override;

    // --- getter ---

    const std::vector<double>& vector() const { return p_vec_; }
    double vector(size_t idx) const { return idx < p_vec_.size() ? p_vec_[idx] : 0.; }
protected:
    // --- setter ---

    std::vector<double>& vector() { return p_vec_; }
    double& vector(size_t idx) { return p_vec_[idx]; }

private:
    std::vector<double> p_vec_;
};




} // namespace MO

#endif // MOSRC_TOOL_EVOLUTIONBASE_H
