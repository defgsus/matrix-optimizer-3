/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/20/2016</p>
*/

#ifndef MOSRC_MATH_FRACTION_H
#define MOSRC_MATH_FRACTION_H

#include <cinttypes>

class QString;

class QDataStream;

namespace MO {
namespace MATH {

/** Light-weight fractional number representation */
struct Fraction
{
    int64_t nom, denom;

    explicit Fraction(int64_t nom = 0, int64_t denom = 0)
        : nom(nom), denom(denom)
    { }

    double value() const { return denom ? double(nom) / denom : 0.; }

    QString toString() const;

    bool operator == (const Fraction& o) const { return o.nom == nom && o.denom == denom; }
    bool operator != (const Fraction& o) const { return o.nom != nom || o.denom != denom; }
    bool operator < (const Fraction& o) const { return value() < o.value(); }
    bool operator <= (const Fraction& o) const { return *this == o || value() < o.value(); }
    bool operator > (const Fraction& o) const { return value() > o.value(); }
    bool operator >= (const Fraction& o) const { return *this == o || value() > o.value(); }

};

QDataStream& operator<<(QDataStream&, const Fraction& f);
QDataStream& operator>>(QDataStream&, Fraction& f);


} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_FRACTION_H

