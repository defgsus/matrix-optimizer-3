/** @file equationpreset.cpp

    @brief Preset group for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#include "equationpreset.h"
#include "io/error.h"
#include "io/xmlstream.h"

namespace MO {
namespace IO {

EquationPreset::EquationPreset()
{

}

EquationPreset::EquationPreset(const QString &filename)
{
    try
    {
        load(filename);
    }
    catch (...)
    {
        filename_ = "";
    }
}

void EquationPreset::serialize(XmlStream & io) const
{
    io.write("version", 1);

    for (auto &e : equs_)
    {
        io.newSection("equation");
            io.write("name", e.name);
            io.write("equ", e.equ);
        io.endSection();
    }
}

void EquationPreset::deserialize(XmlStream & io)
{
    QList<Equ_> equs;

    while (io.nextSubSection())
    {
        if (io.isSection("equation"))
        {
            Equ_ e;
            io.expect("name", e.name);
            io.expect("equ", e.equ);
            equs.append(e);
        }

        io.leaveSection();
    }

    equs_ = equs;
}

void EquationPreset::save(const QString &filename)
{
    XmlStream io;
    io.startWriting("equations");
    serialize(io);
    io.stopWriting();
    io.save(filename);
}

void EquationPreset::load(const QString &filename)
{
    XmlStream io;
    io.load(filename);
    io.startReading("equations");
    deserialize(io);
    io.stopReading();
}


void EquationPreset::setEquation(const QString &name, const QString &equation)
{
    for (int i=0; i<equs_.count(); ++i)
    {
        if (equs_[i].name == name)
        {
            equs_[i].equ = equation;
            return;
        }
    }

    Equ_ e;
    e.name = name;
    e.equ = equation;
    equs_.append(e);
}

void EquationPreset::removeEquation(const QString &name)
{
    for (int i=0; i<equs_.count(); ++i)
    {
        if (equs_[i].name == name)
        {
            equs_.removeAt(i);
            return;
        }
    }
}

void EquationPreset::removeEquation(int index)
{
    equs_.removeAt(index);
}


} // namespace IO
} // namespace MO
