/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#include <QApplication>
#include <QClipboard>

#include "EvolutionMimeData.h"
#include "tool/EvolutionBase.h"

namespace MO {

const QString EvolutionMimeData::mimeTypeString
    = "application/matrixoptimizer.3.evolution";

bool EvolutionMimeData::isInClipboard()
{
    if (QApplication::clipboard())
        if (auto md = QApplication::clipboard()->mimeData())
            return md->hasFormat(mimeTypeString);
    return false;
}

EvolutionMimeData* EvolutionMimeData::evolutionMimeData(QMimeData* d)
{
    return d->hasFormat(mimeTypeString)
        ? static_cast<EvolutionMimeData*>(d)
        : 0;
}

const EvolutionMimeData * EvolutionMimeData::evolutionMimeData(const QMimeData* d)
{
    return d && d->hasFormat(mimeTypeString)
        ? static_cast<const EvolutionMimeData*>(d)
        : 0;
}


void EvolutionMimeData::setSpecimen(EvolutionBase * evo)
{
    auto s = evo->toJsonString();
    setData(mimeTypeString, s.toUtf8());
    setText(s);
}

EvolutionBase* EvolutionMimeData::createSpecimen() const
{
    QByteArray a = data(mimeTypeString);
    return EvolutionBase::fromJsonString(text());
}


} // namespace MO
