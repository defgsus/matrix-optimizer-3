/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H
#define MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H

#include <QWidget>
#include "object/param/floatmatrix.h"


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

public slots:

    void setFloatMatrix(const FloatMatrix&);

private:
    struct Private;
    Private* p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_FLOATMATRIXWIDGET_H
