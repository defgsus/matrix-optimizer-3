/** @file infowindow.h

    @brief Window to display the client information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#ifndef MOSRC_GUI_INFOWINDOW_H
#define MOSRC_GUI_INFOWINDOW_H

#include <QMainWindow>
#include <QFont>

class QLabel;

namespace MO {
namespace GUI {


class InfoWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit InfoWindow(QWidget *parent = 0);

    QString infoString() const;

signals:

public slots:

    void updateInfo();

private:

    void createWidgets_();

    QLabel *labelId_, *labelInfo_;

    // --- config ----

    QFont bigFont_, normalFont_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_INFOWINDOW_H
