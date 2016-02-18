/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/18/2016</p>
*/

#include "evolutioneditinterface.h"
#include "gui/evolutiondialog.h"

namespace MO {


EvolutionEditInterface::~EvolutionEditInterface()
{
    for (auto& i : p_eei_diags_)
        i.second->close();
}

void EvolutionEditInterface::setAttachedEvolutionDialog(
        const QString& key, GUI::EvolutionDialog* d)
{
    if (d)
        d->setAttribute(Qt::WA_DeleteOnClose);

    auto i = p_eei_diags_.find(key);
    if (i != p_eei_diags_.end())
    {
        if (!d)
            p_eei_diags_.erase(i);
        else
            i->second = d;
        return;
    }

    p_eei_diags_.insert(std::make_pair(key, d));
}

GUI::EvolutionDialog* EvolutionEditInterface::getAttachedEvolutionDialog(
        const QString &key) const
{
    auto i = p_eei_diags_.find(key);
    return i == p_eei_diags_.end() ? nullptr : i->second;
}

} // namespace MO
