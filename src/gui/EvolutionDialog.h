/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_GUI_EVOLUTIONDIALOG_H
#define MOSRC_GUI_EVOLUTIONDIALOG_H

#include <QDialog>

namespace MO {
class EvolutionEditInterface;
class EvolutionBase;
namespace GUI {


class EvolutionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EvolutionDialog(QWidget* parent = 0);
    ~EvolutionDialog();

    /** Returns the currently selected specimen for which Ok or Apply was clicked */
    EvolutionBase* selectedSpecimen() const;

    void setEditSpecimen(const EvolutionBase*);

    static EvolutionDialog* openForInterface(EvolutionEditInterface*, const QString& key);

signals:

    /** Emitted on Ok and Apply */
    void apply();

public slots:

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_EVOLUTIONDIALOG_H
