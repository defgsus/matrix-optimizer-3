/** @file equationpreset.cpp

    @brief Preset group for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#include "equationpreset.h"
#include "io/xmlstream.h"
#include "io/log_io.h"
#include "io/error.h"

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
    io.write("name", name_);

    for (auto &e : equs_)
    {
        io.createSection("equation");
            io.write("name", e.name);
            io.write("text", e.equ);
        io.endSection();
    }
}

void EquationPreset::deserialize(XmlStream & io)
{
    QString name;
    QList<Equ_> equs;

    io.expect("name", name);

    while (io.nextSubSection())
    {
        if (io.isSection("equation"))
        {
            Equ_ e;
            io.expect("name", e.name);
            io.expect("text", e.equ);
            equs.append(e);
        }

        io.leaveSection();
    }

    qSort(equs);

    name_ = name;
    equs_ = equs;
}

void EquationPreset::save(const QString &filename)
{
    MO_DEBUG("EquationPreset::save('" << filename << "')");

    XmlStream io;
    io.startWriting("equations");
    serialize(io);
    io.stopWriting();
    io.save(filename);
    filename_ = filename;
}

void EquationPreset::save()
{
     if (filename_.isEmpty())
         MO_IO_ERROR(WRITE, "no filename defined for equation preset '" << name_ << "'");

     save(filename_);
}

void EquationPreset::load(const QString &filename)
{
    MO_DEBUG("EquationPreset::load('" << filename << "')");

    XmlStream io;
    io.load(filename);
    io.startReading("equations");
    deserialize(io);
    io.stopReading();
    filename_ = filename;

    MO_DEBUG_IO("EquationPreset::load('" << filename << "') "
             << count() << " presets");
}


void EquationPreset::clear()
{
    equs_.clear();
    name_.clear();
    filename_.clear();
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

bool EquationPreset::hasEquation(const QString &name) const
{
    for (int i=0; i<equs_.count(); ++i)
    {
        if (equs_[i].name == name)
        {
            return true;
        }
    }
    return false;
}


} // namespace IO
} // namespace MO
