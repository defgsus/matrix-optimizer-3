/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/29/2016</p>
*/

#ifndef MOSRC_GUI_INSERTTIMEDIALOG_H
#define MOSRC_GUI_INSERTTIMEDIALOG_H

#include <QDialog>
#include "types/float.h"

class QComboBox;

namespace MO {
class Scene;
namespace GUI {
class DoubleSpinBox;

class InsertTimeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InsertTimeDialog(QWidget *parent = 0);
    ~InsertTimeDialog();

    Double getWhere() const { return where_; }
    Double getHowMuch() const { return howMuch_; }

signals:

public slots:

    void setScene(Scene* scene);
    void setWhere(Double where);
    void setHowMuch(Double howMuch);

private:

    Scene* scene_;
    Double where_, howMuch_;
    DoubleSpinBox *spinWhere_, *spinHowMuch_;
    QComboBox *cbLocators_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_INSERTTIMEDIALOG_H
