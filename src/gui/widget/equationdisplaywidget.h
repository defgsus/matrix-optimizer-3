/** @file equationdisplaywidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_EQUATIONDISPLAYWIDGET_H
#define MOSRC_GUI_WIDGET_EQUATIONDISPLAYWIDGET_H

#include <QWidget>

#include "types/float.h"
#include "gui/util/viewspace.h"

namespace PPP_NAMESPACE { class Parser; }

namespace MO {
namespace GUI {

namespace PAINTER { class ValueCurve; class Grid; }

class EquationDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EquationDisplayWidget(QWidget *parent = 0);
    ~EquationDisplayWidget();

    const UTIL::ViewSpace& viewSpace() const { return viewSpace_; }
    const QString& equation() const { return equation_; }
    const PPP_NAMESPACE::Parser * parser() const { return parser_; }

signals:

    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    void setViewSpace(const UTIL::ViewSpace&);
    void setEquation(const QString& equation);

protected:

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:

    PPP_NAMESPACE::Parser * parser_;

    QString equation_;

    Double varX_, varY_;

    UTIL::ViewSpace viewSpace_,
                    lastViewSpace_;

    class EquationData;
    EquationData * curveData_;
    PAINTER::ValueCurve * curve_;
    PAINTER::Grid * grid_;

    QPoint lastMousePos_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_EQUATIONDISPLAYWIDGET_H
