/** @file equationpresets.h

    @brief Preset managment for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#ifndef MOSRC_IO_EQUATIONPRESETS_H
#define MOSRC_IO_EQUATIONPRESETS_H

#include <QString>
#include <QList>

namespace MO {
namespace IO {

class EquationPreset;

class EquationPresets
{
public:
    explicit EquationPresets();
    ~EquationPresets();

    // ------------ getter -----------------

    int count() const { return presets_.count(); }

    EquationPreset * preset(int index) { return presets_[index]; }
    const EquationPreset * preset(int index) const { return presets_[index]; }

    const QString& name(int index) const;
    const QString& filename(int index) const;

    // -------------- setter ---------------

    void clear();

    void rescan();

private:

    QList<EquationPreset*> presets_;
};


} // namespace IO
} // namespace MO


#endif // EQUATIONPRESETS_H
