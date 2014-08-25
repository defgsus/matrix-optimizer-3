/** @file sceneconvertdialog.h

    @brief Batch scene converter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#ifndef MOSRC_GUI_SCENECONVERTDIALOG_H
#define MOSRC_GUI_SCENECONVERTDIALOG_H

#include <QDialog>

class QListWidget;
class QLineEdit;
class QPushButton;
class QTextEdit;

namespace MO {
namespace GUI {


class SceneConvertDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SceneConvertDialog(QWidget *parent = 0);

signals:

private slots:

    void chooseInputPath_();
    void chooseOutputPath_();
    void convert_();

private:

    void createWidgets_();
    void fillList_(QListWidget *, const QString& path, bool checkable);
    bool canConvert_();

    QLineEdit * inputPath_, *outputPath_;
    QListWidget * input_, * output_;

    QPushButton * butConv_;

    QTextEdit * log_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SCENECONVERTDIALOG_H
