/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#ifndef MOSRC_GUI_FLOATMATRIXDIALOG_H
#define MOSRC_GUI_FLOATMATRIXDIALOG_H

#include <QDialog>
#include "object/param/FloatMatrix.h"


namespace MO {
namespace GUI {

class FloatMatrixWidget;

class FloatMatrixDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FloatMatrixDialog(QWidget *parent = 0);
    ~FloatMatrixDialog();

    const FloatMatrix& floatMatrix() const;

signals:

    void matrixChanged() const;

public slots:

    void setFloatMatrix(const FloatMatrix&);

private:

    FloatMatrixWidget* p_widget_;
    FloatMatrix p_backup_;
};


} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_FLOATMATRIXDIALOG_H
