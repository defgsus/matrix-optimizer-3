/** @file

    @brief widget for projector setup

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/
#ifndef MOSRC_GUI_PROJECTORSETUPWIDGET_H
#define MOSRC_GUI_PROJECTORSETUPWIDGET_H

#include <QWidget>

namespace Ui {
class ProjectorSetupWidget;
}

class ProjectorSetupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectorSetupWidget(QWidget *parent = 0);
    ~ProjectorSetupWidget();

private:
    Ui::ProjectorSetupWidget *ui;
};

#endif // MOSRC_GUI_PROJECTORSETUPWIDGET_H
