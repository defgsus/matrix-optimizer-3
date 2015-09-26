/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_IMAGELISTWIDGET_H
#define MOSRC_GUI_WIDGET_IMAGELISTWIDGET_H

#include <QWidget>

namespace MO {
namespace GUI {

/** A widget to load, display and arrange a list of images */
class ImageListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageListWidget(QWidget *parent = 0);
    ~ImageListWidget();

    /** The list of image filenames as arranged in the list */
    QStringList imageList() const;

signals:

public slots:

    void clearList();
    void addImages();

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_IMAGELISTWIDGET_H
