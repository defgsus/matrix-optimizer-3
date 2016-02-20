/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/19/2016</p>
*/

#ifndef MOSRC_OBJECT_UTIL_PARAMETEREVOLUTION_H
#define MOSRC_OBJECT_UTIL_PARAMETEREVOLUTION_H

#include "tool/evolutionbase.h"
#include "types/float.h"

namespace MO {
class Object;
class Parameter;

/**  */
class ParameterEvolution : public EvolutionBase
{
public:

    // --- ctor ---

    ParameterEvolution(Object* = nullptr);
    ~ParameterEvolution();
    virtual const QString& className() const override
        { static const QString s("ParameterEvolution"); return s; }
    virtual void copyFrom(const EvolutionBase* o) override;
    virtual ParameterEvolution * createClone() const
        { auto e = new ParameterEvolution(); e->copyFrom(this); return e; }
    virtual bool isCompatible(const EvolutionBase* other) const override;

    // -- mutation --

    virtual void randomize() override;
    virtual void mutate() override;
    virtual void mate(const EvolutionBase* other) override;

    // --- render ---

    virtual void getImage(QImage& img) const override;

    void getRgb(double u, double v, double* r, double* g, double* b) const;

    /* */
    virtual QString toString() const override;

    // --- io ---

    virtual void serialize(QJsonObject&) const override;
    virtual void deserialize(const QJsonObject&) override;

    // --- params ---

    Object* object() const;

    /** Creates/Overwrites an evolution entry for the Parameter */
    void setParameter(Parameter*);

    void updateFromObject();
    void applyParametersToObject(bool updateGui) const;

private:
    struct Private;
    Private* p_;
};


} // namespace MO


#endif // MOSRC_OBJECT_UTIL_PARAMETEREVOLUTION_H
