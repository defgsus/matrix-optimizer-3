/** @file wavetracerdialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.04.2015</p>
*/

#ifndef MOSRC_GUI_WAVETRACERDIALOG_H
#define MOSRC_GUI_WAVETRACERDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {

class WaveTracerWidget;

/** Complete gui wrapper around WaveTracerShader */
class WaveTracerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WaveTracerDialog(QWidget *parent = 0);
    ~WaveTracerDialog();

signals:

public slots:

private:

    WaveTracerWidget * p_widget_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WAVETRACERDIALOG_H
