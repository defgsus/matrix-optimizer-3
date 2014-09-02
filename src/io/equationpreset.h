/** @file equationpreset.h

    @brief Preset group for equations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/2/2014</p>
*/

#ifndef MOSRC_IO_EQUATIONPRESET_H
#define MOSRC_IO_EQUATIONPRESET_H

#include <QString>
#include <QList>

namespace MO {
namespace IO {

class XmlStream;

class EquationPreset
{
public:
    explicit EquationPreset();
    explicit EquationPreset(const QString& filename);

    // ------------- io ---------------

    void serialize(XmlStream&) const;
    void deserialize(XmlStream&);

    void load(const QString& filename);
    void save(const QString& filename);

    // --------- setter ---------------

    void setEquation(const QString& name, const QString& equation);
    void removeEquation(const QString& name);
    void removeEquation(int index);

    // --------- getter ---------------

    int count() const { return equs_.size(); }
    const QString& name(int index) const { return equs_[index].name; }
    const QString& equation(int index) const { return equs_[index].equ; }

private:

    QString filename_;

    struct Equ_
    {
        QString name, equ;
    };

    QList<Equ_> equs_;
};


} // namespace IO
} // namespace MO

#endif // MOSRC_IO_EQUATIONPRESET_H
