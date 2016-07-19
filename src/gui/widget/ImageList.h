/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/26/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_IMAGELIST_H
#define MOSRC_GUI_WIDGET_IMAGELIST_H

#include <QListWidget>

namespace MO {
namespace GUI {

/** A QListWidget for arranging images.

    Items are displayed with the image as icon,
    data for each item contains:
    Qt::UserRole: filename
    Qt::UserRole+1: info string
    Qt::UserRole+2: the QImage
    */
class ImageList : public QListWidget
{
public:
    ImageList(QWidget * parent = 0);

    /** True when addImage(QString) produced an error. */
    bool hasError() const { return !p_errorString_.isEmpty(); }

    /** The list of errors produced by addImage(QString). */
    const QString errorString() const { return p_errorString_; }

    /** Returns the full filenames as arranged in the list */
    QStringList filenames() const;

public slots:

    void clearErrors() { p_errorString_.clear(); }
    /** Adds the file to the image list.
        On error, false is returned and the error is added to the string
        returned by errorString(). */
    bool addImage(const QString& fn);

    /** Adds the image with corresponding filename to the list. */
    void addImage(const QString& fn, const QImage&);

protected:

    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);

private:

    QString p_errorString_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_IMAGELIST_H
