/** @file equationpresets.cpp

    @brief Preset managment for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#include "EquationPresets.h"
#include "EquationPreset.h"
#include "io/Files.h"
#include "io/log_io.h"

namespace MO {
namespace IO {



EquationPresets::EquationPresets()
{
    rescan();
}

EquationPresets::~EquationPresets()
{
    clear();
}

void EquationPresets::clear()
{
    for (auto p : presets_)
        delete p;
    presets_.clear();
}

const QString& EquationPresets::name(int index) const
{
    return presets_[index]->name();
}

const QString& EquationPresets::filename(int index) const
{
    return presets_[index]->filename();
}

void EquationPresets::rescan()
{
    QStringList files;
    Files::findFiles(FT_EQUATION_PRESET, true, files);

    MO_DEBUG_IO("EquationPresets::rescan() " << files.count() << " preset files");

    clear();

    for (auto &f : files)
    {
        auto p = new EquationPreset(f);
        if (p->count())
            presets_.append(p);
        else
            delete p;
    }
}


} // namespace IO
} // namespace MO
