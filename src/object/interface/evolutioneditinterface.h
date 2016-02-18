/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_EVOLUTIONEDITINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_EVOLUTIONEDITINTERFACE_H

#include <map>
#include <QStringList>

namespace MO {
namespace GUI { class EvolutionDialog; }
class EvolutionBase;

/** Interface to define and pass EvolutionBase* instances
    to EvolutionDialog */
class EvolutionEditInterface
{
public:

    EvolutionEditInterface() { }
    virtual ~EvolutionEditInterface();

    GUI::EvolutionDialog * getAttachedEvolutionDialog(const QString& key) const;
    void setAttachedEvolutionDialog(const QString& key, GUI::EvolutionDialog* d);

    /** Returns all keys for which EvolutionBase* instances are available */
    const QStringList& evolutionKeys() const { return p_eei_keys_; }

    /** Returns the current specimen for the given key */
    virtual const EvolutionBase* getEvolution(const QString& key) const = 0;

    /** Sets new specimen */
    virtual void setEvolution(const QString& key, const EvolutionBase*) = 0;

protected:

    void addEvolutionKey(const QString& key) { if (!p_eei_keys_.contains(key)) p_eei_keys_ << key; }

private:

    std::map<QString, GUI::EvolutionDialog*> p_eei_diags_;
    QStringList p_eei_keys_;
};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_EVOLUTIONEDITINTERFACE_H
