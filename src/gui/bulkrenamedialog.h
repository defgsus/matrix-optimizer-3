/** @file bulkrenamedialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.01.2015</p>
*/

#ifndef MOSRC_GUI_BULKRENAMEDIALOG_H
#define MOSRC_GUI_BULKRENAMEDIALOG_H

#include <QDialog>

#include <QDialog>

class QListWidget;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QSpinBox;

namespace MO {
namespace GUI {


class BulkRenameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BulkRenameDialog(QWidget *parent = 0);
    ~BulkRenameDialog();

signals:

private slots:

    void chooseInputPath_();
    void rename_();
    void optionsChanged_();

private:

    void createWidgets_();
    void fillInputList_(const QString& path, bool checkable);
    void fillOutputList_();
    bool canRename_();

    QString rename_(const QString & fn, int index) const;

    QLineEdit * inputPath_, * subStr_;
    QListWidget * input_, * output_;
    QSpinBox * seqStart_, * digitCount_;

    QPushButton * butRename_;

    QTextEdit * log_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_BULKRENAMEDIALOG_H
