/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#ifndef MOSRC_MATH_KALISETEVOLUTION_H
#define MOSRC_MATH_KALISETEVOLUTION_H

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

    /** Returns GLSL */
    virtual QString toString() const override;

    /** Number iterations based on gene length */
    int numIter() const;
};


} // namespace MO

#endif // MOSRC_MATH_KALISETEVOLUTION_H
