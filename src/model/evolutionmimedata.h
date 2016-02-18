/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#ifndef MOSRC_MODEL_EVOLUTIONMIMEDATA_H
#define MOSRC_MODEL_EVOLUTIONMIMEDATA_H

#include <QMimeData>

namespace MO {
class EvolutionBase;

class EvolutionMimeData : public QMimeData
{
    Q_OBJECT
public:

    static const QString mimeTypeString;

    static bool isInClipboard();

    /** Returns the ObjectMimeData wrapper if the mimedata is of that type */
    static EvolutionMimeData* evolutionMimeData(QMimeData*);
    /** Returns the ObjectMimeData wrapper if the mimedata is of that type */
    static const EvolutionMimeData* evolutionMimeData(const QMimeData*);

    // ----------- restore ----------------

    /** Returns the stored description.
        Returns invalid description if none was stored. */
    EvolutionBase* createSpecimen() const;

    // ------------ store --------------

    /** Sets the mime data */
    void setSpecimen(EvolutionBase*);
};

} // namespace MO

#endif // EVOLUTIONMIMEDATA_H
