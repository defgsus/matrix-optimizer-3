/** @file modulatordialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_GUI_MODULATORDIALOG_H
#define MOSRC_GUI_MODULATORDIALOG_H

#include <QDialog>

#include "object/Object_fwd.h"

class QComboBox;

namespace MO {
namespace GUI {

class ModulatorWidget;

class ModulatorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ModulatorDialog(QWidget *parent = 0);

    /** Sets a single modulator to edit */
    void setModulator(Modulator * mod)
        { setModulators(QList<Modulator*>() << mod, mod); }

    /** Sets a list of modulator to edit */
    void setModulators(const QList<Modulator*> mods, Modulator * select);

signals:

public slots:

private slots:

    void comboChanged_();

private:

    QList<Modulator*> modulators_;
    Modulator * selected_;

    QComboBox * combo_;

    ModulatorWidget * modWidget_;
};



} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_MODULATORDIALOG_H
