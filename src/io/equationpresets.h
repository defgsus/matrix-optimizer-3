/** @file equationpresets.h

    @brief Preset managment for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#ifndef MOSRC_IO_EQUATIONPRESETS_H
#define MOSRC_IO_EQUATIONPRESETS_H

namespace MO {
namespace IO {


class EquationPresets
{
public:
    explicit EquationPresets();

    void rescan();

    //void addEquation(const QString& filename, const QString& )
};


} // namespace IO
} // namespace MO


#endif // EQUATIONPRESETS_H
