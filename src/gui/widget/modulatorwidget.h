/** @file modulatorwidget.h

    @brief Editor for modulator settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_MODULATORWIDGET_H
#define MOSRC_GUI_WIDGET_MODULATORWIDGET_H

#include <QWidget>

#include "object/object_fwd.h"

class QLabel;

namespace MO {
namespace GUI {

class DoubleSpinBox;

class ModulatorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ModulatorWidget(QWidget *parent = 0);

signals:

public slots:

    /** Sets the modulator to edit.
        Can be NULL to disable the editor.
        @note The modulator must have an assigned parent object and a scene assigned! */
    void setModulator(Modulator * );

private slots:

    void updateFromWidgets_();
    void updateWidgets_();

private:

    Scene * scene_;
    Object * object_;
    Modulator * modulator_;

    QLabel * labelName_;
    DoubleSpinBox * spinAmplitude_, * spinTimeOffset_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_MODULATORWIDGET_H
