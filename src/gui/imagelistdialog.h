/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#ifndef MOSRC_GUI_IMAGELISTDIALOG_H
#define MOSRC_GUI_IMAGELISTDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {

class ImageListWidget;

class ImageListDialog : public QDialog
{
    Q_OBJECT
public:
    ImageListDialog(bool showUpdateButton, QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~ImageListDialog();

    /** The list of image filenames as arranged in the list */
    QStringList imageList() const;

signals:

    /** Emitted when update button was clicked. */
    void listChanged();

public slots:

    /** Sets the filenames to load into the list.
        Replaces previous contents. */
    void setImageList(const QStringList&);

private:
    ImageListWidget * list;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_IMAGELISTDIALOG_H
