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
    void save();

    // --------- setter ---------------

    void clear();

    /** Sets the preset name */
    void setName(const QString& name) { name_ = name; }

    void setEquation(const QString& name, const QString& equation);
    void removeEquation(const QString& name);
    void removeEquation(int index);

    // --------- getter ---------------

    /** Returns the preset name */
    const QString& name() const { return name_; }
    const QString& filename() const { return filename_; }

    /** Returns the number of equations */
    int count() const { return equs_.size(); }
    const QString& equationName(int index) const { return equs_[index].name; }
    const QString& equation(int index) const { return equs_[index].equ; }

    bool hasEquation(const QString& name) const;

private:

    QString filename_, name_;

    struct Equ_
    {
        QString name, equ;
        bool operator < (const Equ_& r) const { return name < r.name; }
    };

    QList<Equ_> equs_;
};


} // namespace IO
} // namespace MO

#endif // MOSRC_IO_EQUATIONPRESET_H
