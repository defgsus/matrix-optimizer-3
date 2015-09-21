/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2015</p>
*/

#ifndef MOSRC_GUI_DISTANCEFIELDIMAGE_H
#define MOSRC_GUI_DISTANCEFIELDIMAGE_H

#include <QMainWindow>

namespace MO {
namespace GUI {

/** Dialog to downsample high-resoluton monochrome graphics
    via distance-field technique */
class DistanceFieldImage : public QMainWindow
{
    Q_OBJECT
public:
    DistanceFieldImage(QWidget * parent, Qt::WindowFlags f = 0);
    ~DistanceFieldImage();

public slots:

    void loadSourceImage();
    void start();
    void stop();

private slots:

    void threadFinsihed_();
    void setProgress_(float);

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_DISTANCEFIELDIMAGE_H
