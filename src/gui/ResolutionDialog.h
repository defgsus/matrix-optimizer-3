/** @file resolutiondialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#ifndef MOSRC_GUI_RESOLUTIONDIALOG_H
#define MOSRC_GUI_RESOLUTIONDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {

class SpinBox;

/** Little dialog to querry for a resolution.
    Displays a preset list.
    Resturns value in resolution().
    Applicate when QDialog::exec() returns QDialog::Accepted. */
class ResolutionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ResolutionDialog(const QSize& defaultResolution, QWidget *parent = 0);
    ~ResolutionDialog();

    /** The set resolution after accepted */
    QSize resolution() const { return res_; }

    /** Sets a new resolution, updates widgets */
    void setResolution(const QSize& size);

private:

    void createWidgets_();
    void updateSpins_();

    QSize default_, res_;

    SpinBox * s_width_, * s_height_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_RESOLUTIONDIALOG_H
