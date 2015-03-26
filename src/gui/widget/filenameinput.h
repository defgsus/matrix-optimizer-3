/** @file filenameinput.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_FILENAMEINPUT_H
#define MOSRC_GUI_WIDGET_FILENAMEINPUT_H

#include <QWidget>

#include "io/filetypes.h"

class QLineEdit;
class QToolButton;

namespace MO {
namespace GUI {


/** Small convenience widget to edit or select a
    filename or a directory */
class FilenameInput : public QWidget
{
    Q_OBJECT
public:
    explicit FilenameInput(IO::FileType filetype, bool directoryOnly, QWidget *parent = 0);
    explicit FilenameInput(bool directoryOnly = false, QWidget *parent = 0)
        : FilenameInput(IO::FT_ANY, directoryOnly, parent) { }

    QString fileName() const;

signals:

    void filenameChanged(const QString& fn);

public slots:

    /** Sets the contents of the line edit */
    void setFilename(const QString&);

    /** Opens a dialog for selecting the file/directory */
    void openDialog();
    
private slots:

    void onTextChanged_();

private:

    void createWidgets_();

    bool directoryOnly_,
         ignoreSig_;
    IO::FileType ftype_;
    QLineEdit * edit_;
    QToolButton * but_;
};


} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_WIDGET_FILENAMEINPUT_H
