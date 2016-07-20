/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/20/2016</p>
*/

#include <QString>
#include <QDataStream>

#include "Fraction.h"


namespace MO {
namespace MATH {

QString Fraction::toString() const
{
    return QString("%1/%2").arg(nom).arg(denom);
}

QDataStream& operator<<(QDataStream& io, const Fraction& f)
{
    io << (qint64)f.nom << (qint64)f.denom;
    return io;
}

QDataStream& operator>>(QDataStream& io, Fraction& f)
{
    qint64 n, d;
    io >> n >> d;
    f.nom = n;
    f.denom = d;
    return io;
}

} // namespace MATH
} // namespace MO

