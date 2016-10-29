/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H
#define MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H

#include <QWidget>
#include "object/Object_fwd.h"


namespace MO {
namespace GUI {

/** A table editor for 1d & 2d FloatMatrix */
class FloatMatrixWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatMatrixWidget(QWidget *parent = 0);
    ~FloatMatrixWidget();

    const FloatMatrix& floatMatrix() const;

signals:

    void matrixChanged();

public slots:

    void setReadOnly(bool e);

    void setFloatMatrix(const FloatMatrix&);

    void loadDialog();
    void saveDialog();
    void rotateXY();
    void transposeXY();

private:
    struct Private;
    Private* p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H
