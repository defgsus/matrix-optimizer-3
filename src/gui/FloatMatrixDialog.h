/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#ifndef MOSRC_GUI_FLOATMATRIXDIALOG_H
#define MOSRC_GUI_FLOATMATRIXDIALOG_H

#include <QDialog>
#include "math/FloatMatrix.h"

namespace MO {
class ValueFloatMatrixInterface;
class RenderTime;
namespace GUI {

class FloatMatrixWidget;

class FloatMatrixDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FloatMatrixDialog(QWidget *parent = 0);
    ~FloatMatrixDialog();

    const FloatMatrix& floatMatrix() const;

    static FloatMatrixDialog* openForInterface(
            ValueFloatMatrixInterface* iface,
            const RenderTime& time,
            uint channel = 0,
            QWidget* parent = nullptr);

signals:

    void matrixChanged() const;

public slots:

    void setFloatMatrix(const FloatMatrix&);
    void setReadOnly(bool);

private:

    FloatMatrixWidget* p_widget_;
    FloatMatrix p_backup_;
    bool p_readOnly_;
    QPushButton
            *p_butRevert_,
            *p_butCancel_,
            *p_butOk_,
            *p_butUpdate_;
};


} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_FLOATMATRIXDIALOG_H
