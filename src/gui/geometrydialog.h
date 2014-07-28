/** @file geometrydialog.h

    @brief Editor for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_GEOMETRYDIALOG_H
#define MOSRC_GUI_GEOMETRYDIALOG_H

#include <QDialog>


namespace MO {
namespace GUI {

class GeometryWidget;

class GeometryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeometryDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

signals:

public slots:

protected slots:

    void updateGeometry_();

protected:

    void createWidgets_();

    GeometryWidget * geoWidget_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYDIALOG_H
