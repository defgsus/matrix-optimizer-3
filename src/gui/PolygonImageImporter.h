/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/22/2015</p>
*/

#ifndef MOSRC_GUI_POLYGONIMAGEIMPORTER_H
#define MOSRC_GUI_POLYGONIMAGEIMPORTER_H

#include <QDialog>

namespace MO {
namespace GUI {

/** Dialog to downsample high-resoluton monochrome graphics
    via distance-field technique */
class PolygonImageImporter : public QDialog
{
    Q_OBJECT
public:
    PolygonImageImporter(QWidget * parent, Qt::WindowFlags f = 0);
    ~PolygonImageImporter();

    bool isChanged() const;


public slots:

    void loadSHP();
    void render();
    void saveOutputImage();

protected:

    void closeEvent(QCloseEvent*);

private slots:

    void setProgress_(int);

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_POLYGONIMAGEIMPORTER_H
