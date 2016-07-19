/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#ifndef MOSRC_GUI_CSGDIALOG_H
#define MOSRC_GUI_CSGDIALOG_H

#include <QMainWindow>


namespace MO {
class CsgRoot;
namespace GUI {


class CsgDialog : public QMainWindow
{
    Q_OBJECT
public:
    explicit CsgDialog(QWidget *parent = 0);
    ~CsgDialog();

    bool isSaveToChange();

signals:

public slots:

    void loadFile();
    bool saveFile();
    bool saveFileAs();
    void loadFile(const QString& fn);
    bool saveFile(const QString& fn);

    void setRootObject(CsgRoot*);

protected:

    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_CSGDIALOG_H
