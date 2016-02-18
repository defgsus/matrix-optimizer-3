/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#ifndef MOSRC_MATH_KALISETEVOLUTION_H
#define MOSRC_MATH_KALISETEVOLUTION_H

#include <algorithm>
#include <cmath>

#include "tool/evolutionbase.h"

namespace MO {

/** The infamous Kali set as specimen */
class KaliSetEvolution : public EvolutionVectorBase
{
public:

    // --- ctor ---

    virtual const QString& className() const override
        { static const QString s("KaliSetEvolution"); return s; }
    KaliSetEvolution();
    virtual KaliSetEvolution * createClone() const
        { auto e = new KaliSetEvolution(); e->copyFrom(this); return e; }

    // --- render ---

    virtual void getImage(QImage& img) const override;

    void getRgb(double u, double v, double* r, double* g, double* b) const;

    // --- io ---

    virtual void serialize(QJsonObject&) const override;
    virtual void deserialize(const QJsonObject&) override;

    /** Returns GLSL */
    virtual QString toString() const override;

    // --- params ---

    /** Number iterations based on gene length */
    int pNumIter() const { return (vector().size() - 17) / 3; }

    double pColAccX() const { return vector(0); }
    double pColAccY() const { return vector(1); }
    double pColAccZ() const { return vector(2); }

    double pMinAccX() const { return vector(3); }
    double pMinAccY() const { return vector(4); }
    double pMinAccZ() const { return vector(5); }

    double pMinAmtX() const { return vector(6); }
    double pMinAmtY() const { return vector(7); }
    double pMinAmtZ() const { return vector(8); }
    double pMinExp() const { return std::abs(vector(9)) * 20.; }

    double pAmtX() const { return vector(10); }
    double pAmtY() const { return vector(11); }
    double pAmtZ() const { return vector(12); }

    double pScale() const { return std::pow(vector(13), 2.); }
    double pPosX() const { return vector(14); }
    double pPosY() const { return vector(15); }
    double pPosZ() const { return vector(16); }

    double pMagicX(size_t i) const { return std::abs(vector(17+i*3)); }
    double pMagicY(size_t i) const { return std::abs(vector(18+i*3)); }
    double pMagicZ(size_t i) const { return std::abs(vector(19+i*3)); }
};


} // namespace MO

#endif // MOSRC_MATH_KALISETEVOLUTION_H
